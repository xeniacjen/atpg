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

#define RPT_PER_PAT             64 

#define DYN_COMP_MERGE_DEFAULT  10 
#define DYN_COMP_BT_DEFAULT     64 

namespace CoreNs {

typedef std::vector<Atpg*> AtpgVec; 

class AtpgMgr {
public: 
    AtpgMgr();  
    ~AtpgMgr();  

    void                generation(int limit); 

    void                ConfigDynComp(int merge, int backtrack); 
    
    FaultListExtract   *fListExtract_;
    PatternProcessor   *pcoll_;
    Circuit            *cir_;
    Simulator          *sim_;
private: 
    void                DynamicCompression(FaultList &remain); 
    void                ReverseFaultSim(); 
    void                XFill(); 

    void                getPoPattern(Pattern *pat);  
    void                calc_fault_hardness(Fault* f1); 

    Atpg               *atpg_; 

    int                 dyn_comp_merge_; 
    int                 dyn_comp_backtrack; 

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

inline void AtpgMgr::ConfigDynComp(int merge, int backtrack) { 
    dyn_comp_merge_ = (merge<0)?DYN_COMP_MERGE_DEFAULT:merge; 
    dyn_comp_backtrack = (backtrack<0)?DYN_COMP_BT_DEFAULT:backtrack;
} 
    
}; // CoreNs

#endif // _CORE_ATPG_MGR_H_
