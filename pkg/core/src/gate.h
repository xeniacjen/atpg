// **************************************************************************
// File       [ gate.h ]
// Author     [ littleshamoo ]
// Synopsis   [ ]
// Date       [ 2011/07/05 created ]
// **************************************************************************

#ifndef _CORE_GATE_H_
#define _CORE_GATE_H_

#include <iostream>
#include <cstring>

#include "interface/cell.h"

#include "logic-hex.h"

using namespace std; 

namespace CoreNs {

class Gate; 

typedef std::vector<Gate *> GateVec;  
void PrintGateVec(const GateVec& gs); // helper function for GDB  

class Gate {
public:
              Gate();
              ~Gate();

    enum      Type { NA = 0,
                     PI,    PO,
                     PPI,   PPO,   PPI_IN, PPO_IN,
                     INV,   BUF,
                     AND, AND2,  AND3,  AND4,
                     NAND, NAND2, NAND3, NAND4,
                     OR, OR2,   OR3,   OR4,
                     NOR, NOR2,  NOR3,  NOR4,
                     XOR, XOR2,  XOR3,
                     XNOR, XNOR2, XNOR3,
                     MUX,
                     TIE0,   TIE1, TIEX,   TIEZ
                   };

    enum LINE_TYPE { FREE_LINE  = 0, HEAD_LINE,  BOUND_LINE };

    // basic info
    char      *name_; 
    int       id_;    // position in circuit gate array
    int       cid_;   // original cell id in the netlist
    int       pmtid_; // original primitive id in the library cell
    int       lvl_;   // level number after levelization
    int       frame_; // time frame of the gate, for 2-pattern test
    Type      type_;  // type of the gate
    LINE_TYPE ltype_; // line type of the gate 

    // connection
    int       nfi_;   // number of fanin
    int       *fis_;  // fanin array
    int       nfo_;   // number of fanout
    int       *fos_;  // fanout array

    // values
    //Value     v_;     // single value for ATPG
    ParaValue gl_;    // good low
    ParaValue gh_;    // good high
    ParaValue fl_;    // faulty low
    ParaValue fh_;    // faulty high

    // constraint
	// user can tie the gate to certain value
    bool      hasCons_;
    ParaValue cons_;

    // SCOAP, testability
    int       cc0_;
    int       cc1_;
    int       co_o_; 
    int       *co_i_; 

    int       depthFromPo_; // depth from po, this is for fault effect propagation
    int       fiMinLvl_;    // the minimum level of the fanin gates, this is to justify the headline cone, (in atpg.cpp)

    void      print() const; 
    string    getTypeName() const; 
    
    bool      isFanoutStem() const; 

    Value     isUnary() const;
    Value     isInverse() const;
    Value     getInputNonCtrlValue() const;
    Value     getInputCtrlValue() const;
    Value     getOutputCtrlValue() const;

};

inline Gate::Gate() {
    id_      = -1;
    cid_     = -1;
    pmtid_   = 0;
    lvl_     = -1;
    frame_   = 0;
    type_    = NA;
    nfi_     = 0;
    fis_     = NULL;
    nfo_     = 0;
    fos_     = NULL;
    gl_      = PARA_L;
    gh_      = PARA_L;
    fl_      = PARA_L;
    fh_      = PARA_L;
    hasCons_ = false;
    cons_    = PARA_L;

    cc0_     = 0;
    cc1_     = 0;
    co_o_    = 0;
    co_i_    = 0; 

    depthFromPo_ = -1;
    fiMinLvl_    = -1;
}

inline Gate::~Gate() {
    if(!co_i_) delete co_i_;    
}

inline string Gate::getTypeName() const { 
    switch(type_) {
        case PI: return string("PI"); 
        case PO: return string("PO"); 
        case PPI: return string("PPI"); 
        case PPO: return string("PPO"); 
        case INV: return string("INV"); 
        case BUF: return string("BUF"); 
        case AND: return string("AND"); 
        case AND2: return string("AND2"); 
        case AND3: return string("AND3"); 
        case AND4: return string("AND4"); 
        case NAND: return string("NAND"); 
        case NAND2: return string("NAND2"); 
        case NAND3: return string("NAND3"); 
        case NAND4: return string("NAND4"); 
        case OR: return string("OR"); 
        case OR2: return string("OR2"); 
        case OR3: return string("OR3"); 
        case OR4: return string("OR4"); 
        case NOR: return string("NOR"); 
        case NOR2: return string("NOR2"); 
        case NOR3: return string("NOR3"); 
        case NOR4: return string("NOR4"); 
        default: { 
           assert(0); 
           return string("");  
        }
    }
}

inline void Gate::print() const {
    cout << "#  ";
    cout << "id(" << id_ << ") ";
    cout << "lvl(" << lvl_ << ") ";
    cout << "type(" << type_ << ") ";
    cout << "frame(" << frame_ << ")";
    cout << endl;
    cout << "#    fi[" << nfi_ << "]";
    for (int j = 0; j < nfi_; ++j)
        cout << " " << fis_[j];
    cout << endl;
    cout << "#    fo[" << nfo_ << "]";
    for (int j = 0; j < nfo_; ++j)
        cout << " " << fos_[j];
    cout << endl << endl;
}

inline bool Gate::isFanoutStem() const {
    return (nfo_ > 1); 
}

inline Value Gate::isUnary() const {
    return nfi_ == 1 ? H : L;
}

inline Value Gate::isInverse() const {
    switch(type_) {
        case INV:
        case NAND: 
        case NAND2:
        case NAND3:
        case NAND4:
        case NOR: 
        case NOR2:
        case NOR3:
        case NOR4:
        case XNOR: 
        case XNOR2:
        case XNOR3:
            return H;
        default:
            return L;
    }
}

inline Value Gate::getInputNonCtrlValue() const {
    return isInverse() == getOutputCtrlValue() ? L : H;
}

inline Value Gate::getInputCtrlValue() const {
    return getInputNonCtrlValue() == H ? L : H;
}

inline Value Gate::getOutputCtrlValue() const {
    switch(type_) {
        case OR: 
        case OR2:
        case OR3:
        case OR4:
        case NAND: 
        case NAND2:
        case NAND3:
        case NAND4:
            return L;
        case PI: 
        case PPI: 
        case INV: 
        case BUF: 
        case PO: 
        case PPO: 
        case XOR: 
        case XOR2:
        case XOR3:
        case XNOR: 
        case XNOR2:
        case XNOR3:
            return X;
        default:
            return H;
    }
}

inline void PrintGateVec(const GateVec& gs) { // helper function for GDB  
    for (size_t i=0; i<gs.size(); i++) 
        cout << gs[i]->id_ << " "; 
    cout << endl; 
}

}; // CoreNs

#endif

