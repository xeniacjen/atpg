/*
 * =====================================================================================
 *
 *       Filename:  logic-hex.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/24/2016 08:53:46 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#ifndef _CORE_LOGIC_HEX_H_
#define _CORE_LOGIC_HEX_H_ 

#include "logic.h" 

namespace CoreNs {
struct HexValue {
    bool L_; 
    bool B_; 
    bool D_; 
    bool H_; 

    HexValue() {
        L_ = true; 
        B_ = true; 
        D_ = true; 
        H_ = true; 
    }

    HexValue(Value v); 

    bool isSubset(const HexValue& hv) const; 

    HexValue& operator=(HexValue hv);  
    friend bool operator==(const HexValue& lhs, const HexValue& rhs); 
    friend bool operator!=(const HexValue& lhs, const HexValue& rhs); 
}; 

inline HexValue EvalNotHex(HexValue a) {
    HexValue ret; 

    ret.L_ = a.H_; 
    ret.B_ = a.D_; 
    ret.D_ = a.B_; 
    ret.H_ = a.L_; 

    return ret; 
}

inline HexValue EvalAndHex(HexValue a, HexValue b) {
    HexValue ret; 

    ret.L_ = a.L_ | b.L_ | (a.D_&b.B_) | (a.B_&b.D_); 
    ret.B_ = (a.H_&b.B_) | (a.B_&b.H_) | (a.B_&b.B_); 
    ret.D_ = (a.H_&b.D_) | (a.D_&b.H_) | (a.D_&b.D_); 
    ret.H_ = a.H_&b.H_; 

    return ret; 
}

inline HexValue EvalOrHex(HexValue a, HexValue b) {
    HexValue ret; 

    ret.L_ = a.L_&b.L_; 
    ret.B_ = (a.L_&b.B_) | (a.B_&b.L_) | (a.B_&b.B_); 
    ret.D_ = (a.L_&b.D_) | (a.D_&b.L_) | (a.D_&b.D_); 
    ret.H_ = a.H_ | b.H_ | (a.D_&b.B_) | (a.B_&b.D_); 

    return ret; 
}

inline HexValue EvalAndNHex(std::vector<HexValue>& vals) {
    if(vals.size()<2) assert(0); 

    HexValue ret = EvalAndHex(vals[0], vals[1]); 
    for (size_t n=2; n<vals.size(); n++) 
        ret = EvalAndHex(ret, vals[n]); 

    return ret; 
}

inline HexValue EvalNandNHex(std::vector<HexValue>& vals) {
    return EvalNotHex(EvalAndNHex(vals)); 
}

inline HexValue EvalOrNHex(std::vector<HexValue>& vals) {
    if(vals.size()<2) assert(0); 

    HexValue ret = EvalOrHex(vals[0], vals[1]); 
    for (size_t n=2; n<vals.size(); n++) 
        ret = EvalOrHex(ret, vals[n]); 

    return ret; 
}

inline HexValue EvalNorNHex(std::vector<HexValue>& vals) {
    return EvalNotHex(EvalOrNHex(vals)); 
}

inline HexValue::HexValue(Value v) {
    L_ = false; 
    B_ = false; 
    D_ = false; 
    H_ = false; 
    
    switch (v) {
        case L: 
            L_ = true; 
            break; 
        case B: 
            B_ = true; 
            break; 
        case D: 
            D_ = true; 
            break; 
        case H: 
            H_ = true; 
            break; 
        default: 
            L_ = true; 
            B_ = true; 
            D_ = true; 
            H_ = true; 
            break;  
    }
}

inline HexValue& HexValue::operator=(HexValue hv) {
    L_ = hv.L_; 
    B_ = hv.B_; 
    D_ = hv.D_; 
    H_ = hv.H_; 

    return *this; 
} 


inline bool operator==(const HexValue& lhs, const HexValue& rhs) {
    if ((lhs.L_!=rhs.L_)
      ||(lhs.B_!=rhs.B_)
      ||(lhs.D_!=rhs.D_)
      ||(lhs.H_!=rhs.H_)) 
        return false;  

    return true; 
} 

inline bool operator!=(const HexValue& lhs, const HexValue& rhs) {
    return !(lhs == rhs); 
} 

inline bool HexValue::isSubset(const HexValue& hv) const {
    if ((hv.L_==false && L_==true) 
      ||(hv.B_==false && B_==true)
      ||(hv.D_==false && D_==true)
      ||(hv.H_==false && H_==true)) 
        return false; 
    
    return true; 
} 

};//  CoreNs

#endif // _CORE_LOGIC_HEX_H_  
