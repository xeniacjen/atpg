/*
 * =====================================================================================
 *
 *       Filename:  atpg.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/09/2016 08:06:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#ifndef _CORE_ATPG_H_ 
#define _CORE_ATPG_H_ 

#define _MAX_BACK_TRACK_LIMIT_  64

#include "implicator.h" 

namespace CoreNs { 

typedef std::pair<int, Value> Objective; 

class Atpg { 
public: 
    enum GenStatus             { TEST_FOUND = 0, UNTESTABLE, ABORT }; 

        Atpg(Circuit *cir, Fault *f); 
        ~Atpg(); 

    virtual GenStatus Tpg(); 
    void      GetPiPattern(Pattern *p); 

private: 
    bool isTestPossible(); 
    bool isaTest(); 

    Objective   current_obj_; 

protected:
    virtual bool FaultActivate(); 
    virtual bool DDrive(); 
    virtual bool Backtrace(); 

    void init(); 
    bool Imply(); 
    bool BackTrack(); 
    Gate *FindHardestToSetFanIn(Gate *g, Value obj) const; 
    Gate *FindEasiestToSetFanIn(Gate *g, Value obj) const; 

    Circuit     *cir_;
    Implicator  *impl_; 

    Fault       *current_fault_;

    unsigned    back_track_count; 
    unsigned    back_track_limit; 
}; //Atpg 

inline Atpg::Atpg(Circuit *cir, Fault *f) { 
    cir_ = cir;
    current_fault_ = f; 

    impl_ = new Implicator(cir, f); 
    init(); 
}

inline Atpg::~Atpg() { 
    delete impl_; 
}

inline void Atpg::GetPiPattern(Pattern *p) { 
    impl_->GetPiPattern(p);    
}
}; //CoreNs 

#endif //_CORE_ATPG_H_

