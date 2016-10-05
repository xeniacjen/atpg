/*
 * =====================================================================================
 *
 *       Filename:  learn.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/04/2016 04:39:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#ifndef _CORE_LEARN_H_ 
#define _CORE_LEARN_H_ 

#include <map>

#include "implicator.h" 

namespace CoreNs { 

struct LearnInfo; 
typedef std::pair<int, Value> Objective; 
typedef std::vector<LearnInfo* > ImplyList; 
typedef std::map<Objective, LearnInfo *> LearnInfoList; 
typedef LearnInfoList::iterator LearnInfoListIter; 

struct LearnInfo { 
    ImplyList impl_objs_; 

    Objective obj_; 

    bool operator< (const LearnInfo& rhs) const { 
        return obj_ < rhs.obj_; 
    }
}; // LearnInfo 

class LearnInfoMgr { 
public: 
    LearnInfoMgr(Circuit *cir); 
    ~LearnInfoMgr(); 

    void GetLearnInfo(std::stack<Objective>& objs) const; 
    bool SetLearnInfo(const Objective& obj); 

private:  
    Circuit        *cir_;
    Implicator     *impl_; 

    LearnInfoList   learn_infos_; 
}; 

}; // CoreNs

#endif //_CORE_LEARN_H_ 
