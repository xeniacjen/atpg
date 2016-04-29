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

#include <cassert>

#include "atpg_mgr.h" 

using namespace std; 

using namespace CoreNs; 

void AtpgMgr::generation() { 
    pcoll_->init(cir_); 
    Fault *f = NULL; 
    FaultList flist = fListExtract_->current_; 

    while (flist.begin()!=flist.end()) { 
        if (flist.front()->state_==Fault::DT) { 
            flist.pop_front(); 
            continue; 
        }
        // if (flist.front()->state_==Fault::AB) 
        if (flist.front()->state_==Fault::AB || flist.front()->state_==Fault::PT) 
            break; 

        if (f==flist.front()) { 
            // assert(0); // shouldn't reach here
            #if MODE == dbg
            cout << "#  " << f->gate_ << " s-a-";  
            cout << f->type_ << endl;
            cout << "#    pi order: ";
            for (int i = 0; i < pcoll_->npi_; ++i)
                cout << " " << pcoll_->piOrder_[i];
            cout << endl;
            cout << "#    ppi order:";
            for (int i = 0; i < pcoll_->nppi_; ++i)
                cout << " " << pcoll_->ppiOrder_[i];
            cout << endl;
            cout << "#    po order: ";
            for (int i = 0; i < pcoll_->npo_; ++i)
                cout << " " << pcoll_->poOrder_[i];
            cout << endl;

            cout << "#" << endl;

            cout << "#      pi1: ";
            if (pcoll_->pats_.back()->pi1_)
                for (int j = 0; j < pcoll_->npi_; ++j)
                    printValue(pcoll_->pats_.back()->pi1_[j]);
            cout << endl;
            cout << "#      pi2: ";
            if (pcoll_->pats_.back()->pi2_)
                for (int j = 0; j < pcoll_->npi_; ++j)
                    printValue(pcoll_->pats_.back()->pi2_[j]);
            cout << endl;
            cout << "#      ppi: ";
            if (pcoll_->pats_.back()->ppi_)
                for (int j = 0; j < pcoll_->nppi_; ++j)
                    printValue(pcoll_->pats_.back()->ppi_[j]);
            cout << endl;
            cout << "#      po1: ";
            if (pcoll_->pats_.back()->po1_)
                for (int j = 0; j < pcoll_->npo_; ++j)
                    printValue(pcoll_->pats_.back()->po1_[j]);
            cout << endl;
            cout << "#      po2: ";
            if (pcoll_->pats_.back()->po2_)
                for (int j = 0; j < pcoll_->npo_; ++j)
                    printValue(pcoll_->pats_.back()->po2_[j]);
            cout << endl;
            cout << "#      ppo: ";
            if (pcoll_->pats_.back()->ppo_)
                for (int j = 0; j < pcoll_->nppi_; ++j)
                    printValue(pcoll_->pats_.back()->ppo_[j]);
            cout << endl << "#" << endl;
            #endif 
            f->state_ = Fault::PT; 
            flist.push_back(flist.front()); 
            flist.pop_front(); 
        }

        f = flist.front();  
        atpg_ = new Atpg(cir_, f); 
        Atpg::GenStatus ret = atpg_->Tpg(); 

        if (ret==Atpg::TEST_FOUND) { 
            Pattern *p = new Pattern; 
		    p->pi1_ = new Value[cir_->npi_];
		    p->ppi_ = new Value[cir_->nppi_];
		    p->po1_ = new Value[cir_->npo_];
		    p->ppo_ = new Value[cir_->nppi_];
		    pcoll_->pats_.push_back(p);
            atpg_->GetPiPattern(p); 

            sim_->pfFaultSim(pcoll_->pats_.back(), flist); 
            getPoPattern(pcoll_->pats_.back()); 
        }
        else if (ret==Atpg::UNTESTABLE) { 
            flist.front()->state_ = Fault::AU; 
            flist.pop_front(); 
        }
        else { // ABORT 
            //TODO 
         // cout << "*** BACKTRACK NEEDED!! \n";  
            flist.front()->state_ = Fault::AB; 
            flist.push_back(flist.front()); 
            flist.pop_front(); 
        }

        delete atpg_; 
    }
}

void AtpgMgr::getPoPattern(Pattern *pat) { 
    sim_->goodSim();
    int offset = cir_->ngate_ - cir_->npo_ - cir_->nppi_;
    for (int i = 0; i < cir_->npo_ ; i++) {
        if (cir_->gates_[offset + i].gl_ == PARA_H)
            pat->po1_[i] = L;
        else if (cir_->gates_[offset + i].gh_ == PARA_H)
            pat->po1_[i] = H;
        else
            pat->po1_[i] = X;
    }
	if(pat->po2_!=NULL && cir_->nframe_>1)
		for( int i = 0 ; i < cir_->npo_ ; i++ ){
			if (cir_->gates_[offset + i + cir_->ngate_].gl_ == PARA_H)
				pat->po2_[i] = L;
			else if (cir_->gates_[offset + i + cir_->ngate_].gh_ == PARA_H)
				pat->po2_[i] = H;
			else
				pat->po2_[i] = X;
		}

    offset = cir_->ngate_ - cir_->nppi_;
	if(cir_->nframe_>1)
		offset += cir_->ngate_;
    for (int i = 0; i < cir_->nppi_; i++) {
        if (cir_->gates_[offset + i].gl_ == PARA_H)
            pat->ppo_[i] = L;
        else if (cir_->gates_[offset + i].gh_ == PARA_H)
            pat->ppo_[i] = H;
        else
            pat->ppo_[i] = X;
    }
}
