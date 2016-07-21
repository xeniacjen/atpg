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
#include <iomanip>

#include "atpg_mgr.h" 

using namespace std; 

using namespace CoreNs; 

bool comp_fault(Fault* f1, Fault* f2);  
void AtpgMgr::generation() { 
    pcoll_->init(cir_); 
    Fault *f = NULL; 
    for (int i=0; i<fListExtract_->faults_.size(); i++) 
        calc_fault_hardness(fListExtract_->faults_[i]); 

    FaultList flist = fListExtract_->current_; 
    flist.sort(comp_fault); 

    cout << "# ------------------------------------------------------------------------\n"; 
    cout << "# #patterns  fault     #faults  #faults \n"; 
    cout << "# simulated  coverage  in list  detected\n"; 
    cout << "# ------------------------------------------------------------------------\n"; 
    
    while (flist.begin()!=flist.end()) { 
        if (flist.front()->state_==Fault::DT) { 
            flist.pop_front(); 
            continue; 
        }
        // if (flist.front()->state_==Fault::AB) 
          // || flist.front()->state_==Fault::AU) 
        if (flist.front()->state_==Fault::AB 
          || flist.front()->state_==Fault::PT) 
            break; 

        if (f==flist.front()) { 
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

		if ((pcoll_->staticCompression_ == PatternProcessor::OFF) && (pcoll_->XFill_ == PatternProcessor::ON)){
			pcoll_->randomFill(pcoll_->pats_.back());
		}

            sim_->pfFaultSim(pcoll_->pats_.back(), flist); 
            // flist.front()->state_ = Fault::DT; 
            // flist.pop_front(); 
            getPoPattern(pcoll_->pats_.back()); 
        }
        else if (ret==Atpg::UNTESTABLE) { 
            flist.front()->state_ = Fault::AU; 
            // flist.push_back(flist.front()); 
            flist.pop_front(); 
        }
        else { // ABORT 
            flist.front()->state_ = Fault::AB; 
            flist.push_back(flist.front()); 
            flist.pop_front(); 
        }

        delete atpg_; 

        if (ret==Atpg::TEST_FOUND && pcoll_->pats_.size()%RPT_PER_PAT==0) {
            int fu = fListExtract_->current_.size(); 
            int dt = fListExtract_->getNStatus(Fault::DT); 
            cout << "# " << setw(9) << pcoll_->pats_.size(); 
            cout << "  " << setw(8) << (float)dt / (float)fu * 100.f << "%";  
            cout << "  " << setw(7) << fu - dt; 
            cout << "  " << setw(8) << dt; 
            cout << endl; 
        }   
    }
}

bool comp_fault(Fault* f1, Fault* f2) {
    return f1->hard_ > f2->hard_; 
} 

void AtpgMgr::calc_fault_hardness(Fault* f1) {
    int t1; 
    
    t1 = (f1->type_==Fault::SA0 || f1->type_==Fault::STR)?cir_->gates_[f1->gate_].cc1_:cir_->gates_[f1->gate_].cc0_; 
    t1 *= (f1->line_)?cir_->gates_[f1->gate_].co_i_[f1->line_-1]:cir_->gates_[f1->gate_].co_o_;

    f1->hard_ = t1; 

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
