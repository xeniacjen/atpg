/*
 * =====================================================================================
 *
 *       Filename:  fan.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/10/2016 10:46:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include <climits>

#include "fan.h" 

using namespace std; 
using namespace CoreNs; 

Atpg::GenStatus Fan::Tpg() {
    if (!FaultActivate()) return UNTESTABLE ; 
    back_trace_flag = true; 
    while (true) {
        Imply(); 
        if (!DDrive() || !Backtrace()){ 
            if(!BackTrack()) 
                return (back_track_count>=back_track_limit)?ABORT:UNTESTABLE; 
        }
        else impl_->MakeDecision(&cir_->gates_[final_obj_.first], final_obj_.second); 
    }
}

bool Fan::FaultActivate() {
    Gate *fg = &cir_->gates_[current_fault_->gate_]; 
    int fline = current_fault_->line_; 
   
    Value v = (current_fault_->type_==Fault::SA0 
        || current_fault_->type_==Fault::STR)?H:L;
    if (fline) { // input stuck fault on fan-out branch. 
        Gate *g = &cir_->gates_[fg->fis_[fline-1]]; 
        return impl_->MakeDecision(g, v); 
    } 
    else { // output stuck fault on P/PI. XXX: might has problem 
        return impl_->MakeDecision(fg, v); 
    }  
}

bool Fan::DDrive() { 
    if (!impl_->IsFaultAtPo()) { // if fault effect not prop. to P/PO 
        back_trace_flag = impl_->GetDFrontier(dfront_);  
        impl_->GetJFrontier(jfront_); 
        if (dfront_.size()>=1) return true; 
        else return false; 
    } 
    
    back_trace_flag = impl_->GetJFrontier(jfront_); 
    return true; 
}

bool Fan::GenInitDObjectives() { 
    if (dfront_.size()>1) { 
        Gate *gtoprop = NULL; 
        int observ = INT_MAX; 
        for (size_t i=0; i<dfront_.size(); i++) 
            if(dfront_[i]->co_o_<observ) { 
                gtoprop = dfront_[i]; 
                observ = dfront_[i]->co_o_; 
            }
        pushSideInputObj(gtoprop); 
        return true; 
    } 
    else { 
        return UniquePathSensitize(dfront_[0]); 
    } 
}

bool Fan::GenInitJObjectives() { 
    //XXX: un-justified free line 
    for (size_t i=0; i<jfront_.size(); i++) {
        Objective obj; 
        obj.first = jfront_[i]->id_; 
        obj.second = impl_->GetVal(obj.first); 
        init_objs_.push_back(obj); 
    } 
}

bool Fan::UniquePathSensitize(Gate *g) { 
    // XXX: wrong impl. 
    while (!g->isFanoutStem()) { 
        if (impl_->GetVal(g->id_)!=X) 
            return false; 
        pushSideInputObj(g); 
        g = &cir_->gates_[g->fos_[0]]; 
    }
    return true; 
}

bool Fan::Backtrace() { 
    bool skip_fanout_objs_check = false; 
    while (true) { 
        if (back_trace_flag) { // is back-trace flag on?  
            back_trace_flag = false; // reset back-trace flag 
            clearObjList(); // let all obj lists empty 
            if (jfront_.size()>0) { // is there any unjustified line? 
                GenInitJObjectives(); // let all unjustified line be init objs 
                if (!impl_->IsFaultAtPo()) // fault effect prop to P/PO 
                    if(!GenInitDObjectives()) return false; // add a gate in d-front to init objs 
            }
            else { 
                if(!GenInitDObjectives()) return false; // add a gate in d-front to init objs 
            }

            if (MultipleBacktrace(true)) { 
                skip_fanout_objs_check = true; 
                continue; 
            }
            return true; 
        } 
        else { 
            if (fanout_objs_.empty() || skip_fanout_objs_check) { // fan-out objs empty? 
                skip_fanout_objs_check = false; 
                while (!head_objs_.empty()) { // head objs empty? 
                    Objective head_obj = head_objs_.back(); 
                    head_objs_.pop_back(); 
                    if (impl_->GetVal(head_obj.first)==X) { // is the head unspecified? 
                        final_obj_ = head_obj; 
                        return true; 
                    }
                } 
                back_trace_flag = true; 
                continue; 
            } 
            else { 
            if (MultipleBacktrace(false)) { 
                skip_fanout_objs_check = true; 
                continue; 
            }
                return true; 
            }
        } 
    }
}

bool Fan::MultipleBacktrace(bool is_from_init) { 
    if (!MapObjectives()) return false; 

    while (true) { 
        FanObjective obj; 
        if (!curr_objs_.empty() && is_from_init) { 
            obj = curr_objs_.back(); 
            curr_objs_.pop_back(); 
        } 
        else { 
            if (!fanout_objs_.empty() || !is_from_init) { 
                FanObjectiveMap::iterator it, it_to_pop; 
                int lvl = INT_MIN; 
                for (it=fanout_objs_.begin(); it!=fanout_objs_.end(); ++it) { 
                    Gate *g = &cir_->gates_[it->second.gid]; 
                    if (g->lvl_>lvl) { 
                        obj = it->second; 
                        it_to_pop = it; 
                    }
                }
                fanout_objs_.erase(it_to_pop); 
                if (!impl_->isPossiblyToSetDorB(obj.gid) 
                  && (obj.n0!=0 && obj.n1!=0)) { 
                    final_obj_.first = obj.gid; 
                    final_obj_.second = (obj.n0>obj.n1)?L:H; 
                    return false; 
                }
            }
            else return true; 
        }
        
        Gate *g = &cir_->gates_[obj.gid]; 
        if (g->ltype_==Gate::HEAD_LINE) { 
            // TODO 
            // head_objs_.push_back(obj); 
        }
        else { 
            // TODO: gates without controlling value 
            Gate *fi_easiest_to_set_cont_val 
              = FindEasiestToSetFanIn(g, EvalNot(g->getOutputCtrlValue())); 
            for (int i=0; i<g->nfi_; i++) { 
                Gate *fi = &cir_->gates_[g->fis_[i]]; 
                if (fi->isFanoutStem()) { 
                    // TODO 
                }
                else if (fi==fi_easiest_to_set_cont_val) { 
                    FanObjective fi_obj; 
                    fi_obj.gid = fi->id_; 
                    if (!g->isInverse()) { 
                        fi_obj.n0 = obj.n0; 
                        fi_obj.n1 = obj.n1; 
                    }
                    else { 
                        fi_obj.n0 = obj.n1; 
                        fi_obj.n1 = obj.n0; 
                    }
                    curr_objs_.push_back(fi_obj); 
                }
                else { 
                    FanObjective fi_obj; 
                    fi_obj.gid = fi->id_; 
                    // TODO 
                    curr_objs_.push_back(fi_obj); 
                }
            }
        }
    }
}

bool Fan::MapObjectives() { 
    // TODO: mapping with aids of learning  
    for (size_t i=0; i<init_objs_.size(); i++) { 
        FanObjective obj; 

        obj.gid = init_objs_[i].first; 
        if (init_objs_[i].second==L) obj.n0 = 1; 
        else obj.n1 = 1; 

        curr_objs_.push_back(obj); 
    }
    return true; 
}
