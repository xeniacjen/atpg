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

#include "common/tm_usage.h"

#include "atpg.h" 

#define STR_VAL(str)    #str 
#define LOGFILE         STR_VAL(atpg.log)
#define PATFILE         STR_VAL(atpg.pat)

#define RPT_PER_PAT     64 

namespace CoreNs {

typedef std::vector<Atpg*> AtpgVec; 

class AtpgMgr {
public: 
    AtpgMgr();  
    ~AtpgMgr();  

    void                generation(CommonNs::TmUsage &tmusg); 
    
    FaultListExtract    *fListExtract_;
    PatternProcessor    *pcoll_;
    Circuit             *cir_;
    Simulator           *sim_;
private: 
    void                DynamicCompression(FaultList &remain); 
    void                ReverseFaultSim(); 
    void                XFill(); 

    void                getPoPattern(Pattern *pat);  
    void                calc_fault_hardness(Fault* f1); 

    void                printHeader(size_t n, 
                                    const std::string& name); 
    void                printLogger(CommonNs::TmUsage &tmusg); 

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
