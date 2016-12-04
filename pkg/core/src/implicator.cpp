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


size_t Implicator::nsa_ = 0; 
size_t Implicator::nnsa_ = 0; 

bool Implicator::EventDrivenSim() {
    while (!events_b->empty()) events_b->pop();  
    while (!events_->empty()) { 
        Gate *g = &cir_->gates_[events_->front()]; 
        events_->pop(); 

        Value v; 
        v = (g->has_fault_)?FaultEval(g):GoodEval(g); 
        if (v==X) continue; 

        if (GetVal(g->id_)!=v) { 
            if (!SetVal(g->id_, v)) { 
                if (g->has_fault_)
                    values_[g->id_] = v; 
                else { 
                    return false; 
                    // assert(0); 
                }
            }
            e_front_list_.push_back(g->id_); 
            PushFanoutEvent(g->id_); 
        }
    }
    return true; 
}

bool Implicator::EventDrivenSimB() { 
    while (!events_b->empty()) { 
        Gate *g = &cir_->gates_[events_b->front()]; 
        events_b->pop(); 

        if (g->nfo_>1) PushFanoutEvent(g->id_); 

        Value v = values_[g->id_]; 
        if (g->type_==Gate::BUF || g->type_==Gate::INV) { 
            v = (g->isInverse())?EvalNot(v):v; 
            if (!SetVal(g->fis_[0], v)) return false; 
            e_front_list_.push_back(g->fis_[0]); 
            PushBEvent(g->fis_[0]); 
        }
        else if (g->type_==Gate::PI|| g->type_==Gate::PPI)  
            continue; 
        else { 
            if (g->getOutputCtrlValue()==v) { 
                for (int i=0; i<g->nfi_; i++) { 
                    if (!SetVal(g->fis_[i], g->getInputNonCtrlValue())) 
                        return false; 
                    e_front_list_.push_back(g->fis_[i]); 
                    PushBEvent(g->fis_[i]); 
                }
            }
            else { 
                // TODO 
            }
        }
    }
}

bool Implicator::EventDrivenSimHex() {
    // TODO 
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
        case Gate::AND: 
        case Gate::AND2: 
        case Gate::AND3: 
        case Gate::AND4: 
            return EvalAndN(vs); 
            break; 
        case Gate::NAND: 
        case Gate::NAND2: 
        case Gate::NAND3: 
        case Gate::NAND4: 
            return EvalNandN(vs); 
            break; 
        case Gate::OR: 
        case Gate::OR2: 
        case Gate::OR3: 
        case Gate::OR4: 
            return EvalOrN(vs); 
            break; 
        case Gate::NOR: 
        case Gate::NOR2: 
        case Gate::NOR3: 
        case Gate::NOR4: 
            return EvalNorN(vs); 
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
    Value v, v1, v2; 
    bool f_at_input = false, f_at_output = false; 

    Fault *target_fault_; 
    if (g->fs_[0]) { //fault on output 
        if (g->type_==Gate::PI || g->type_==Gate::PPI) 
            v = values_[g->id_]; 
        else 
            v = GoodEval(g); 

        target_fault_ = g->fs_[0]; 
        if (v==L&&(target_fault_->type_==Fault::SA1
            || target_fault_->type_==Fault::STF)) v = B; 
        if (v==H&&(target_fault_->type_==Fault::SA0
            || target_fault_->type_==Fault::STR)) v = D; 

        f_at_output = true; 
        v2 = v;  
    }

    vector<Value> vs; 
    for (int i=0; i<g->nfi_; i++){
        v = GetVal(g->fis_[i]); 
        if (g->fs_[i+1]) { 
            target_fault_ = g->fs_[i+1]; 
            if (v==L&&(target_fault_->type_==Fault::SA1
             || target_fault_->type_==Fault::STF)) v = B; 
            if (v==H&&(target_fault_->type_==Fault::SA0
             || target_fault_->type_==Fault::STR)) v = D; 

            f_at_input = true; 
        }

        vs.push_back(v); 
    } 

    if (f_at_input) { 
        switch (g->type_) { 
            case Gate::INV: 
                v1 = EvalNot(v); 
                break; 
            case Gate::AND: 
            case Gate::AND2: 
            case Gate::AND3: 
            case Gate::AND4: 
                v1 = EvalAndN(vs); 
                break; 
            case Gate::NAND: 
            case Gate::NAND2: 
            case Gate::NAND3: 
            case Gate::NAND4: 
                v1 = EvalNandN(vs); 
                break; 
            case Gate::OR: 
            case Gate::OR2: 
            case Gate::OR3: 
            case Gate::OR4: 
                v1 = EvalOrN(vs); 
                break; 
            case Gate::NOR: 
            case Gate::NOR2: 
            case Gate::NOR3: 
            case Gate::NOR4: 
                v1 = EvalNorN(vs); 
                break; 
            case Gate::PO: 
            case Gate::PPO: 
            case Gate::BUF: 
                v1 = v; 
                break;  
            default:  
                assert(0); // should never get here... 
                break; 
        }
    }

    if (f_at_input && f_at_output) {
        assert(v1==v2); 
        return v1; 
    }
    else if (f_at_input)
        return v1; 
    else if (f_at_output) 
        return v2; 

    assert(0); 
    return X; 
}

void Implicator::PushEvent(int gid) {
    Gate *g = &cir_->gates_[gid]; 
    if (g->type_==Gate::PI || g->type_==Gate::PPI)  
        assert(g->has_fault_); 

    events_->push(gid); 

}

void Implicator::PushFanoutEvent(int gid) {
    Gate *g = &cir_->gates_[gid]; 
    for (int n=0; n<g->nfo_; n++) 
        PushEvent(g->fos_[n]); 
}

void Implicator::PushFaninEvent(int gid) {
    Gate *g = &cir_->gates_[gid]; 
    for (int n=0; n<g->nfi_; n++) 
        PushBEvent(g->fis_[n]); 
}

void Implicator::PushBEvent(int gid) {
    events_b->push(gid); 
}

void Implicator::PushEventHex(int gid) {
    Gate *g = &cir_->gates_[gid]; 
    if (g->type_==Gate::PI || g->type_==Gate::PPI)
        assert(g->has_fault_); 

    hevents_->push(gid); 

}

void Implicator::PushFanoutEventHex(int gid) {
    Gate *g = &cir_->gates_[gid]; 
    for (int n=0; n<g->nfo_; n++) 
        PushEventHex(g->fos_[n]); 
}

bool Implicator::IsFaultAtPo() const {
    for (int n=0; n<(cir_->npo_+cir_->nppi_); n++) {
        Value v = GetVal(cir_->tgate_ - n - 1); 
        if (v==D || v==B) 
            return true;        
    }
    return false; 
}

bool Implicator::isGateDrivePpo(Gate *g) { 
    Value v = GetVal(g->id_); 
    if (v!=D && v!=B) return false; 
    if (g->type_==Gate::PO || g->type_==Gate::PPO)
        return true; 

    for (int i=g->nfo_-1; i>=0; i--) { 
        Gate *fo = &cir_->gates_[g->fos_[i]]; 
        if (isGateDrivePpo(fo)) return true; 
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
 
bool Implicator::GetDFrontierNCheck(GateVec& df) {
    bool d_front_flag_ = false; 
    df.clear(); 

    for (int n=0; n<cir_->tgate_; n++) { 
        GateSet::iterator it = d_front_.find(n); 
        if (it!=d_front_.end()) {
            if (GetVal(n)!=X) { 
                d_front_.erase(it); 
                d_front_flag_ = true; 
            }
        } 
        else { 
            if (GetVal(n)!=X) 
                continue; 
        
            Gate *g = &cir_->gates_[n]; 
            for (int m=0; m<g->nfi_; m++) { 
                Value v = GetVal(g->fis_[m]); 
                if(v==D||v==B) { 
                    d_front_.insert(n); 
                    d_front_flag_ = true; 
                    break; 
                }
            }
        }
    }
    for (GateSet::iterator it = d_front_.begin(); it!=d_front_.end(); ++it) 
        df.push_back(&cir_->gates_[*it]); 

    return d_front_flag_; 
}

bool Implicator::GetJFrontier(GateVec& jf) { 
    bool j_front_flag_ = false; 
    jf.clear(); 

    for (int i=0; i<cir_->tgate_; i++) { 
        GateSet::iterator it = j_front_.find(i); 
        if (it!=j_front_.end()) { 
            if (!isUnjustified(i)) { 
                j_front_.erase(it); 
                j_front_flag_ = true; 
            }
        } 
        else { 
            if (GetVal(i)==X) 
                continue; 

            if (isUnjustified(i)) { 
                j_front_.insert(i); 
                j_front_flag_ = true; 
            }
        }
    }
    for (GateSet::iterator it = j_front_.begin(); it!=j_front_.end(); ++it) 
        jf.push_back(&cir_->gates_[*it]); 

    return j_front_flag_; 
}

bool Implicator::MakeDecision(Gate *g, Value v) {
    assert(g->type_==Gate::PI || g->type_==Gate::PPI); 

    //TODO: HexValue backtrack functionality 
    decision_tree_.put(g->id_, e_front_list_.size()); 
    AssignValue(g->id_, v); 

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

        if (cir_->gates_[gid].has_fault_) 
            PushEvent(gid); 
        else 
            PushFanoutEvent(gid); 
        return true; 
    }

    if (back_track_point>0) { 
        for (int i=back_track_point; i<e_front_list_.size(); i++) 
            values_[e_front_list_[i]] = X; 

        e_front_list_.resize(back_track_point); 
    } 

    return false; 
}

void Implicator::RelaxRSA(const GateSet &nsa, unsigned sp) { 
    vector<Objective> nsas; 
    for (size_t i=0; i<decision_tree_.size(); i++) { 
        GateSet::const_iterator it 
          = nsa.find(decision_tree_.tree_[i].gid_); 

        if (it!=nsa.end()) { 
            nsas.push_back(make_pair(*it, values_[*it])); 
            nnsa_++; 
        }
    }
    nsa_+=decision_tree_.size(); 
    if (nsas.size()==decision_tree_.size()) return; 

    for (size_t i=sp; i<e_front_list_.size(); i++) 
        values_[e_front_list_[i]] = X; 
    e_front_list_.resize(sp); 

    decision_tree_.clear(); 
    for (size_t i=0; i<nsas.size(); i++) { 
        MakeDecision(&cir_->gates_[nsas[i].first], 
                     nsas[i].second); 
        EventDrivenSim(); 
    }
}
