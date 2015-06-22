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

#include <cassert>

#include "implicator.h" 

using namespace std; 

using namespace CoreNs; 

bool Implicator::EventDrivenSim() {
    for (int l=0; l<cir_->tlvl_; l++) { 
        while (!events_[l].empty()) { 
            Gate *g = &cir_->gates_[events_[l].front()]; 
            events_[l].pop(); 

            Value v; 
            v = (g->id_==target_fault_->gate_)?FaultEval(g):GoodEval(g); 

            if (GetVal(g->id_)!=v) { 
                values_[g->id_] = v; 
                PushFanoutEvent(g->id_); 
            }
        }
    }
    return true; 
}

Value Implicator::GoodEval(Gate *g) const { 
    Value v = GetVal(g->fis_[0]); 
    vector<Value> vs; 
    for (int n=0; n<g->nfi_; n++) 
        vs.push_back(GetVal(g->fis_[n])); 
            
    switch (g->type_) { 
        case Gate::INV: 
            return EvalNot(v); 
            break; 
        case Gate::AND2: 
        case Gate::AND3: 
        case Gate::AND4: 
            return EvalAndN(vs); 
            break; 
        case Gate::NAND2: 
        case Gate::NAND3: 
        case Gate::NAND4: 
            return EvalNandN(vs); 
            break; 
        case Gate::OR2: 
        case Gate::OR3: 
        case Gate::OR4: 
            return EvalOrN(vs); 
            break; 
        case Gate::NOR2: 
        case Gate::NOR3: 
        case Gate::NOR4: 
            return EvalNorN(vs); 
            break; 
        case Gate::PO: 
        case Gate::PPO: 
        case Gate::BUF: 
            return v; 
            break;  
        default:  
            assert(0); // should never get here... 
            break; 
    }
    return X; 
}

Value Implicator::FaultEval(Gate *g) const { 
    if (target_fault_->line_==0) { //fault on output 
        Value v = GoodEval(g); 
        if (v==L&&(target_fault_->type_==Fault::SA1
            || target_fault_->type_==Fault::STF)) v = B; 
        if (v==H&&(target_fault_->type_==Fault::SA0
            || target_fault_->type_==Fault::STR)) v = D; 
        return v; 
    }
    else {
        int line = target_fault_->line_; 
        Value v = GetVal(g->fis_[line-1]); 
        if (v==L&&(target_fault_->type_==Fault::SA1
            || target_fault_->type_==Fault::STF)) v = B; 
        if (v==H&&(target_fault_->type_==Fault::SA0
            || target_fault_->type_==Fault::STR)) v = D; 
        
        vector<Value> vs; vs.push_back(v); 
        for (int n=0; n<g->nfi_; n++) 
            if (n!=line-1) 
                vs.push_back(GetVal(g->fis_[n])); 
                
        switch (g->type_) { 
            case Gate::INV: 
                return EvalNot(v); 
                break; 
            case Gate::AND2: 
            case Gate::AND3: 
            case Gate::AND4: 
                return EvalAndN(vs); 
                break; 
            case Gate::NAND2: 
            case Gate::NAND3: 
            case Gate::NAND4: 
                return EvalNandN(vs); 
                break; 
            case Gate::OR2: 
            case Gate::OR3: 
            case Gate::OR4: 
                return EvalOrN(vs); 
                break; 
            case Gate::NOR2: 
            case Gate::NOR3: 
            case Gate::NOR4: 
                return EvalNorN(vs); 
                break; 
            case Gate::PO: 
            case Gate::PPO: 
            case Gate::BUF: 
                return v; 
                break;  
            default:  
                assert(0); // should never get here... 
                break; 
        }
    }

    return X; 
}

void Implicator::PushEvent(int gid) {
    events_[cir_->gates_[gid].lvl_].push(gid); 
}

void Implicator::PushFanoutEvent(int gid) {
    Gate *g = &cir_->gates_[gid]; 
    for (int n=0; n<g->nfo_; n++) 
        PushEvent(g->fos_[n]); 
}

bool Implicator::IsFaultAtPo() const {
    for (int n=0; n<(cir_->npo_+cir_->nppi_); n++) {
        Value v = GetVal(cir_->tgate_ - n - 1); 
        if (v==D || v==B) 
            return true;        
    }
    return false; 
}

void Implicator::GetDFrontier(GateVec& df) const {
    for (int n=0; n<cir_->tgate_; n++) { 
        if (GetVal(n)!=X) 
            continue; 
        
        Gate *g = &cir_->gates_[n]; 
        for (int m=0; m<g->nfi_; m++) { 
            Value v = GetVal(g->fis_[m]); 
            if(v==D||v==B) { 
                df.push_back(g); 
                break; 
            }
        }
    }
}
