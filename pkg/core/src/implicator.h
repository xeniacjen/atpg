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

  //bool MakeDecision(); 

    Value GetVal(int gid) const; 

    void GetDFrontier(GateVec& df) const; 

    bool IsFaultAtPo() const; 

    void GetPiPattern(Pattern& p) const; 
    void GetPoPattern(Pattern& p) const; 
    void assignPatternPiValue(Pattern *pat); // write PI values to pattern
    void assignPatternPoValue(Pattern *pat); // write PO values to pattern

private: 
    Value           GoodEval(Gate *g) const; 
    Value           FaultEval(Gate* g) const; 

    Circuit         *cir_; 
    Fault           *target_fault_; 

    Value           *values_; 
    std::queue<int> *events_; 

}; // Implicator 

inline Implicator::Implicator(Circuit *cir, Fault *ftarget) {
    cir_ = cir; 
    target_fault_ = ftarget;     

    values_ = new Value [cir_->tgate_]; 
    events_ = new std::queue<int> [cir_->tlvl_]; 
}

inline Implicator::~Implicator() {
    delete [] values_; 
    delete [] events_; 
} 

inline void Implicator::Init() {
    for (int i=0; i<cir_->tgate_; i++) 
        values_[i] = X; 
}

inline Value Implicator::GetVal(int gid) const {
    return values_[gid]; 
}

}; // CoreNs

#endif // _CORE_IMPL_H_
