// **************************************************************************
// File       [ fan_mgr.h ]
// Author     [ littleshamoo ]
// Synopsis   [ ]
// Date       [ 2011/08/30 created ]
// **************************************************************************

#ifndef _FAN_FAN_MGR_H_
#define _FAN_FAN_MGR_H_

#include "interface/netlist.h"
#include "interface/techlib.h"

#include "core/atpg_mgr.h"

namespace FanNs {

class FanMgr {
public:
    FanMgr() {
        lib            = NULL;
        nl             = NULL;
        atpg_mgr       = NULL; 
        atpgStat.rTime = 0;
    }
    ~FanMgr() {}

    IntfNs::Techlib     *lib;
    IntfNs::Netlist     *nl;
    CoreNs::AtpgMgr     *atpg_mgr;
    CommonNs::TmUsage   tmusg;
    CommonNs::TmStat    atpgStat;
};

};

#endif


