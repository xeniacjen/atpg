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

    // TODO 

    delete impl; 
    delete dummy_fault; 

    return true; 
}
