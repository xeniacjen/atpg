/*
 * =====================================================================================
 *
 *       Filename:  implicator.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/08/2015 05:10:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include "implicator.h" 

using namespace std; 

using namespace CoreNs; 

void Implicator::PushEvent(int gid) {
    events_[cir_->gates_[gid]->lvl_].push(gid); 
}

void Implicator::PushFanoutEvent(int gid) {
    Gate *g = cir_->gates_[gid]; 
    for (int n=0; n<g->nfo_; n++) 
        PushEvent(g->fos_[n]); 
}

bool Implicator::IsFaultAtPo() const {
    for (int n=0; n<(cir_->npo_+cir_->nppi_); n++) {
        Value v = GetVal(cir_->tgate_ - n - 1); 
        if (Value==D || Value ==B) 
            return true;        
    }
    return false; 
}

void GetDFrontier(GateVec& df) const {
    for (int n=0; n<tgate_; n++) { 
        if (GetVal(n)!=X) 
            continue; 
        
        Gate *g = cir_->gates_[n]; 
        for (int m=0; m<g->nfi_; m++) { 
            Value v = GetVal(g->fis[m])
            if(v==D||v==B) { 
                df.push_back(g); 
                break; 
            }
        }
    }
}
