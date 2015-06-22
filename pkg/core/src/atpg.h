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

#define _MAX_BACK_TRACK_LIMIT_   256

#include "implicator.h" 

namespace CoreNs { 

typedef std::pair<int, Value> Objective; 

class Atpg { 
public: 
    enum GenStatus             { TEST_FOUND = 0, UNTESTABLE, ABORT }; 
    enum AtpgStatus            { IMPLY_AND_CHECK = 0, 
                                 DECISION, 
                                 BACKTRACE, 
                                 BACKTRACK,  
                                 EXIT }; 

        Atpg(Circuit *cir, Fault *f); 
        ~Atpg(); 

    GenStatus Tpg(); 

    void assignPatternPiValue(Pattern *pat); // write PI values to pattern
    void assignPatternPoValue(Pattern *pat); // write PO values to pattern

protected:
    Circuit     *cir_;
    Implicator  *impl_; 

    Fault       *current_fault_;

}; //Atpg 

inline Atpg::Atpg(Circuit *cir, Fault *f) { 
    cir_ = cir;
    impl_ = new Implicator(cir, f); 
}

}; //CoreNs 

#endif //_CORE_ATPG_H_

