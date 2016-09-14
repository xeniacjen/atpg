set history save on

break CoreNs::Atpg::Tpg() if is_obj_optim_mode_==true \
  && current_fault_->gate_==19 \
  && current_fault_->line_==1 \
  && current_fault_->type_==Fault::SA0
r -f script/atpg_s27_saf.script 
# continue 
delete breakpoints 1
break CoreNs::Atpg::MultiDDrive() 
break CoreNs::Atpg::GenObjs() 
break CoreNs::Atpg::AddGateToProp(CoreNs::Gate*)
break CoreNs::Atpg::insertObj
break CoreNs::Implicator::MakeDecision(CoreNs::Gate*, unsigned char)
break CoreNs::Atpg::DBackTrack() 
break CoreNs::Implicator::BackTrack()
break CoreNs::Atpg::MultiDBackTrack
break CoreNs::Implicator::SetVal(int, unsigned char)
break implicator.cpp:356
source .gdb/dbinit_stl_views-1.03.txt

commands 4 
    print *gtoprop 
end 
 
# dump inserted obj. 
commands 5 
    print obj 
end 

# dump decision made 
commands 6 
    print *g 
    print v
    print e_front_list_.size() 
end 

commands 7 
    record 
    continue 
end 

commands 11 
    if e_front_list_[i]!=186 
        record stop 
        continue 
    end 
end 

continue 
