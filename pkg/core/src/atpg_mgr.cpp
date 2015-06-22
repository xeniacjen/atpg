/*
 * =====================================================================================
 *
 *       Filename:  atpg_mgr.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/22/2015 08:33:49 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  xenia-cjen (xc), jonah0604@gmail.com
 *        Company:  LaDS(I), GIEE, NTU
 *
 * =====================================================================================
 */

#include "atpg_mgr.h" 

using namespace std; 

using namespace CoreNs; 

void AtpgMgr::generation() { 
    pcoll_->init(cir_); 

    FaultListIter fit = fListExtract_->current_.begin(); 
    for (; fit!=fListExtract_->current_.end(); ++fit) { 
        Fault *f = *fit; 
        if (f->state_==Fault::DT) 
            continue; 
        atpg_ = new Atpg(cir_, f); 
        Atpg::GenStatus ret = atpg_->Tpg(); 

        if (ret==Atpg::TEST_FOUND) { 
            Pattern *p = new Pattern; 
		    p->pi1_ = new Value[cir_->npi_];
		    p->ppi_ = new Value[cir_->nppi_];
		    p->po1_ = new Value[cir_->npo_];
		    p->ppo_ = new Value[cir_->nppi_];
		    pcoll_->pats_.push_back(p);
            atpg_->GetPattern(p); 

            f->state_ = Fault::DT; 
        }
        else { 
            //TODO 
            cout << "*** BACKTRACK NEEDED!! \n";  
        }

        delete atpg_; 
    }
}
