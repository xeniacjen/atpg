// **************************************************************************
// File       [ pattern.h ]
// Author     [ littleshamoo ]
// Synopsis   [ ]
// Date       [ 2011/09/13 created ]
// **************************************************************************

#ifndef _CORE_PATTERN_H_
#define _CORE_PATTERN_H_

#include <cstdlib>
#include <iostream>
#include <vector> 
#include "clique_partition.h"
#include "circuit.h"
#include "fault.h"

using namespace std; 

namespace CoreNs {

class Pattern;

typedef std::vector<Pattern *> PatternVec;

class Pattern {
public:
          Pattern();
          ~Pattern();
    Value *pi1_;
    Value *pi2_;
    Value *ppi_;
    Value *si_;
    Value *po1_;
    Value *po2_;
    Value *ppo_;

};

// This class process the test pattern set
//
// it supports :
//  X-filling
//  static compression
class PatternProcessor {
public:
    // Three types of patterns
	// BASIC_SCAN is for single time frame, stuck-at fault
	// LAUNCH_CAPTURE is for 2-pattern test , launch-on-capture
	// LAUNCH_SHIFT is for 2-pattern test, launch-on-shift
    enum       Type { BASIC_SCAN = 0, LAUNCH_CAPTURE, LAUNCH_SHIFT };
	
               PatternProcessor();
               ~PatternProcessor();

	//  pattern set attribute
	//  dynamic compression should be moved to ATPG  (E.4)
	//  static compression and x fill should be rewritten as two independent methods (A.1)
	//
	enum	   State { OFF = 0, ON };		   
	State	   staticCompression_; // Flag; ON = need static compression
	State	   dynamicCompression_; // Flag; ON = need dynamic compression
	State	   XFill_;//  Flag;  On = need X-Filling	
			   
    Type       type_;
    int        npi_;
    int        nppi_;
    int        nsi_;
    int        npo_;

    size_t     npat_hard_; // #pattern detecting hard-to-detect fault 
    size_t     nbit_spec_; // total specified bit counts
    size_t     nbit_spec_max; // max. specified bit counts among patterns 
    PatternVec pats_;

    int        *piOrder_;
    int        *ppiOrder_;
    int        *poOrder_;

    void       BuildGraph(); 
    void       ClearGraph(); 

    void       init(Circuit *cir);
	void 	   StaticCompression();
	void 	   StaticCompressionGraph();
    void       PrintPorts() const; 
    void       PrintPatterns() const; 
    void       PrintPattern(unsigned i) const; 

    void       randomFill(Pattern* pat);  

    bool       IsCompatible(size_t i, size_t j) const; 
    bool       Merge(size_t i, size_t j);  
};

inline Pattern::Pattern() {
    pi1_ = NULL;
    pi2_ = NULL;
    ppi_ = NULL;
    si_  = NULL;
    po1_ = NULL;
    po2_ = NULL;
    ppo_ = NULL;
}

inline Pattern::~Pattern() {
    delete [] pi1_;
    delete [] pi2_;
    delete [] ppi_;
    delete [] si_;
    delete [] po1_;
    delete [] po2_;
    delete [] ppo_;
}

inline PatternProcessor::PatternProcessor() {
	
	staticCompression_ = OFF; //  
	dynamicCompression_ = OFF; // dynamic compression should be moved to ATPG 
	XFill_ = OFF; //  
	
    type_     = BASIC_SCAN;
    npi_      = 0;
    nppi_     = 0;
    nsi_      = 0;
    npo_      = 0;
    piOrder_  = NULL;
    ppiOrder_ = NULL;
    poOrder_  = NULL;
    
    nbit_spec_= 0; 
    nbit_spec_max = 0; 
    npat_hard_ = 0; 
}

inline PatternProcessor::~PatternProcessor() {
    if (pats_.size() > 0)
        delete [] pats_[0];
    pats_.clear();
    delete [] piOrder_;
    delete [] ppiOrder_;
    delete [] poOrder_;
}

inline void PatternProcessor::init(Circuit *cir) {
    delete [] piOrder_;
    delete [] ppiOrder_;
    delete [] poOrder_;
    piOrder_ = NULL;
    ppiOrder_ = NULL;
    poOrder_ = NULL;

    npi_ = cir->npi_;
    npo_ = cir->npo_;
    nppi_ = cir->nppi_;

    piOrder_ = new int[npi_];
    for (int i = 0; i < npi_; ++i)
        piOrder_[i] = i;

    poOrder_ = new int[npo_];
    for (int i = 0; i < npo_; ++i)
        poOrder_[i] = cir->ngate_ - cir->npo_ - cir->nppi_ + i;

    ppiOrder_ = new int[nppi_];
    for (int i = 0; i < nppi_; ++i)
        ppiOrder_[i] = cir->npi_ + i;
}

inline void PatternProcessor::PrintPorts() const {
    cout << "#    pi order: ";
    for (int i = 0; i < npi_; ++i)
        cout << " " << piOrder_[i];
    cout << endl;
    cout << "#    ppi order:";
    for (int i = 0; i < nppi_; ++i)
        cout << " " << ppiOrder_[i];
    cout << endl;
    cout << "#    po order: ";
    for (int i = 0; i < npo_; ++i)
        cout << " " << poOrder_[i];
    cout << endl;
} 

inline void PatternProcessor::PrintPatterns() const {
    for (int i = 0; i < (int)pats_.size(); ++i) {
        cout << "#    pattern " << i << endl;
        PrintPattern(i); 
        cout << endl << "#" << endl;
    }
} 

inline void PatternProcessor::PrintPattern(unsigned i) const {
    cout << "#      pi1: ";
    if (pats_[i]->pi1_)
        for (int j = 0; j < npi_; ++j)
            printValue(pats_[i]->pi1_[j]);
    cout << endl;
    cout << "#      pi2: ";
    if (pats_[i]->pi2_)
        for (int j = 0; j < npi_; ++j)
            printValue(pats_[i]->pi2_[j]);
    cout << endl;
    cout << "#      ppi: ";
    if (pats_[i]->ppi_)
        for (int j = 0; j < nppi_; ++j)
            printValue(pats_[i]->ppi_[j]);
    cout << endl;
    cout << "#      po1: ";
    if (pats_[i]->po1_)
        for (int j = 0; j < npo_; ++j)
            printValue(pats_[i]->po1_[j]);
    cout << endl;
    cout << "#      po2: ";
    if (pats_[i]->po2_)
        for (int j = 0; j < npo_; ++j)
            printValue(pats_[i]->po2_[j]);
    cout << endl;
    cout << "#      ppo: ";
    if (pats_[i]->ppo_)
        for (int j = 0; j < nppi_; ++j)
            printValue(pats_[i]->ppo_[j]);
} 

// **************************************************************************
// Function   [ PatternProcessor::StaticCompressionGraph ]
// Commentor  [ HKY CYW ]
// Synopsis   [ usage: do static compression
// 		We first compare each pair of the patterns and check whether they are compatible.
// 		If so, we will merge these patterns bit by bit
// 		Note that
// 		Value L = 0
// 		Value H = 1
// 		Value X = 3
// 		And the rule of compression
// 		(X,L) -> L
// 		(X,H) -> H
//		So when we merge two patterns, always let the bit to be the smaller one.
//		That is, always don't use X as a result if one bit is L or H.
//              in:    Pattern list
//              out:   void
//            ]
// Date       [ HKY Ver. 1.0 started 2014/09/01 ]
// **************************************************************************

inline void PatternProcessor::StaticCompression() { 
    bool mergeRecord[(int)pats_.size()]; 
    for (size_t i=0; i<pats_.size(); i++) 
        mergeRecord[i] = false; 


    for (size_t i=0; i<pats_.size(); i++) { 
        if (mergeRecord[i]) continue; 
        for (size_t j=0; j<pats_.size(); j++) { 
            if (mergeRecord[j] || i==j) continue; 
            if (IsCompatible(i, j)) { 
                Merge(i, j); 
                mergeRecord[j] = true; 
            }
        }
    }

    PatternVec comp_pats; 
    for (size_t i=0; i<pats_.size(); i++) { 
        if (!mergeRecord[i])  
            comp_pats.push_back(pats_[i]); 
    }

    pats_ = comp_pats; 
}

inline void PatternProcessor::StaticCompressionGraph() {
    CliquePartition cp; 

    CompGraph compat; 

    // size_t matrix_mem = 0; 
    // size_t list_mem = 0; 
    for (size_t i=0; i<pats_.size(); i++) { 
        for (size_t j=i+1; j<pats_.size(); j++) { 
            // matrix_mem++; 
            if (IsCompatible(i, j)) { 
                compat.add_edge(i, j); 
                // list_mem++; 
            }
        } 
    }
    // cout << "Comp.rate = " 
    //      << (float)list_mem/(float)matrix_mem << endl;  

    cp.clique_partition(compat, pats_.size()); 

    std::map<int, Pattern *> comp_pats;  
    for (size_t i=0; i<cp.clique_set.size(); i++) { 
        int merged_pat = -1; 
        int pat_quality = 0; 
        for (int j=0; j<cp.clique_set[i].members.size(); j++) { 
            merged_pat = cp.clique_set[i].members[j]; 
            pat_quality = (merged_pat>pat_quality)?merged_pat:pat_quality;; 
            if (j>0) { 
                Merge(cp.clique_set[i].members[j], 
                      cp.clique_set[i].members[j-1]); 
            }
        } 
        comp_pats.insert(std::make_pair(pat_quality, pats_[merged_pat])); 
    }

    PatternVec comp_pat_vec; 
    std::map<int, Pattern *>::iterator it = comp_pats.begin();  
    for (; it!=comp_pats.end(); ++it) 
        comp_pat_vec.push_back(it->second); 

    pats_ = comp_pat_vec; 
}

inline void PatternProcessor::randomFill(Pattern *pat){
    size_t nbit_spec = 0; 

    srand(0);
    for( int i = 0 ; i < npi_ ; i++ ) { 
        if( pat->pi1_[i] == X ) { 
            pat->pi1_[i] = rand()%2; 
        }
        else nbit_spec++; 
    } 
    for( int i = 0 ; i < nppi_ ; i++ ) { 
        if( pat->ppi_[i] == X ) { 
            pat->ppi_[i] = rand()%2; 
        }
        else nbit_spec++; 
    }
	if(pat->pi2_!=NULL) { 
		for( int i = 0 ; i < npi_ ; i++ ) { 
			if( pat->pi2_[i] == X ) { 
				pat->pi2_[i] = rand()%2;
            } 
            else nbit_spec++; 
        }
    }
	if(pat->si_!=NULL) { 
		if( pat->si_[0] == X ) { 
			pat->si_[0] = rand()%2; 
        }
        else nbit_spec++; 
    }

    nbit_spec_+=nbit_spec; 
    if (nbit_spec>nbit_spec_max) 
        nbit_spec_max = nbit_spec; 
}

inline bool PatternProcessor::IsCompatible(size_t i, size_t j) const { 
	bool compatible = true;
	for (int k=0; k<npi_; ++k){//If any bit of the patterns has different values(one is high and one is low), the patterns are not compatible
		if( ((pats_[i]->pi1_[k]==L)&&(pats_[j]->pi1_[k]==H)) || ((pats_[i]->pi1_[k]==H)&&(pats_[j]->pi1_[k]==L)) ){
			compatible = false;
			break;
		}
	}
	if ((pats_[i]->pi2_!= NULL)&&(compatible == true)){//If the pattern has second primary input, we have to check it too
		for (int k=0; k<npi_; ++k){
			if( ((pats_[i]->pi2_[k]==L)&&(pats_[j]->pi2_[k]==H)) || ((pats_[i]->pi2_[k]==H)&&(pats_[j]->pi2_[k]==L)) ){
				compatible = false;
				break;
			}
		}
	}
	if ((pats_[i]->ppi_!= NULL)&&(compatible == true)){//Check ppi
		for (int k=0; k<nppi_; ++k){
			if( ((pats_[i]->ppi_[k]==L)&&(pats_[j]->ppi_[k]==H)) || ((pats_[i]->ppi_[k]==H)&&(pats_[j]->ppi_[k]==L)) ){
				compatible = false;
				break;
			}
		}
	}
	if ((pats_[i]->si_!= NULL)&&(compatible == true)){//Check si
		for (int k=0; k<nsi_; ++k){
			if( ((pats_[i]->si_[k]==L)&&(pats_[j]->si_[k]==H)) || ((pats_[i]->si_[k]==H)&&(pats_[j]->si_[k]==L)) ){
				compatible = false;
				break;
			}
		}
	}

    return compatible; 
}

inline bool PatternProcessor::Merge(size_t i, size_t j) { 
	for (int k=0; k<npi_; ++k){
		pats_[i]->pi1_[k] = (pats_[i]->pi1_[k]<pats_[j]->pi1_[k]) ? pats_[i]->pi1_[k]: pats_[j]->pi1_[k];
	}
	if (pats_[i]->pi2_!= NULL){
		for (int k=0; k<npi_; ++k){
			pats_[i]->pi2_[k] = (pats_[i]->pi2_[k]<pats_[j]->pi2_[k]) ? pats_[i]->pi2_[k]: pats_[j]->pi2_[k];
		}
	}
	if (pats_[i]->ppi_!= NULL){
		for (int k=0; k<nppi_; ++k){
			pats_[i]->ppi_[k] = (pats_[i]->ppi_[k]<pats_[j]->ppi_[k]) ? pats_[i]->ppi_[k]: pats_[j]->ppi_[k];
		}
	}
	if (pats_[i]->si_!= NULL){
		for (int k=0; k<nsi_; ++k){
			pats_[i]->si_[k] = (pats_[i]->si_[k]<pats_[j]->si_[k]) ? pats_[i]->si_[k]: pats_[j]->si_[k];
		}
	}

	for (int k=0; k<npo_; ++k){
		pats_[i]->po1_[k] = (pats_[i]->po1_[k]<pats_[j]->po1_[k]) ? pats_[i]->po1_[k]: pats_[j]->po1_[k];
	}
	if (pats_[i]->po2_!= NULL){
		for (int k=0; k<npo_; ++k){
			pats_[i]->po2_[k] = (pats_[i]->po2_[k]<pats_[j]->po2_[k]) ? pats_[i]->po2_[k]: pats_[j]->po2_[k];
		}
	}
	if (pats_[i]->ppo_!= NULL){
		for (int k=0; k<nppi_; ++k){
			pats_[i]->ppo_[k] = (pats_[i]->ppo_[k]<pats_[j]->ppo_[k]) ? pats_[i]->ppo_[k]: pats_[j]->ppo_[k];
		}
	}

    return true; 
} 

};

#endif


