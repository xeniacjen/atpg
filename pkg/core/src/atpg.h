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

#define _MAX_BACK_TRACK_LIMIT_  256

#include <map>

#include "implicator.h" 
#include "d_decision_tree.h"

namespace CoreNs { 

typedef std::pair<int, Value> Objective; 
typedef std::map<int, Value> ObjList; 

class Atpg { 
public: 
    enum GenStatus             { TEST_FOUND = 0, UNTESTABLE, ABORT }; 

        Atpg(Circuit *cir, Fault *f); 
        Atpg(Circuit *cir, Fault *f, Pattern *p); 
        ~Atpg(); 

    virtual GenStatus Tpg(); 
    void    GetPiPattern(Pattern *p); 
    bool    TurnOnPoMode(); 
    bool    TurnOnObjOptimMode(); 

    bool    CheckCompatibility(Fault *f);  

private: 
    bool DDDrive(); 
    bool MultiDDrive(); 
    bool isTestPossible(); 
    bool isaTest(); 

    bool MultiDBackTrack(); 

    bool        is_path_oriented_mode_; 
    bool        is_obj_optim_mode_; 
    ObjList     objs_; 
    Objective   current_obj_; 

protected:
    virtual bool FaultActivate(); 
    virtual bool DDrive(); 
    virtual bool Backtrace(); 

    bool GenObjs(); 

    bool DBackTrack(); 
    bool CheckPath(const GateVec &path) const; 

    bool CheckXPath(Gate *g); 
    bool CheckDPath(Gate *g) const; 
    bool CheckDFrontier(GateVec &dfront); 

    void ResetXPath(); 

    void init(); 
    void init(Pattern *p); 
    bool Imply(); 
    bool BackTrack(); 
    Gate *FindHardestToSetFanIn(Gate *g, Value obj) const; 
    Gate *FindEasiestToSetFanIn(Gate *g, Value obj) const; 

    Circuit    *cir_;
    Implicator *impl_; 

    Fault      *current_fault_;

    unsigned    back_track_count; 
    unsigned    back_track_limit; 

    DDTree      d_tree_; 
    Value      *x_path_; // keep the x-path search status 
}; //Atpg 

inline Atpg::Atpg(Circuit *cir, Fault *f) { 
    cir_ = cir;
    current_fault_ = f; 

    is_path_oriented_mode_ = false; 
    is_obj_optim_mode_ = false; 
    impl_ = new Implicator(cir, f); 
    x_path_ = new Value[cir_->tgate_]; 

    init(); 
}

inline Atpg::Atpg(Circuit *cir, Fault *f, Pattern *p) { 
    cir_ = cir;
    current_fault_ = f; 

    is_path_oriented_mode_ = false; 
    is_obj_optim_mode_ = false; 
    impl_ = new Implicator(cir, f); 
    x_path_ = new Value[cir_->tgate_]; 

    init(p); 
} 

inline Atpg::~Atpg() { 
    delete    impl_; 
    delete [] x_path_; 
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

inline bool Atpg::CheckDPath(Gate *g) const { 
    // get the previous object 
    GateVec gids; d_tree_.top(gids); // TODO: make a switch 
    for (size_t i=0; i<gids.size(); i++) { 
        if (g->id_==gids[i]->id_) { 
            return true; 
        }
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
    back_track_limit = _MAX_BACK_TRACK_LIMIT_; 

    return is_path_oriented_mode_; 
}

inline bool Atpg::TurnOnObjOptimMode() { 
    is_obj_optim_mode_ = true; 

    return is_obj_optim_mode_; 
} 

inline void Atpg::ResetXPath() { 
    for (int i=0; i<cir_->tgate_; i++) 
        x_path_[i] = X; 
}
  
}; //CoreNs 

#endif //_CORE_ATPG_H_

