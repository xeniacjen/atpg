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
#include <set>

#include "decision_tree.h"
#include "pattern.h"
#include "simulator.h"

namespace CoreNs {

class LearnInfoMgr;  

typedef std::set<int> GateSet; 

class Implicator { 
    friend class LearnInfoMgr; 
public: 
         Implicator(Circuit *cir, Fault *ftarget); 
         ~Implicator(); 

    void Init(); 
    void Init(Pattern *p); 

    bool EventDrivenSim(); 
    bool EventDrivenSimB(); // backward event-driven sim. 
    bool EventDrivenSimHex(); 
    void PushFanoutEvent(int gid); 
    void PushFaninEvent(int gid); 
    void PushEvent(int gid); 
    void PushBEvent(int gid); 
    void PushFanoutEventHex(int gid); 
    void PushEventHex(int gid); 

    bool MakeDecision(Gate *g, Value v); 
    bool BackTrack(); 

    bool  isPossiblyToSetVal(int gid, Value v) const; 
    // bool  isPossiblyToSetDorB(int gid) const; 
    Value GetVal(int gid) const; 
    HexValue GetHexVal(int gid) const; 
    Value Get3Val(int gid) const; 
    bool  SetVal(int gid, Value v); 
    bool  SetVal(int gid, HexValue hv); 

    void PrintGate(int gid) const; 

    void GetDFrontier(GateVec& df) const; 
    bool GetDFrontierNCheck(GateVec& df); 
    bool GetJFrontier(GateVec& jf); 

    bool IsFaultAtPo() const; 
    bool isGateDrivePpo(Gate *g); 


    void GetPiPattern(Pattern *p) const; 
    void GetPoPattern(Pattern *p) const; 

    bool isUnjustified(int gid) const; 

    size_t GetEFrontierSize() const; 
    void ClearDecisionTree();    
    const DecisionTree &getDecisionTree() const; 
    void setDecisionTree(const DecisionTree &tree); 

private: 
    Value           GoodEval(Gate *g) const; 
    Value           FaultEval(Gate* g) const; 
    HexValue        GoodEvalHex(Gate *g) const; 
    HexValue        FaultEvalHex(Gate *g) const; 

    void            AssignValue(int gid, Value v); 

    Circuit         *cir_; 
    Simulator       *sim_;  

    Fault           *target_fault_; 

    Value           *values_; 
    HexValue        *hvalues_; 
    std::queue<int> *events_; 
    std::queue<int> *events_b; // backward implication events 
    std::queue<int> *hevents_; 

    GateSet         d_front_; 
    GateSet         j_front_; 

    DecisionTree    decision_tree_; 
    std::vector<int> e_front_list_; 

    bool            *is_possibly_D; 
}; // Implicator 

inline Implicator::Implicator(Circuit *cir, Fault *ftarget) {
    cir_ = cir; 
    target_fault_ = ftarget;     

    sim_    = new Simulator(cir_); 
    is_possibly_D = new bool [cir_->tgate_]; 
    values_ = new Value [cir_->tgate_]; 
    hvalues_= new HexValue [cir_->tgate_];  
    events_ = new std::queue<int>();  
    events_b = new std::queue<int>();  
    hevents_ = new std::queue<int>();  
}

inline Implicator::~Implicator() {
    delete    sim_; 
    delete [] is_possibly_D; 
    delete [] values_; 
    delete [] hvalues_; 
    delete    events_; 
    delete    events_b; 
    delete    hevents_; 
} 

inline void Implicator::Init() {
    for (int i=0; i<cir_->tgate_; i++) { 
        values_[i] = X; 
        hvalues_[i] = HexValue(X); 
        cir_->gates_[i].gl_ = PARA_L; 
        cir_->gates_[i].gh_ = PARA_L; 
        cir_->gates_[i].fl_ = PARA_L; 
        cir_->gates_[i].fh_ = PARA_L; 
    }

    // std::queue<int> possibly_D_queue; 
    // possibly_D_queue.push(target_fault_->gate_); 
// 
    // while (!possibly_D_queue.empty()) { 
        // int gid = possibly_D_queue.back(); 
        // possibly_D_queue.pop(); 
        // is_possibly_D[gid] = true; 
// 
        // Gate *g = &cir_->gates_[gid]; 
        // for (int i=0; i<g->nfo_; i++) 
            // possibly_D_queue.push(g->fos_[i]); 
    // } 
}

inline void Implicator::Init(Pattern *p) {
    for (int j=0; j<cir_->npi_; ++j)
        AssignValue(j, p->pi1_[j]);
    for (int j=0; j<cir_->nppi_; ++j)
        AssignValue(cir_->npi_ + j, p->ppi_[j]);

    EventDrivenSim(); 
    e_front_list_.clear(); 
}


inline Value Implicator::GetVal(int gid) const {
    return values_[gid]; 
}

inline HexValue Implicator::GetHexVal(int gid) const {
    return hvalues_[gid]; 
}

inline Value Implicator::Get3Val(int gid) const {
    if (values_[gid]==D || values_[gid]==B) 
        return (values_[gid]==D)?H:L;
    else 
        return values_[gid]; 
}

// inline bool Implicator::isPossiblyToSetDorB(int gid) const { 
//     return is_possibly_D[gid]; 
// } 

inline bool Implicator::SetVal(int gid, Value v) { 
    if (values_[gid]!=X && values_[gid]!=v) return false; 

    values_[gid] = v; 
    return true; 
}

inline bool Implicator::SetVal(int gid, HexValue hv) {
    if (!hv.isSubset(hvalues_[gid])) return false; 

    hvalues_[gid] = hv; 
    return true; 
}

inline bool Implicator::isUnjustified(int gid) const { 
    Gate *g = &cir_->gates_[gid]; 
    Value v = (g->id_==target_fault_->gate_)?FaultEval(g):GoodEval(g); 
    return (v!=GetVal(gid)); 
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

inline void Implicator::PrintGate(int gid) const {
    Gate *g = &cir_->gates_[gid]; 

    cout << "#  ";
    cout << "id(" << g->id_ << ") ";
    cout << "lvl(" << g->lvl_ << ") ";
    cout << "type(" << g->type_ << ") ";
    cout << "frame(" << g->frame_ << ")";
    cout << endl;
    cout << "#    fi[" << g->nfi_ << "]";
    for (int j = 0; j < g->nfi_; ++j) { 
        cout << " " << g->fis_[j] << "("; 
        printValue(GetVal(g->fis_[j])); 
        cout << ")"; 
    }
    cout << endl;
    cout << "#    fo[" << g->nfo_ << "]";
    for (int j = 0; j < g->nfo_; ++j)
        cout << " " << g->fos_[j];
    cout << endl << endl;
} 

inline size_t Implicator::GetEFrontierSize() const { 
    return e_front_list_.size(); 
}

inline void Implicator::ClearDecisionTree() { 
   decision_tree_.clear();  
}

inline const DecisionTree &Implicator::getDecisionTree() const { 
    return decision_tree_; 
}
 
inline void Implicator::setDecisionTree(const DecisionTree &tree) { 
    decision_tree_ = tree; 
} 

inline void Implicator::AssignValue(int gid, Value v) { 
    // if (!SetVal(gid, v)) return false; 
    if (!SetVal(gid, v)) assert(0); 
    // if (!SetVal(gid, HexValue(v))) assert(0); 
    
    e_front_list_.push_back(gid); 

    if (target_fault_->gate_==gid) { 
        PushEvent(gid); 
        // PushEventHex(g->id_); 
    }
    else { 
        PushFanoutEvent(gid); 
        // PushFanoutEventHex(g->id_); 

        // if (g->isFanoutStem()) 
        PushBEvent(gid); 
    }
} 
 
}; // CoreNs

#endif // _CORE_IMPL_H_
