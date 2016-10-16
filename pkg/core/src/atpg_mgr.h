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

    LearnInfoMgr       *learn_mgr_; 

    bool                set_dfs_on_; 
    bool                set_oo_on_; 

    int                 req_dt_; 
    int                 req_pat_; 
private: 
    void                DynamicCompression(FaultList &remain); 
    void                ReverseFaultSim(); 
    void                RVEFaultSim(); 
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
        learn_mgr_    = NULL; 
        atpg_         = NULL;

        set_dfs_on_   = true; 
        set_oo_on_    = true; 

        req_dt_       = -1; 
    }

inline AtpgMgr::~AtpgMgr() {
    if (!fListExtract_) delete fListExtract_;
    if (!pcoll_) delete pcoll_;
    if (!cir_) delete cir_;
    if (!sim_) delete sim_;
    if (!learn_mgr_) delete learn_mgr_;
    if (!atpg_) delete atpg_;
}

inline void AtpgMgr::ConfigDynComp(int merge, int backtrack) { 
    dyn_comp_merge_ = (merge<0)?DYN_COMP_MERGE_DEFAULT:merge; 
    dyn_comp_backtrack = (backtrack<0)?DYN_COMP_BT_DEFAULT:backtrack;
} 
    
}; // CoreNs

#endif // _CORE_ATPG_MGR_H_
