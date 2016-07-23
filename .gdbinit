break CoreNs::Atpg::Tpg() if current_fault_->gate_==7
r -f script/atpg_b01_saf.script 
delete breakpoints 1
break CoreNs::Atpg::DDrive()
break CoreNs::Atpg::Backtrace()
break CoreNs::Implicator::MakeDecision(CoreNs::Gate*, unsigned char)
break CoreNs::Atpg::DBackTrack() 
continue 
source dbinit_stl_views-1.03.txt
