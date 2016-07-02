/*
 * =====================================================================================
 *
 *       Filename:  fan.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/10/2016 04:12:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#ifndef _CORE_FAN_H_
#define _CORE_FAN_H_ 

#include <map>

#include "atpg.h"

namespace CoreNs { 

struct FanObjective { 
    int gid; 
    int n0; 
    int n1; 

    FanObjective() { 
        gid = -1; 
        n0 = 0; 
        n1 = 0; 
    }
};  

typedef std::map<int, FanObjective> FanObjectiveMap; 

class Fan : public Atpg{
public: 
        Fan(Circuit *cir, Fault *f) : Atpg(cir, f) { } 
        ~Fan(); 

    Atpg::GenStatus Tpg();  
private:
    bool GenInitDObjectives(); 
    bool GenInitJObjectives(); 
    bool UniquePathSensitize(Gate *g); 
    bool MultipleBacktrace(bool is_from_init); 
    bool MapObjectives(); 

    void pushSideInputObj(Gate* g); 
    void clearObjList(); 

    bool back_trace_flag;  // true if continuous back-trace not meaningful

    GateVec dfront_; 
    GateVec jfront_; 

    std::vector<Objective> init_objs_; 
    std::vector<FanObjective> curr_objs_; 
    FanObjectiveMap fanout_objs_; 
    std::vector<Objective> head_objs_; 
    Objective final_obj_; 

protected: 
    bool FaultActivate(); 
    bool DDrive(); 
    bool Backtrace(); 
}; // Fan 

inline void Fan::pushSideInputObj(Gate* g) { 
    for (size_t i=0; i<g->nfi_; i++) { 
        if (impl_->GetVal(g->fis_[i])!=X) 
            continue; 
        Objective obj; 
        obj.first = g->fis_[i]; 
        obj.second = g->getInputNonCtrlValue(); 
        init_objs_.push_back(obj); 
    } 
}

inline void Fan::clearObjList() { 
    init_objs_.clear(); 
    curr_objs_.clear(); 
    fanout_objs_.clear(); 
    head_objs_.clear(); 
}

} // CoreNs

#endif // _CORE_FAN_H_
