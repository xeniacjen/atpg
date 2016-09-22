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
        Value *mask = new Value [fgs.size()]; 
        for (size_t i=0; i<fgs.size(); i++)  
            mask[i] = H; 
        
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
