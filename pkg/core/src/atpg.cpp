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

        if (is_obj_optim_mode_) { 
            Value *mask = new Value [f.size()]; 
            for (size_t i=0; i<f.size(); i++) 
                mask[i] = H; 
            d_tree_.top()->set_mask_(mask); 
        }
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
        bool is_a_test = (is_obj_optim_mode_)?isaMultiTest():isaTest();
        if (is_a_test) { 
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

bool Atpg::isaMultiTest() { 
    // TODO 
    GateVec gids; 

    // get the previous object 
    d_tree_.top(gids); 
    for (size_t i=0; i<gids.size(); i++) { 
        Gate *g = gids[i]; 

        if (!impl_->isGateDrivePpo(g)) return false; 
    } 

    return true; 
}

void Atpg::init() { 
    back_track_count = 0; 
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

bool Atpg::insertObj(const Objective& obj, ObjList& objs) { 
    Value v = impl_->GetVal(obj.first); 
    assert(v==X); 

    pair<ObjListIter, bool> ret = objs.insert(obj); 

    if (!ret.second && ret.first->second!=obj.second)  
        return false; 

    return true; 
} 

bool Atpg::AddGateToProp(Gate *gtoprop) { 
    Value v = impl_->GetVal(gtoprop->id_); 

    if (v==D || v==B) // D-frontier pushed forward 
        return true; 
    else if (v!=X) // D-frontier compromised 
        return false;  
    else  { 
        // TODO 
        ObjList objs = objs_; // create a temp. copy 

        Objective obj; 
        obj.first = gtoprop->id_; 
        obj.second = gtoprop->getOutputCtrlValue(); 
    
        assert(!gtoprop->isUnary()); 

        stack<Objective> event_list; 
        event_list.push(obj); 
        while (!event_list.empty()) { 
            obj = event_list.top(); 
            event_list.pop(); 

            Value v = impl_->GetVal(obj.first); 
            if (v!=X && v==EvalNot(obj.second)) return false; 
            if (!insertObj(obj, objs)) return false; 
           
            Gate *g = &cir_->gates_[obj.first]; 
            if (g->type_==Gate::BUF || g->type_==Gate::INV) { 
                obj.first = g->fis_[0]; 
                obj.second = (g->isInverse())?EvalNot(obj.second):obj.second;
                event_list.push(obj); 
            } 
            else { 
                if (g->getOutputCtrlValue()==obj.second) { 
                    for (int i=0; i<g->nfi_; i++) { 
                        if (impl_->GetVal(g->fis_[i])!=X) continue; 
                        obj.first = g->fis_[i]; 
                        obj.second = g->getInputNonCtrlValue(); 
                        event_list.push(obj); 
                    }
                }
            }
        }
        objs_ = objs; 
    }

    return true; 
}

bool Atpg::GenObjs() { 
    GateVec gids; 
    size_t size; 
    bool ret = false; 

    objs_.clear(); 

    // get the previous object 
    d_tree_.top(gids); 
    Value *mask = d_tree_.top()->get_mask_(size); 
    int j = 0; 
    for (size_t i=0; i<size; i++) { 
        if (mask[i]==L) continue; 
        Gate *gtoprop = gids[j++]; 
    
        if (!AddGateToProp(gtoprop)) { 
            if (mask[i]==H) return false; 
            mask[i] = L; 
        } 
        else { 
            mask[i] = H; 
            ret = true; 

            if (!objs_.empty()) { 
                // if has P/PIs obj. 
                if (objs_.begin()->first<cir_->npi_+cir_->nppi_) 
                    break; 
            }
        }
    }
    // assert(j==gids.size()); 

    if (!objs_.empty()) { 
        // if (!CheckDDDrive()) return false; 
        current_obj_ = *objs_.begin(); 
    }

    return ret; 
}

bool Atpg::CheckDDDrive() { 
    GateVec gs_proped, gs_toprop; 

    d_tree_.top(gs_toprop); 
    d_tree_.sub_top()->top(gs_proped); 
    for (size_t n=0; n<gs_proped.size(); n++) { 
        Gate *g = gs_proped[n]; 
        if (impl_->isGateDrivePpo(g)) continue; 

        bool successor_found = false; 
        // for (int i=0; i<g->nfo_; i++) { 
        //     ObjListIter it = objs_.find(g->fis_[i]); 
        //     if (it!=objs_.end()) { 
        for (int i=0; i<g->nfo_; i++) { 
            Gate *fo = &cir_->gates_[g->fos_[i]]; 
            for (size_t j=0; j<gs_toprop.size(); j++) { 
                // if (it!=objs_.end()) { 
                if (fo==gs_toprop[j]) { 
                    successor_found = true; 
                    break; 
                }
            }
            if (successor_found) break; 
        }
        if (!successor_found) 
            return false; 
    }

    return true; 
}

bool comp_gate(Gate* g1, Gate* g2); 
bool Atpg::MultiDDrive() { 
    GateVec dpath; 

    if (!GenObjs()) return false; 

    // Check path is sensitized 
    d_tree_.GetMultiPath(dpath); 
    if (CheckPath(dpath)) { 
        if (objs_.empty()) { // D-frontier pushed forward 
            GateVec dfront; 
            impl_->GetDFrontier(dfront); 
    
            if (!CheckDFrontier(dfront)) return false;
    
            sort (dfront.begin(), dfront.end(), comp_gate); 
            Gate *gtoprop = dfront.back(); 
    
            assert(gtoprop->isUnary()==L); 
            d_tree_.push(dfront, 
                impl_->GetEFrontierSize(), 
                impl_->getDecisionTree()); 
            impl_->ClearDecisionTree();  

            Value *mask = new Value [dfront.size()]; 
            for (size_t i=0; i<dfront.size(); i++) 
                mask[i] = X; 
            d_tree_.top()->set_mask_(mask); 

            return GenObjs(); 
        } 
        else return true; // initial objective unchanged 
    }

    return false; // D-path justification failed 
}

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
    
            // sort (dfront.begin(), dfront.end(), comp_gate); 
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
    if (is_obj_optim_mode_) return MultiDDrive(); 
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
    assert(impl_->GetVal(g->id_)==X); 
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
        if (is_obj_optim_mode_) { 
            if (MultiDBackTrack(tree)) { 
                impl_->setDecisionTree(tree); 
                if(d_tree_.empty()) return false; 
                continue; 
            } 
        } 
        else { 
            if (d_tree_.pop(tree)) { 
                impl_->setDecisionTree(tree); 
                if(d_tree_.empty()) return false; 
                continue; 
            } 
        }
        return true; 
    }

    return false; 
} 

bool Atpg::MultiDBackTrack(DecisionTree &tree) { 
    bool is_flipped = false; 
    bool ret = true; 
    size_t size; 
    Value *mask = d_tree_.top()->get_mask_(size); 
    // TODO  
    while (size!=0) { 
        Value &v = mask[--size]; 
        if (v==H) { 
            if (!is_flipped) { 
                v = L; 
                is_flipped = true; 
            }
            else 
                ret = false; 
        }
        else { 
            if (!is_flipped) { 
                v = X; 
                ret = false; 
            }
        }
    }

    if (ret) 
        d_tree_.pop_hard(tree); 

    return ret; 
}

bool Atpg::CheckCompatibility(Fault *f) { 
    Gate *g = &cir_->gates_[f->gate_]; 

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
