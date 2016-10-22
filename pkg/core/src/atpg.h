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

#define _MAX_BACK_TRACK_LIMIT_  512

#include "b_decision_tree.h"
#include "d_decision_tree.h"

namespace CoreNs { 

typedef std::map<int, Value> ObjList; 
typedef ObjList::iterator ObjListIter; 

class Atpg { 
    friend struct comp_gate; 
    struct comp_gate { 
        comp_gate(Atpg *atpg) { atpg_ = atpg; }
        Atpg *atpg_; 

        bool operator()(Gate* g1, Gate* g2); 
    }; 
    
public: 
    enum GenStatus             { TEST_FOUND = 0, UNTESTABLE, ABORT }; 
    enum SetObjRet             { SUCCESS = 0, FAIL, NOCHANGE }; 

        Atpg(Circuit *cir, Fault *f); 
        Atpg(Circuit *cir, Fault *f, Pattern *p); 
        ~Atpg(); 

    bool    SetBackTrackLimit(int limit); 

    virtual GenStatus Tpg(); 
    void    GetPiPattern(Pattern *p); 

    bool    CheckCompatibility(Fault *f);  

    bool    TurnOnPoMode(); 
    bool    TurnOnObjOptimMode(FaultListExtract *fl); 

    void    SetLearnEngine(LearnInfoMgr *learn_mgr); 

    // int     prop_fs_;
    // int     prob_fs_; 

private: 
    // podem help function 
    bool isTestPossible(); 
    bool isaTest(); 

    // dfs-podem 
    bool init_d_tree(); 
    bool DDDrive(); 
    bool CheckDPath(Gate *g) const; 
    bool CheckPath(const GateVec &path) const; 
    bool DBackTrack(); 

    // obj-optim. help function 
    Value GetObj(int gid, const ObjList& objs); 
    SetObjRet SetObj(const Objective& obj, ObjList& objs);  
    void EvalObj(Objective& obj, ObjList& objs); 
    void PushFaninObjEvent(Gate *g, 
                           std::stack<Objective>& events); 
    void PushFanoutObjEvent(const Objective& obj, 
                            std::queue<Objective>& events_forward); 
    bool BackwardObjProp(Gate *gtoprop, 
                         ObjList& objs,  
                         std::queue<Objective>& events_forward); 
    bool ForwardObjProp(ObjList& objs, 
                        std::queue<Objective>& events_forward); 
    bool AddGateToProp(Gate *gtoprop); 
    bool AddUniquePathObj(Gate *gtoprop, std::queue<Objective>& events); 
    bool GenObjs(); 
    bool CheckDDDrive(); 
    bool MultiCheckDPath(Gate *g); 
    bool MultiDDrive(); 
    bool MultiDBackTrack(DecisionTree &tree); 
    bool isaMultiTest(); 

    void FindHardestToSetObj(Objective& obj); 
    void FindEasiestToSetObj(Objective& obj); 
    Gate *FindEasiestToSetFanInObj(Gate *g, Value obj); 
    Gate *FindHardestToSetFanInObj(Gate *g, Value obj); 
    bool BacktraceOO(); 
    bool BBackTrack(Gate *&g, Value& objv); 

    bool CheckXPathObj(Gate *g, const ObjList& objs); 
    void ResetXPathObj(); 

    // TODO: prob. no need 
    // void PropFaultSet(FaultSetMap &f2p, GateSetMap &pred); 
    void PropFaultSet(const GateVec &gv, GateSetMap &pred); 
    Fault *GetFault(Gate *g, int line); 
    Fault *GetProbFault(Gate *g, int line, Value vf); 
    void AddFaultSet(Gate *g, FaultSet &fs); 
    int GetProbFaultSet(Gate *g, Value vi); 

    bool        is_path_oriented_mode_; 
    bool        is_obj_optim_mode_; 
    ObjList     objs_; 

    int        *prob_fs; 

    Value      *x_path_obj_; 

    DDTree      d_tree_; 
    BDTree      b_tree_; 

    FaultListExtract *flist_; 
protected:
    virtual bool FaultActivate(); 
    virtual bool DDrive(); 
    virtual bool Backtrace(); 

    bool CheckXPath(Gate *g); 
    bool CheckDFrontier(GateVec &dfront); 

    void ResetXPath(); 
    void ResetProbFaultSet(); 
    void ResetFaultReach();   

    void init(); 
    void init(Pattern *p); 
    bool Imply(); 
    bool BackTrack(); 
    Gate *FindHardestToSetFanIn(Gate *g, Value obj) const; 
    Gate *FindEasiestToSetFanIn(Gate *g, Value obj) const; 

    void CalcIsFaultReach(const GateVec &gv); 

    Circuit    *cir_;
    Implicator *impl_; 

    LearnInfoMgr * learn_mgr_; 

    Fault      *current_fault_;

    unsigned    back_track_count; 
    unsigned    back_track_limit; 

    Objective   current_obj_; 
    Value      *x_path_; // keep the x-path search status 
    bool       *is_fault_reach_; 
}; //Atpg 

inline Atpg::Atpg(Circuit *cir, Fault *f) { 
    cir_ = cir;
    current_fault_ = f; 

    is_path_oriented_mode_ = false; 
    is_obj_optim_mode_ = false; 
    impl_ = new Implicator(cir, f); 
    x_path_ = new Value[cir_->tgate_]; 
    x_path_obj_ = new Value[cir_->tgate_]; 
    prob_fs = new int[cir_->tgate_]; 
    is_fault_reach_ = new bool[cir_->tgate_]; 

    learn_mgr_ = NULL; 

    init(); 
}

inline Atpg::Atpg(Circuit *cir, Fault *f, Pattern *p) { 
    cir_ = cir;
    current_fault_ = f; 

    is_path_oriented_mode_ = false; 
    is_obj_optim_mode_ = false; 
    impl_ = new Implicator(cir, f); 
    x_path_ = new Value[cir_->tgate_]; 
    x_path_obj_ = new Value[cir_->tgate_]; 
    prob_fs = new int[cir_->tgate_]; 
    is_fault_reach_ = new bool[cir_->tgate_]; 

    learn_mgr_ = NULL; 

    init(p); 
} 

inline Atpg::~Atpg() { 
    delete    impl_; 
    delete [] x_path_; 
    delete [] x_path_obj_; 
    delete [] prob_fs; 
    delete [] is_fault_reach_; 
}

inline void Atpg::GetPiPattern(Pattern *p) { 
    impl_->GetPiPattern(p);    
}

inline bool Atpg::CheckPath(const GateVec &path) const { 
    if (path.empty()) return true; 

    GateVec p = path; 

    // p.pop_back(); // pass the currently justifying gate 
    while (!p.empty()) { 
        Value v = impl_->GetVal(p.back()->id_); 
        if (v!=D && v!=B) return false; 

        p.pop_back(); 
    } 
    
    return true; 
}

inline bool Atpg::CheckXPath(Gate *g) { 
    if (impl_->GetVal(g->id_)!=X || x_path_[g->id_]==L) { 
        x_path_[g->id_] = L; 
        return false; 
    }

    if (g->type_==Gate::PO || g->type_==Gate::PPO || x_path_[g->id_]==H) { 
        x_path_[g->id_] = H; 
        return true; 
    }

    for (int i=0; i<g->nfo_; i++) { 
        Gate *fo = &cir_->gates_[g->fos_[i]]; 
        if (CheckXPath(fo)) { 
            x_path_[g->id_] = H; 
            return true; 
        }
    }
    
    x_path_[g->id_] = L; 
    return false; 
}

inline bool Atpg::CheckXPathObj(Gate *g, const ObjList& objs) { 
    if (GetObj(g->id_, objs)!=X || x_path_obj_[g->id_]==L) { 
        x_path_obj_[g->id_] = L; 
        return false; 
    }

    if (g->type_==Gate::PO || g->type_==Gate::PPO || x_path_obj_[g->id_]==H) { 
        x_path_obj_[g->id_] = H; 
        return true; 
    }

    for (int i=0; i<g->nfo_; i++) { 
        Gate *fo = &cir_->gates_[g->fos_[i]]; 
        if (CheckXPathObj(fo, objs)) { 
            x_path_obj_[g->id_] = H; 
            return true; 
        }
    }
    
    x_path_obj_[g->id_] = L; 
    return false; 
    
}

inline bool Atpg::CheckDPath(Gate *g) const { 
    // get the previous object 
    if (is_obj_optim_mode_) { 
        GateVec gids; d_tree_.top(gids); // TODO: make a switch 
        for (size_t i=0; i<gids.size(); i++) { 
            if (g->id_==gids[i]->id_) { 
                return true; 
            }
        }
    } 
    else { 
        int gid; d_tree_.top(gid); 
        if (g->id_==gid) return true; 
    }

    for (int i=0; i<g->nfi_; i++) { 
        Gate *fi = &cir_->gates_[g->fis_[i]]; 
        Value vi = impl_->GetVal(fi->id_); 
        if (vi!=D && vi!=B) continue; 
        if (CheckDPath(fi)) { 
            return true; 
        }
    }
    
    return false; 
}

inline bool Atpg::TurnOnPoMode() { 
    is_path_oriented_mode_ = true; 

    return is_path_oriented_mode_; 
}

inline bool Atpg::TurnOnObjOptimMode(FaultListExtract *fl) { 
    is_path_oriented_mode_ = true; 
    is_obj_optim_mode_ = true; 

    flist_ = fl; 

    return is_obj_optim_mode_; 
} 

inline void Atpg::ResetXPath() { 
    for (int i=0; i<cir_->tgate_; i++) 
        x_path_[i] = X; 
}

inline void Atpg::ResetXPathObj() { 
    for (int i=0; i<cir_->tgate_; i++) { 
        if (x_path_[i]==L) x_path_obj_[i] = L; 
        else x_path_obj_[i] = X; 
    }
}

inline void Atpg::ResetProbFaultSet() { 
    for (int i=0; i<cir_->tgate_; i++) 
        prob_fs[i] = -1; 
}

inline void Atpg::ResetFaultReach() { 
    for (int i=0; i<cir_->tgate_; i++) 
        is_fault_reach_[i] = false; 
}
  
inline bool Atpg::SetBackTrackLimit(int limit) { 
    back_track_limit = limit; 
}

inline Value Atpg::GetObj(int gid, const ObjList& objs) { 
    Value vg = impl_->GetVal(gid); 
    if (vg==X) { 
        ObjList::const_iterator it = objs.find(gid); 
        if (it==objs.end()) return X; 
        return it->second; 
    }
    else return vg; 
} 

inline Atpg::SetObjRet Atpg::SetObj(const Objective& obj, ObjList& objs) { 
    int gid = obj.first; Value vo = obj.second; 
    Value vg = impl_->GetVal(gid); 
    if (vg==X) { 
        std::pair<ObjListIter, bool> ret = objs.insert(obj); 
        if (!ret.second && ret.first->second!=vo)  
            return FAIL; 
        else if (!ret.second && ret.first->second==vo)  
            return NOCHANGE; 
        else return SUCCESS; 
    } 
    else { 
        if (vg==EvalNot(vo)) return FAIL; 
        return NOCHANGE; 
    }
}

inline void Atpg:: PushFaninObjEvent(Gate *g, 
                                      std::stack<Objective>& events) { 

    Objective obj; 
    for (int i=0; i<g->nfi_; i++) { 
        if (is_fault_reach_[g->fis_[i]]) continue; 
        obj.first = g->fis_[i]; 
        obj.second = g->getInputNonCtrlValue(); 
        events.push(obj); 
    }
} 

inline void Atpg::EvalObj(Objective& obj, ObjList& objs) { 
    if (obj.second!=X) return; 

    Gate *g = &cir_->gates_[obj.first]; 
    for (int i=0; i<g->nfi_; i++) { 
        if (GetObj(g->fis_[i], objs)!=g->getInputNonCtrlValue()) 
            return; 
    }

    obj.second = g->getOutputCtrlValue(); 
} 

inline void Atpg::SetLearnEngine(LearnInfoMgr *learn_mgr) { 
    learn_mgr_ = learn_mgr; 
}

}; //CoreNs 

#endif //_CORE_ATPG_H_

