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
    while (!events_->empty()) { 
        Gate *g = &cir_->gates_[events_->front()]; 
        events_->pop(); 

        Value v, v1, v2; 
        v = (g->id_==target_fault_->gate_)?FaultEval(g):GoodEval(g); 
        sim_->goodEval(g->id_); 
    
        if (v==D || v==B) 
            v1 = (v==D)?H:L;
        else 
            v1 = v;

        if (g->gl_!=PARA_L) v2 = L; 
        else if (g->gh_!=PARA_L) v2 = H; 
        else v2 = X; 
/** 
        if (v1!=v2) {
            cout << "\n----------------------------------------------------------------\n"; 
            cout << "Mismatch Found: \n"; 
            cout << "----------------------------------------------------------------\n"; 
            PrintGate(g->id_); 
            printValue(v); cout << " =/= "; 
            printValue(g->gl_, g->gh_); cout << endl;  
            cout << "----------------------------------------------------------------\n"; 
            v = v2; 
        }
*/ 
        if (GetVal(g->id_)!=v) { 
            if (!SetVal(g->id_, v)) { 
                if (g->id_==target_fault_->gate_)
                    values_[g->id_] = v; 
                else 
                    assert(0); 
            }
            e_front_list_.push_back(g->id_); 
            PushFanoutEvent(g->id_); 
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
        case Gate::XOR2: 
        case Gate::XOR3: 
            return EvalXorN(vs); 
            break; 
        case Gate::XNOR2: 
        case Gate::XNOR3: 
            return EvalXnorN(vs); 
            break; 
        case Gate::BUF: 
        case Gate::PO: 
        case Gate::PPO: 
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
        Value v;  
        if (g->type_==Gate::PI || g->type_==Gate::PPI) 
            v = values_[g->id_]; 
        else 
            v = GoodEval(g); 
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
            case Gate::XOR2: 
            case Gate::XOR3: 
                return EvalXorN(vs); 
                break; 
            case Gate::XNOR2: 
            case Gate::XNOR3: 
                return EvalXnorN(vs); 
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
    events_->push(gid); 
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

bool Implicator::MakeDecision(Gate *g, Value v) {
    // if (!SetVal(g->id_, v)) return false; 
    assert(g->type_==Gate::PI || g->type_==Gate::PPI); 
    if (!SetVal(g->id_, v)) assert(0); 

    if (v==L) g->gl_ = PARA_H; 
    else if (v==H) g->gh_ = PARA_H; 

    decision_tree_.put(g->id_, e_front_list_.size()); 
    e_front_list_.push_back(g->id_); 

    if (target_fault_->gate_==g->id_) 
        PushEvent(g->id_); 
    else 
        PushFanoutEvent(g->id_); 
    return true; 
}

bool Implicator::BackTrack() {
    int gid = -1; 
    unsigned back_track_point = 0; 

    while (!decision_tree_.empty()) { // while decision tree is not empty 
        if (decision_tree_.get(gid, back_track_point)) // while last node is flagged 
            continue; 

        Value flipped_val; 
        if (GetVal(gid)==D) flipped_val = L; 
        else if (GetVal(gid)==B) flipped_val = H; 
        else if (GetVal(gid)==X) assert(0); 
        else flipped_val = EvalNot(GetVal(gid)); 

        for (int i=back_track_point+1; i<e_front_list_.size(); i++) 
            values_[e_front_list_[i]] = X; 

        e_front_list_.resize(back_track_point+1); 
        values_[gid] = flipped_val; 

        if (target_fault_->gate_==gid) 
            PushEvent(gid); 
        else 
            PushFanoutEvent(gid); 
        return true; 
    }

    return false; 
}
