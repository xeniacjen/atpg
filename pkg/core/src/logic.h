// **************************************************************************
// File       [ logic.h ]
// Author     [ littleshamoo ]
// Synopsis   [ Logic representation and operation ]
// Date       [ 2010/12/14 created ]
// **************************************************************************

#ifndef _CORE_LOGIC_H_
#define _CORE_LOGIC_H_

#include <iostream>
#include <cassert>
#include <vector>

namespace CoreNs {

// type defines
// typedef uint8_t   Value;
// typedef uintptr_t ParaValue;
typedef unsigned char Value;
typedef unsigned long ParaValue;

// constant single logic
const   Value     L         = 0;       // Low
const   Value     H         = 1;       // High
const   Value     X         = 2;       // Unknown
const   Value     D         = 3;       // D (good 1 / faulty 0)
const   Value     B         = 4;       // D-bar (good 0 / faulty 1)
const   Value     Z         = 5;       // High-impedance
const   Value     I         = 255;     // Invalid

// constant multi-bit logic
const   ParaValue PARA_L    = 0;       // all bits are zero
const   ParaValue PARA_H    = ~PARA_L; // all bits are one

// determine word size
const   size_t    BYTE_SIZE = 8;
const   size_t    WORD_SIZE = sizeof(ParaValue) * BYTE_SIZE;

const Value NotMap[5] = { H, L, X, B, D };

const Value AndMap[5][5] = { 
        { L, L, L, L, L }, 
        { L, H, X, D, B }, 
        { L, X, X, X, X }, 
        { L, D, X, D, L }, 
        { L, B, X, L, B }
     };

const Value OrMap[5][5] = { 
        { L, H, X, D, B }, 
        { H, H, H, H, H }, 
        { X, H, X, X, X }, 
        { D, H, X, D, H }, 
        { B, H, X, H, B }
     };

const Value XorMap[5][5] = {  
        { L, H, X, D, B }, 
        { H, L, X, B, D }, 
        { X, X, X, X, X }, 
        { D, B, X, L, H }, 
        { B, D, X, H, L }
     };
	
const Value NandMap[5][5] = { 
        { H, H, H, H, H }, 
        { H, L, X, B, D }, 
        { H, X, X, X, X }, 
        { H, B, X, B, H }, 
        { H, D, X, H, D }
     };

const Value NorMap[5][5] = { 
        { H, L, X, B, D }, 
        { L, L, L, L, L }, 
        { X, L, X, X, X }, 
        { B, L, X, B, L }, 
        { D, L, X, L, D }
     };

const Value XnorMap[5][5] = { 
        { H, L, X, B, D }, 
        { L, H, X, D, B }, 
        { X, X, X, X, X }, 
        { B, D, X, H, L }, 
        { D, B, X, L, H }
     };


inline void setBitValue(ParaValue &pv, const size_t &i, const Value &v) {
    pv = v == L ? pv & ~((ParaValue)0x01 << i) : pv | ((ParaValue)0x01 << i);
}

inline Value getBitValue(const ParaValue &pv, const size_t &i) {
    return (pv & ((ParaValue)0x01 << i)) == PARA_L ? L : H;
}

inline Value EvalNot(Value v){
	return NotMap[v];
}

inline Value EvalAndN(std::vector<Value>& vals) {
    if(vals.size()<2) return X; 

    Value ret = AndMap[vals[0]][vals[1]]; 
    for(size_t n=2; n<vals.size(); n++) 
        ret = AndMap[ret][vals[n]]; 

    return ret; 
}

inline Value EvalOrN(std::vector<Value>& vals) {
    if(vals.size()<2) return X; 

    Value ret = OrMap[vals[0]][vals[1]]; 
    for(size_t n=2; n<vals.size(); n++) 
        ret = OrMap[ret][vals[n]]; 

    return ret; 
}

inline Value EvalXorN(std::vector<Value>& vals) {
    if (vals.size()<2) assert(0);  

    Value ret = XorMap[vals[0]][vals[1]]; 
    for (size_t n=2; n<vals.size(); n++) 
        ret = XorMap[ret][vals[n]]; 

    return ret; 
}

inline Value EvalNandN(std::vector<Value>& vals) {
    return EvalNot(EvalAndN(vals)); 
}

inline Value EvalNorN(std::vector<Value>& vals) {
    return EvalNot(EvalOrN(vals)); 
}

inline Value EvalXnorN(std::vector<Value>& vals) {
    return EvalNot(EvalXorN(vals)); 
}

void printValue(const Value &v, std::ostream &out = std::cout);
void printValue(const ParaValue &v, std::ostream &out = std::cout);
void printValue(const ParaValue &l, const ParaValue &h,
                std::ostream &out = std::cout);

};

#endif

