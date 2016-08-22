/*
 * =====================================================================================
 *
 *       Filename:  atpg.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/22/2015 06:13:31 PM
 *       Revision:  none
 *       Compiler:  g++ 
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include <cassert>
#include <climits>
#include <algorithm>

#include "atpg.h" 

using namespace std; 

using namespace CoreNs; 

Atpg::GenStatus Atpg::Tpg() { 
    if (is_path_oriented_mode_) { 
        GateVec f; f.push_back(&cir_->gates_[current_fault_->gate_]); 
        DecisionTree tree_dummy; tree_dummy.clear(); 
        d_tree_.push(f, 0, tree_dummy); 
    }

    while (true) { 
        if (isTestPossible()) { 
            Backtrace(); // Make a decision 
        } 
        else { 
            bool tpg_fail = (is_path_oriented_mode_)?!DBackTrack():!BackTrack();
            if(tpg_fail) // TPG process failed  
                return (back_track_count>=back_track_limit)?ABORT:UNTESTABLE; 
        }
        Imply(); 
        if (isaTest()) { 
            return TEST_FOUND; 
        }
    }
}

bool Atpg::isTestPossible() { 
    Gate *fg = &cir_->gates_[current_fault_->gate_]; 
    if (impl_->GetVal(fg->id_)==X) {  // GUT output at X? 
        FaultActivate(); 
        return true; 
    }    
    else  
        return DDrive(); 
}

bool Atpg::isaTest() { 
    for (size_t n=0; n<cir_->npo_+cir_->nppi_; n++) { 
        Value v = impl_->GetVal(cir_->tgate_ - n - 1); 
        if (v==D || v==B) return true; 
    }
    return false; 
}

void Atpg::init() { 
    back_track_count = 0; 
    // back_track_limit = _MAX_BACK_TRACK_LIMIT_; 
    back_track_limit = 0; 

    impl_->Init(); 
}

void Atpg::init(Pattern *p) { 
    init(); 

    impl_->Init(p); 
}

bool Atpg::Imply() { 
    return impl_->EventDrivenSim(); 
} 

bool Atpg::FaultActivate() { // TODO: TDF support 
    Gate *fg = &cir_->gates_[current_fault_->gate_]; 
    int fline = current_fault_->line_; 

    if (fline) { // input stuck fault on GUT. 
        Value v = impl_->GetVal(fg->fis_[fline-1]); 
        if (v==X) { // faulted input at X? 
            Value objv = (current_fault_->type_==Fault::SA0 
                || current_fault_->type_==Fault::STR)?H:L;
            current_obj_.first = fg->fis_[fline-1]; 
            current_obj_.second = objv; 
        }
        else { 
            Value objv = fg->getOutputCtrlValue(); 
            if (objv==X) assert(0); //NOT, PO, PPO, TODO: XOR, XNOR
            current_obj_.first = fg->id_; 
            current_obj_.second = objv; 
            return true; 
        }
    }
    else { 
        Value objv = (current_fault_->type_==Fault::SA0 
            || current_fault_->type_==Fault::STR)?H:L;
        current_obj_.first = fg->id_; 
        current_obj_.second = objv; 
        return true; 
    }
}

bool Atpg::CheckDFrontier(GateVec &dfront) { 
   if (is_path_oriented_mode_) { 
       for (int i=dfront.size()-1; i>=0; i--) 
           if (!CheckDPath(dfront[i])) { 
               dfront[i] = dfront.back(); 
              dfront.pop_back(); 
           }
   }

   ResetXPath(); 
   for (int i=dfront.size()-1; i>=0; i--) 
        if (!CheckXPath(dfront[i])) { 
            dfront[i] = dfront.back(); 
            dfront.pop_back(); 
        }

    return (!dfront.empty()); 
} 

bool comp_gate(Gate* g1, Gate* g2); 
bool Atpg::DDDrive() { 
    GateVec dpath; 
    int gid; 

    // get the previous object 
    d_tree_.top(gid); 
    Gate *gtoprop = &cir_->gates_[gid]; 
    current_obj_.first = gtoprop->id_; 
    current_obj_.second = gtoprop->getOutputCtrlValue(); 
    Value v = impl_->GetVal(current_obj_.first); 

    // Check path is sensitized 
    d_tree_.GetPath(dpath); 
    if (CheckPath(dpath)) { 
        if (v==D || v==B) { // D-frontier pushed forward 
            GateVec dfront; 
            impl_->GetDFrontier(dfront); 
    
            if (!CheckDFrontier(dfront)) return false;
    
            sort (dfront.begin(), dfront.end(), comp_gate); 
            gtoprop = dfront.back(); 
    
            assert(gtoprop->isUnary()==L); 
            d_tree_.push(dfront, 
                impl_->GetEFrontierSize(), 
                impl_->getDecisionTree()); 
            impl_->ClearDecisionTree();  

            current_obj_.first = gtoprop->id_; 
            current_obj_.second = gtoprop->getOutputCtrlValue(); 
            return true; 
        } 
        else if (v!=X) { // D-frontier compromised 
            return false; 
        }
        else return true; // initial objective unchanged 
    }

    return false; // D-path justification failed 
}

bool comp_gate(Gate* g1, Gate* g2) { 
    return g1->co_o_ > g2->co_o_; 
}

bool Atpg::DDrive() { 
    if (is_path_oriented_mode_) return DDDrive(); 

    GateVec dfront; 
    impl_->GetDFrontier(dfront); 

    if (!CheckDFrontier(dfront)) return false;

    Gate *fg = &cir_->gates_[current_fault_->gate_]; 
    assert(impl_->GetVal(fg->id_)==D || impl_->GetVal(fg->id_)==B); 
    
    Gate *gtoprop = NULL; 
    int observ = INT_MAX; 
    for (size_t i=0; i<dfront.size(); i++) 
        if(dfront[i]->co_o_<observ) { 
            gtoprop = dfront[i]; 
            observ = dfront[i]->co_o_; 
        }

    assert(gtoprop->isUnary()==L); 
    current_obj_.first = gtoprop->id_; 
    current_obj_.second = gtoprop->getOutputCtrlValue(); 

    return true; 
}

bool Atpg::Backtrace() {
    Gate *g = &cir_->gates_[current_obj_.first]; 
    Value objv = current_obj_.second; 
    while (!(g->type_==Gate::PI 
        || g->type_==Gate::PPI)) { // while objective net not fed by P/PI 
        
        if (g->getOutputCtrlValue()==X) { //NOT, BUF, TODO: XOR, XNOR 
            if (g->type_==Gate::INV) 
                objv = EvalNot(objv); 
            g = &cir_->gates_[g->fis_[0]]; 
            assert(impl_->GetVal(g->id_)==X); 
            continue; 
        } 

        Gate *gnext = NULL; 
        if (objv==EvalNot(g->getOutputCtrlValue())) { // if objv is easy to set 
            // choose input of "g" which 
            //  1) is at X 
            //  2) is easiest to control 
            gnext = FindEasiestToSetFanIn(g, objv); 
        }
        else if (objv==g->getOutputCtrlValue()) { // is objv is hard to set
            // choose input of "g" which 
            //  1) is at X 
            //  2) is hardest to control 
            gnext = FindHardestToSetFanIn(g, objv); 
        } 
        
        if (g->isInverse()) 
            objv = EvalNot(objv); 

        g = gnext; 
    }

    return impl_->MakeDecision(g, objv); 
}

Gate *Atpg::FindHardestToSetFanIn(Gate *g, Value obj) const {
    Gate *ret = 0; 
    int ctr_ablility = INT_MIN; 
    if (g->isInverse()) 
        obj = EvalNot(obj); 

    for (size_t n=0; n<g->nfi_; n++ ) { 
        int cc = (obj==H)?cir_->gates_[g->fis_[n]].cc1_
          : cir_->gates_[g->fis_[n]].cc0_; 

        if (impl_->GetVal(g->fis_[n])==X && cc>ctr_ablility) { 
            ret = &cir_->gates_[g->fis_[n]]; 
            ctr_ablility = cc; 
        }
    }

    return ret; 
} 

Gate *Atpg::FindEasiestToSetFanIn(Gate *g, Value obj) const { 
    Gate *ret = 0; 
    int ctr_ablility = INT_MAX; 
    if (g->isInverse()) 
        obj = EvalNot(obj); 

    for (size_t n=0; n<g->nfi_; n++ ) { 
        int cc = (obj==H)?cir_->gates_[g->fis_[n]].cc1_
          : cir_->gates_[g->fis_[n]].cc0_; 

        if (impl_->GetVal(g->fis_[n])==X && cc<ctr_ablility) { 
            ret = &cir_->gates_[g->fis_[n]]; 
            ctr_ablility = cc; 
        }
    }

    return ret; 
} 

bool Atpg::BackTrack() { 
    if (!is_path_oriented_mode_) { // in normal mode 
        back_track_count++; 
           if (back_track_count>=back_track_limit) return false; 
        // return false; 
    }

    return impl_->BackTrack(); 
}

bool Atpg::DBackTrack() { 
    back_track_count++; 
    if (back_track_count>=back_track_limit) return false; 

    // int gid; 
    Gate* gtoprop; 
    DecisionTree tree; 
    while (!d_tree_.empty()) { 
        if (BackTrack()) return true; 
        if (d_tree_.pop(tree)) { 
            impl_->setDecisionTree(tree); 
            if(d_tree_.empty()) return false; 
            continue; 
        } 
        return true; 
    }

    return false; 
} 

bool Atpg::CheckCompatibility(Fault *f) { 
    Gate *g = &cir_->gates_[f->gate_]; 
    // TODO 
    Value v = impl_->Get3Val(g->id_); 
    if (v==L  
      && (f->type_==Fault::SA0 
      || f->type_==Fault::STR)) return false; 
    else if (v==H  
      && (f->type_==Fault::SA1 
      || f->type_==Fault::STF)) return false; 

    ResetXPath(); 
    return CheckXPath(g); 
    
}
