/*
 * =====================================================================================
 *
 *       Filename:  fan.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/10/2016 10:46:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include "fan.h" 

using namespace std; 
using namespace CoreNs; 

Atpg::GenStatus Fan::Tpg() {
    if (!FaultActivate()) return UNTESTABLE ; 
    back_trace_flag = false; 
    while (true) {
        Imply(); 
    }
}

bool Fan::FaultActivate() {
    Gate *fg = &cir_->gates_[current_fault_->gate_]; 
    int fline = current_fault_->line_; 
   
    Value v = (current_fault_->type_==Fault::SA0 
        || current_fault_->type_==Fault::STR)?H:L;
    if (fline) { // input stuck fault on fan-out branch. 
        Gate *g = &cir_->gates_[fg->fis_[fline-1]]; 
        return impl_->MakeDecision(g, v); 
    } 
    else { // output stuck fault on P/PI. 
        return impl_->MakeDecision(fg, v); 
    }  
}
