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
#include <cstdlib>
#include <iomanip>
#include <algorithm>

#include "atpg_mgr.h" 

using namespace std; 

using namespace CoreNs; 

bool comp_fault_hard(Fault* f1, Fault* f2);  
bool comp_fault_lvl(Fault* f1, Fault* f2);  
void AtpgMgr::generation(int limit) { 
    if (limit<0) limit = _MAX_BACK_TRACK_LIMIT_; 

    // TODO: using script 
    // learn_mgr_ = new LearnInfoMgr(cir_); 
    // learn_mgr_->StaticLearn(); 

    pcoll_->init(cir_); 
    Fault *f = NULL; 
    for (int i=0; i<fListExtract_->faults_.size(); i++) 
        calc_fault_hardness(fListExtract_->faults_[i]); 
    FaultList flist = fListExtract_->current_; 

    if (!set_oo_on_) { 
        flist.sort(comp_fault_hard); 
    
        cout << "\n# ------------------------------------------------------------------------\n"; 
        cout << "# Phase 1: drop faults need no back-track \n"; 
        cout << "# ------------------------------------------------------------------------\n"; 
        cout << "# #patterns  fault     #faults  #faults \n"; 
        cout << "# simulated  coverage  in list  detected\n"; 
        cout << "# ------------------------------------------------------------------------\n"; 
     
        while (flist.begin()!=flist.end()) { 
            if (flist.front()->state_==Fault::DT) { 
                flist.pop_front(); 
                continue; 
            }
            if (flist.front()->state_==Fault::AH)  
                break; 
    
            if (f==flist.front()) { 
                assert(0); 
            }
    
            f = flist.front();  
            atpg_ = new Atpg(cir_, f); 
            atpg_->SetBackTrackLimit(0); 
            Atpg::GenStatus ret = atpg_->Tpg(); 
    
            if (ret==Atpg::TEST_FOUND) { 
                Pattern *p = new Pattern; 
		        p->pi1_ = new Value[cir_->npi_];
		        p->ppi_ = new Value[cir_->nppi_];
		        p->po1_ = new Value[cir_->npo_];
		        p->ppo_ = new Value[cir_->nppi_];
		        pcoll_->pats_.push_back(p);
                atpg_->GetPiPattern(p); 
    
                if (pcoll_->dynamicCompression_==PatternProcessor::ON) 
                DynamicCompression(flist); 
    
		        if ((pcoll_->staticCompression_ == PatternProcessor::OFF) 
                && (pcoll_->XFill_ == PatternProcessor::ON)){
			        pcoll_->randomFill(pcoll_->pats_.back());
		        }
    
                sim_->pfFaultSim(pcoll_->pats_.back(), flist); 
                getPoPattern(pcoll_->pats_.back()); 
            }
            else if (ret==Atpg::UNTESTABLE) { 
                flist.front()->state_ = Fault::AU; 
                flist.pop_front(); 
            }
            else { // ABORT 
                flist.front()->state_ = Fault::AH; 
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

    pcoll_->nbit_spec_ = 0; 
    pcoll_->nbit_spec_max = 0; 
    flist.sort(comp_fault_lvl); 

    cout << "\n# ------------------------------------------------------------------------\n"; 
    cout << "# Main Phase: hard-to-detect fault \n"; 
    cout << "# ------------------------------------------------------------------------\n"; 
    cout << "# #patterns  fault     #faults  #faults \n"; 
    cout << "# simulated  coverage  in list  detected\n"; 
    cout << "# ------------------------------------------------------------------------\n"; 
    while (flist.begin()!=flist.end()) { 
        if (flist.front()->state_==Fault::DT) { 
            flist.pop_front(); 
            continue; 
        }
        if (flist.front()->state_==Fault::AB)  
            break; 

        if (f==flist.front()) { 
            assert(0); 
        }

        f = flist.front();  
        atpg_ = new Atpg(cir_, f); 
        atpg_->SetBackTrackLimit(limit); 
        // atpg_->SetLearnEngine(learn_mgr_); 
        if (set_dfs_on_) atpg_->TurnOnPoMode(); 
        if (set_oo_on_) atpg_->TurnOnObjOptimMode(fListExtract_); 
        Atpg::GenStatus ret = atpg_->Tpg(); 

        if (ret==Atpg::TEST_FOUND) { 
            Pattern *p = new Pattern; 
		    p->pi1_ = new Value[cir_->npi_];
		    p->ppi_ = new Value[cir_->nppi_];
		    p->po1_ = new Value[cir_->npo_];
		    p->ppo_ = new Value[cir_->nppi_];
		    pcoll_->pats_.push_back(p);
            pcoll_->npat_hard_++; 
            atpg_->GetPiPattern(p); 

            flist.front()->state_ = Fault::DH; 
            // if (set_oo_on_) { 
            //     prob_fs+=atpg_->prob_fs_; 
            //     prop_fs+=atpg_->prop_fs_; 
            // }

            if (pcoll_->dynamicCompression_==PatternProcessor::ON) 
                DynamicCompression(flist); 

		    if ((pcoll_->staticCompression_ == PatternProcessor::OFF) 
              && (pcoll_->XFill_ == PatternProcessor::ON)){
			    pcoll_->randomFill(pcoll_->pats_.back());
		    }

            // flist.pop_front(); 
            sim_->pfFaultSim(pcoll_->pats_.back(), flist); 
            getPoPattern(pcoll_->pats_.back()); 
        }
        else if (ret==Atpg::UNTESTABLE) { 
            flist.front()->state_ = Fault::AU; 
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
            int dt = fListExtract_->getNStatus(Fault::DT) 
                + fListExtract_->getNStatus(Fault::DH); 
            cout << "# " << setw(9) << pcoll_->pats_.size(); 
            cout << "  " << setw(8) << (float)dt / (float)fu * 100.f << "%";  
            cout << "  " << setw(7) << fu - dt; 
            cout << "  " << setw(8) << dt; 
            // cout << "  " << setw(8) << (float)prop_fs/(float)prob_fs*100.f<<"% "; 
            // cout << "  " << setw(8) << prop_fs; 
            cout << endl; 

            // prop_fs = 0; 
            // prob_fs = 0; 
        }   
    }

    if (set_dfs_on_ && set_oo_on_) { 
        flist.sort(comp_fault_hard); 
    
        cout << "\n# ------------------------------------------------------------------------\n"; 
        cout << "# Phase 2: run top-off atpg \n"; 
        cout << "# ------------------------------------------------------------------------\n"; 
        cout << "# #patterns  fault     #faults  #faults \n"; 
        cout << "# simulated  coverage  in list  detected\n"; 
        cout << "# ------------------------------------------------------------------------\n"; 
     
        while (flist.begin()!=flist.end()) { 
            if (flist.front()->state_==Fault::DT) { 
                flist.pop_front(); 
                continue; 
            }
            if (flist.front()->state_==Fault::AH)  
                break; 
    
            if (f==flist.front()) { 
                assert(0); 
            }
    
            f = flist.front();  
            atpg_ = new Atpg(cir_, f); 
            atpg_->SetBackTrackLimit(limit); 
            atpg_->TurnOnPoMode(); 
            Atpg::GenStatus ret = atpg_->Tpg(); 
    
            if (ret==Atpg::TEST_FOUND) { 
                Pattern *p = new Pattern; 
		        p->pi1_ = new Value[cir_->npi_];
		        p->ppi_ = new Value[cir_->nppi_];
		        p->po1_ = new Value[cir_->npo_];
		        p->ppo_ = new Value[cir_->nppi_];
		        pcoll_->pats_.push_back(p);
                atpg_->GetPiPattern(p); 
    
                flist.front()->state_ = Fault::DH; 

                // if (pcoll_->dynamicCompression_==PatternProcessor::ON) 
                // DynamicCompression(flist); 
    
		        if ((pcoll_->staticCompression_ == PatternProcessor::OFF) 
                && (pcoll_->XFill_ == PatternProcessor::ON)){
			        pcoll_->randomFill(pcoll_->pats_.back());
		        }
    
                sim_->pfFaultSim(pcoll_->pats_.back(), flist); 
                getPoPattern(pcoll_->pats_.back()); 
            }
            else if (ret==Atpg::UNTESTABLE) { 
                flist.front()->state_ = Fault::AU; 
                flist.pop_front(); 
            }
            else { // ABORT 
                flist.front()->state_ = Fault::AH; 
                flist.push_back(flist.front()); 
                flist.pop_front(); 
            }
    
            delete atpg_; 

            if (ret==Atpg::TEST_FOUND && pcoll_->pats_.size()%RPT_PER_PAT==0) {
                int fu = fListExtract_->current_.size(); 
                int dt = fListExtract_->getNStatus(Fault::DT) 
                       + fListExtract_->getNStatus(Fault::DH); 
                cout << "# " << setw(9) << pcoll_->pats_.size(); 
                cout << "  " << setw(8) << (float)dt / (float)fu * 100.f << "%";  
                cout << "  " << setw(7) << fu - dt; 
                cout << "  " << setw(8) << dt; 
                cout << endl; 
            }   
        }
    }

    if (pcoll_->staticCompression_==PatternProcessor::ON) { 
        // ReverseFaultSim(); 
        RVEFaultSim(); 
        // pcoll_->StaticCompressionGraph(); 
        pcoll_->StaticCompression(); 

        if (pcoll_->XFill_==PatternProcessor::ON) 
            XFill(); 
	}

    RVEFaultSim(); 
    ReverseFaultSim(); 
}

bool comp_fault_hard(Fault* f1, Fault* f2) {
    return f1->hard_ > f2->hard_; 
} 

bool comp_fault_lvl(Fault* f1, Fault* f2) {
    return f1->gate_ < f2->gate_; 
}

void AtpgMgr::calc_fault_hardness(Fault* f1) {
    int t1; 
    
    t1 = (f1->type_==Fault::SA0 || f1->type_==Fault::STR)?cir_->gates_[f1->gate_].cc1_:cir_->gates_[f1->gate_].cc0_; 
    t1 *= (f1->line_)?cir_->gates_[f1->gate_].co_i_[f1->line_-1]:cir_->gates_[f1->gate_].co_o_;

    f1->hard_ = t1; 

}

void AtpgMgr::ReverseFaultSim() { 
    int total_dt = fListExtract_->getNStatus(Fault::DT) 
      + fListExtract_->getNStatus(Fault::DH); 
    FaultList flist = fListExtract_->current_; 
    // req_dt_ = (req_dt_>0)?req_dt_:total_dt;

    int curr_dt = 0; 
    PatternVec comp_pats; 
    for (int i = 0; i < pcoll_->pats_.size(); ++i) {
        Pattern *p = pcoll_->pats_[pcoll_->pats_.size()-i-1]; 
        int dt = sim_->pfFaultSim(p, flist); 

        if(dt > 0) { 
            comp_pats.insert(comp_pats.begin(), p); 
            if (req_dt_>0 && curr_dt<req_dt_) 
                req_pat_ = comp_pats.size(); 
        }

        curr_dt+=dt; 
    }

    assert(curr_dt>=total_dt);  
    pcoll_->pats_ = comp_pats; 
}

void AtpgMgr::RVEFaultSim() { 
    typedef set<size_t> PatSet; 
    typedef PatSet::iterator PatSetIter; 
    typedef map<Fault *, PatSet> FaultPatSetMap; 
    typedef FaultPatSetMap::iterator FaultPatSetMapIter; 

    int *num_essential_faults = new int[pcoll_->pats_.size()]; 
    FaultPatSetMap fault_detect; 
    for (size_t i=0; i < pcoll_->pats_.size(); i++) {
        // size_t pid = pcoll_->pats_.size()-i-1; 
        size_t pid = i; 
        Pattern *p = pcoll_->pats_[pid]; 
        num_essential_faults[pid] = 0; 

        FaultList flist = fListExtract_->current_; 
        FaultVec detect; 
        sim_->pfFaultSim(p, detect, flist); 
        for (size_t j=0; j<detect.size(); j++) { 
            PatSet ps; ps.insert(pid); 
            pair<FaultPatSetMapIter, bool> ret 
              = fault_detect.insert(make_pair(detect[j], ps)); 
            if (ret.second) { 
                num_essential_faults[pid]++; 
            }
            else if (ret.first->second.size()==1) { 
                ret.first->second.insert(pid); 

                size_t pid_check = *ret.first->second.begin(); 
                if (--num_essential_faults[pid_check]==0) { 
                    FaultPatSetMapIter it = fault_detect.begin(); 
                    for (; it!=fault_detect.end(); ++it) { 
                        PatSetIter it_pat = it->second.find(pid_check); 
                        if (it_pat!=it->second.end()) 
                            it->second.erase(it_pat); 
                        if (it->second.size()==1) { 
                            num_essential_faults[*it->second.begin()]++; 
                        }
                    }
                }
            }
        }
    }
    
    PatternVec comp_pats; 
    for (size_t i=0; i<pcoll_->pats_.size(); i++) { 
        if (num_essential_faults[i]>0) 
            comp_pats.push_back(pcoll_->pats_[i]); 
    }

    pcoll_->pats_ = comp_pats; 

    delete[] num_essential_faults; 
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

void AtpgMgr::XFill() { 
    pcoll_->npat_hard_ = 0; 
    for (int i=0; i<pcoll_->pats_.size(); i++) { 
        Pattern *p = pcoll_->pats_[i]; 
        pcoll_->randomFill(p); 
		sim_->assignPatternToPi(p);
		sim_->goodSim();
        getPoPattern(p); 
        pcoll_->npat_hard_++; 
    }
}

void AtpgMgr::DynamicCompression(FaultList &remain) { 
    Pattern *p = pcoll_->pats_.back(); 
    Atpg::GenStatus stat = Atpg::TEST_FOUND; 
    Atpg *atpg = atpg_; 
    bool begin = true; 
    FaultList skipped_fs; 
    int fail_count = 0;  

    while (true) { 
        if (stat==Atpg::TEST_FOUND) { 
            if (begin) begin = false; 
            else delete atpg_; 
            atpg_ = atpg; 
            atpg_->GetPiPattern(p); 
            sim_->pfFaultSim(p, remain);  
            getPoPattern(p); 
        }
        else { 
            delete atpg; 
            skipped_fs.push_back(remain.front()); 
            remain.pop_front(); 
            if (++fail_count>=dyn_comp_merge_) 
                break; 
        }

        while (!remain.empty() 
          && !atpg_->CheckCompatibility(remain.front())) { 
            skipped_fs.push_back(remain.front()); 
            remain.pop_front(); 
        }
        if (remain.empty()) break; 

        atpg = new Atpg(cir_, remain.front(), p); 
        atpg->SetBackTrackLimit(dyn_comp_backtrack); 
        if (set_dfs_on_) atpg->TurnOnPoMode(); 
        stat = atpg->Tpg(); 
    }

    while (!skipped_fs.empty()) { 
        remain.push_front(skipped_fs.back()); 
        skipped_fs.pop_back(); 
    }
}
