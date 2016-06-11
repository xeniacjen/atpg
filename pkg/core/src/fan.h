/*
 * =====================================================================================
 *
 *       Filename:  fan.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/10/2016 04:12:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#ifndef _CORE_FAN_H_
#define _CORE_FAN_H_ 

#include "atpg.h"

namespace CoreNs { 

class Fan : public Atpg{
public: 
        Fan(Circuit *cir, Fault *f) : Atpg(cir, f) { } 
        ~Fan(); 

    Atpg::GenStatus Tpg();  
private:
    bool back_trace_flag;  // true if continuous back-trace 

    std::vector<Objective> init_objs_; 
    std::vector<Objective> curr_objs_; 
    std::vector<Objective> fanout_objs_; 
    std::vector<Objective> final_objs_; 

protected: 
    bool FaultActivate(); 
    bool DDrive(); 
    bool Backtrace(); 

    
}; // Fan 

} // CoreNs

#endif // _CORE_FAN_H_
