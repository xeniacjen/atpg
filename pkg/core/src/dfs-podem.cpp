/*
 * =====================================================================================
 *
 *       Filename:  dfs-podem.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/19/2016 02:58:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#define NFAULTACT       1 

#include <cassert>
#include <climits>
#include <algorithm>

#include "atpg.h" 

using namespace std; 

using namespace CoreNs; 

bool Atpg::init_d_tree() {
    GateVec fgs; 
    // FaultSetMap f2p; 
    // GateSetMap p; 

    Fault *f = current_fault_; 
    Gate *fg = &cir_->gates_[f->gate_]; 
    fgs.push_back(fg);   
    DecisionTree tree_dummy; tree_dummy.clear(); 
    // if (is_obj_optim_mode_) { 
    //     FaultSet fs; 
    //     f2p.insert(pair<Gate *, FaultSet>(fg, fs)); 
    // }

    d_tree_.push(fgs, 0, tree_dummy); 

    if (is_obj_optim_mode_) { 
        fgs.clear(); 
        FaultVec fs; 
        vector<Value> vs; 
        FaultListIter it = flist_->begin(); 
        for (size_t i=0; i<NFAULTACT; i++) {  
            if (it==flist_->end()) break; 
            f = *it; 
            fgs.push_back(&cir_->gates_[f->gate_]); 
            fs.push_back(f); 
            vs.push_back(X); 
            ++it; 
        }
        
        Value *mask = new Value[vs.size()]; 
        for (size_t i=0; i<vs.size(); i++) 
            mask[i] = vs[i]; 

        d_tree_.top()->dfront_ = fgs; 
        d_tree_.top()->fs_ = fs; 
        d_tree_.top()->set_mask_(mask); 
        // d_tree_.top()->set_f2p(f2p); 
    }

    return true; 
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
            // GateSet nsa; 
            // FindNSA(&cir_->gates_[current_obj_.first], nsa); 
            // impl_->RelaxRSA(nsa, d_tree_.top()->startPoint_); 
            // assert(impl_->GetVal(current_obj_.first)==D 
            //     || impl_->GetVal(current_obj_.first)==B); 

            GateVec dfront; 
            impl_->GetDFrontier(dfront); 
    
            if (!CheckDFrontier(dfront)) return false;
    
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

void Atpg::FindNSA(Gate *gproped, GateSet &nsa) { 
    stack<Gate *> l; 
    l.push(gproped); 
    while (!l.empty()) { 
        Gate *g = l.top(); 
        l.pop(); 

        Value v = impl_->GetVal(g->id_); 
        if (g->type_==Gate::PI || g->type_==Gate::PPI) 
            nsa.insert(g->id_); 
        // else if (v==X) continue; 
        else if (v==EvalNot(g->getOutputCtrlValue())) { 
            for (int i=0; i<g->nfi_; i++) { 
                if (impl_->GetVal(g->fis_[i])
                  ==g->getInputCtrlValue()) { 
                    l.push(&cir_->gates_[g->fis_[i]]); 
                }
            }
        }
        else { 
            for (int i=0; i<g->nfi_; i++)
                l.push(&cir_->gates_[g->fis_[i]]); 
        }
    }
}
