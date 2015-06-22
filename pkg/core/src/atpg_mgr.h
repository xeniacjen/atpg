/*
 * =====================================================================================
 *
 *       Filename:  atpg_mgr.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/17/2015 02:53:37 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#ifndef _CORE_ATPG_MGR_H_
#define _CORE_ATPG_MGR_H_ 

#include "atpg.h" 
#include "simulator.h" 

namespace CoreNs {

typedef std::vector<Atpg*> AtpgVec; 

class AtpgMgr {
public: 
    AtpgMgr();  
    ~AtpgMgr();  
    
    FaultListExtract    *fListExtract_;
    PatternProcessor    *pcoll_;
    Circuit             *cir_;
    Simulator           *sim_;
    Atpg                *atpg_; 

    //AtpgVec             atpgs_; 
}; 
 
inline AtpgMgr::AtpgMgr() {
        fListExtract_ = NULL;
        pcoll_        = NULL;
        cir_          = NULL;
        sim_          = NULL;
        atpg_         = NULL;
    }

inline AtpgMgr::~AtpgMgr() {}
    
}; // CoreNs

#endif // _CORE_ATPG_MGR_H_
