/*
 * =====================================================================================
 *
 *       Filename:  impl.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/19/2015 02:17:57 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#ifndef _CORE_IMPL_H_ 
#define _CORE_IMPL_H_ 

#include <queue> 

#include "decision_tree.h"
#include "pattern.h"

namespace CoreNs {

typedef std::vector<Gate *> GateVec; 

class Implicator { 
public: 
         Implicator(Circuit *cir, Fault *ftarget); 
         ~Implicator(); 

    void Init(); 

    bool EventDrivenSim(); 
    void PushFanoutEvent(int gid); 
    void PushEvent(int gid); 

    bool MakeDecision(Gate *g, Value v); 
    bool BackTrack(); 

    Value GetVal(int gid) const; 
    Value Get3Val(int gid) const; 
    bool  SetVal(int gid, Value v); 

    void GetDFrontier(GateVec& df) const; 

    bool IsFaultAtPo() const; 

    void GetPiPattern(Pattern *p) const; 
    void GetPoPattern(Pattern *p) const; 

private: 
    Value           GoodEval(Gate *g) const; 
    Value           FaultEval(Gate* g) const; 

    Circuit         *cir_; 
    Fault           *target_fault_; 

    Value           *values_; 
    std::queue<int> *events_; 

    DecisionTree    decision_tree_; 
    std::vector<int> e_front_list_; 
}; // Implicator 

inline Implicator::Implicator(Circuit *cir, Fault *ftarget) {
    cir_ = cir; 
    target_fault_ = ftarget;     

    values_ = new Value [cir_->tgate_]; 
    events_ = new std::queue<int>();  
}

inline Implicator::~Implicator() {
    delete [] values_; 
    delete    events_; 
} 

inline void Implicator::Init() {
    for (int i=0; i<cir_->tgate_; i++) 
        values_[i] = X; 
}

inline Value Implicator::GetVal(int gid) const {
    return values_[gid]; 
}

inline Value Implicator::Get3Val(int gid) const {
    if (values_[gid]==D || values_[gid]==B) 
        return (values_[gid]==D)?H:L;
    else 
        return values_[gid]; 
}

inline bool Implicator::SetVal(int gid, Value v) { 
    if (values_[gid]!=X && values_[gid]!=v) return false; 

    values_[gid] = v; 
    return true; 
}

inline void Implicator::GetPiPattern(Pattern *p) const {
    for (int i=0; i<cir_->npi_; i++)    
        p->pi1_[i] = Get3Val(i); 
	if(p->pi2_!=NULL && cir_->nframe_>1)
		for( int i = 0 ; i < cir_->npi_ ; i++ )
            p->pi2_[i] = Get3Val(i+cir_->ngate_); 
	for( int i = 0 ; i < cir_->nppi_ ; i++ )
            p->ppi_[i] = Get3Val(i+cir_->npi_); 
}

inline void Implicator::GetPoPattern(Pattern *p) const  {}

}; // CoreNs

#endif // _CORE_IMPL_H_
