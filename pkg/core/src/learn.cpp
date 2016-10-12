/*
 * =====================================================================================
 *
 *       Filename:  learn.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/05/2016 02:16:51 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include "learn.h"

using namespace std; 

using namespace CoreNs; 

void LearnInfoMgr::StaticLearn() { 
    for (int i=0; i<cir_->tgate_; i++) { 
        Objective obj; 
        obj.first = i; 
        obj.second = L; 
        SetLearnInfo(obj); 

        obj.second = H; 
        SetLearnInfo(obj); 
    }
}  

LearnInfoMgr::LearnInfoMgr(Circuit *cir) { 
    cir_ = cir; 
}

LearnInfoMgr::~LearnInfoMgr() { 
    LearnInfoListIter it = learn_infos_.begin(); 
    for (; it!=learn_infos_.end(); ++it)  
        delete it->second; 
}

void LearnInfoMgr::GetLearnInfo(stack<Objective>& objs) const { 
    stack<Objective> events = objs; 

    while (!events.empty()) { 
        Objective obj = events.top(); 
        events.pop(); 

        LearnInfoList::const_iterator it = learn_infos_.find(obj); 
        if (it==learn_infos_.end()) continue; 

        for (int i=0; i<it->second->impl_objs_.size(); i++) { 
            objs.push(it->second->impl_objs_[i]->obj_); 
            events.push(it->second->impl_objs_[i]->obj_); 
        }
    }
} 

bool LearnInfoMgr::SetLearnInfo(const Objective& obj) { 
    Fault *dummy_fault = new Fault(); 
    Implicator *impl = new Implicator(cir_, dummy_fault); 
    impl->Init(); 

    impl->AssignValue(obj.first, obj.second); 
    // if (!impl->EventDrivenSimB()) { 
    //     delete impl; 
    //     delete dummy_fault; 
    //     return false; 
    // }
    if (!impl->EventDrivenSim()) { 
        delete impl; 
        delete dummy_fault; 
        return false; 
    }

    for (int i=0; i<impl->GetEFrontierSize(); i++) { 
        Objective impl_obj; 
        impl_obj.first = impl->e_front_list_[i]; 
        impl_obj.second = impl->GetVal(impl_obj.first); 
        if (obj.first==impl_obj.first) continue; 
        if (isWorthLearn(obj, impl_obj, impl)) 
            addImplObj(obj, impl_obj); 
    }

    delete impl; 
    delete dummy_fault; 

    return true; 
}

bool LearnInfoMgr::isWorthLearn(const Objective& obj, 
                                const Objective& impl_obj,  
                                Implicator* impl) { 

    Gate *g = &cir_->gates_[impl_obj.first]; 
    if (g->getOutputCtrlValue()!=impl_obj.second) return false; 
    if (CheckIfHasDirectPass(obj, impl_obj)) return false; 

    return true; 
} 

bool LearnInfoMgr::addImplObj(Objective obj, 
                              Objective impl_obj) {  

    impl_obj.second = EvalNot(impl_obj.second); 
    LearnInfoListIter it_impl; getLearnInfo(impl_obj, it_impl); 

    obj.second = EvalNot(obj.second); 
    LearnInfoListIter it; getLearnInfo(obj, it); 
    it_impl->second->impl_objs_.push_back(it->second); 

    return true; 
}

bool LearnInfoMgr::getLearnInfo(const Objective& obj, 
                   LearnInfoListIter& it) { 

    it = learn_infos_.find(obj); 
    if (it==learn_infos_.end()) { 
        LearnInfo *learn_info = new LearnInfo(); 
        learn_info->obj_ = obj; 

        pair<LearnInfoListIter, bool> ret 
          = learn_infos_.insert(make_pair(obj, learn_info));  
        it = ret.first; 
        return false; 
    } 

    return true; 
} 

bool LearnInfoMgr::CheckIfHasDirectPass(Objective obj, 
                                        Objective impl_obj) const { 

    // TODO 
    Gate *g = &cir_->gates_[impl_obj.first]; 
    while(g->isUnary() && g->nfi_>0) {  
        if (g->id_==obj.first) return true; 

        g = &cir_->gates_[g->fis_[0]]; 
    }

    return false; 
} 

