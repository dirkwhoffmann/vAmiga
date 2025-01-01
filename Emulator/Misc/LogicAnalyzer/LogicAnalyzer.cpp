// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "LogicAnalyzer.h"
#include "Amiga.h"

namespace vamiga {

/*
LogicAnalyzer::LogicAnalyzer(Amiga &ref) : SubComponent(ref)
{
    
}
*/

void
LogicAnalyzer::_didReset(bool hard)
{
    servicePROEvent();
}

void
LogicAnalyzer::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {
        
        dumpConfig(os);
    }
}

i64
LogicAnalyzer::getOption(Option option) const
{
    switch (option) {
            
        case OPT_LA_PROBE0: return (i64)config.channel[0];
        case OPT_LA_PROBE1: return (i64)config.channel[1];
        case OPT_LA_PROBE2: return (i64)config.channel[2];
        case OPT_LA_PROBE3: return (i64)config.channel[3];
        case OPT_LA_ADDR0: return (i64)config.addr[0];
        case OPT_LA_ADDR1: return (i64)config.addr[1];
        case OPT_LA_ADDR2: return (i64)config.addr[2];
        case OPT_LA_ADDR3: return (i64)config.addr[3];

        default:
            fatalError;
    }
}

void
LogicAnalyzer::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_LA_PROBE0:
        case OPT_LA_PROBE1:
        case OPT_LA_PROBE2:
        case OPT_LA_PROBE3:
            
            if (!ProbeEnum::isValid(value)) {
                throw Error(VAERROR_OPT_INV_ARG, ProbeEnum::keyList());
            }
            
        case OPT_LA_ADDR0:
        case OPT_LA_ADDR1:
        case OPT_LA_ADDR2:
        case OPT_LA_ADDR3:
            
            return;

        default:
            throw(VAERROR_OPT_UNSUPPORTED);
    }
}

void
LogicAnalyzer::setOption(Option option, i64 value)
{
    switch (option) {
            
        case OPT_LA_PROBE0: config.channel[0] = (Probe)value; break;
        case OPT_LA_PROBE1: config.channel[1] = (Probe)value; break;
        case OPT_LA_PROBE2: config.channel[2] = (Probe)value; break;
        case OPT_LA_PROBE3: config.channel[3] = (Probe)value; break;
        case OPT_LA_ADDR0:  config.addr[0] = (u32)value; break;
        case OPT_LA_ADDR1:  config.addr[1] = (u32)value; break;
        case OPT_LA_ADDR2:  config.addr[2] = (u32)value; break;
        case OPT_LA_ADDR3:  config.addr[3] = (u32)value; break;

        default:
            fatalError;
    }

    scheduleNextProEvent();
}

void
LogicAnalyzer::servicePROEvent()
{
    
    // Schedule next event
    agnus.scheduleImm<SLOT_PRO>(PRO_RECORD);    
}

void
LogicAnalyzer::scheduleNextProEvent()
{
    if (config.channel[0] != PROBE_NONE ||
        config.channel[1] != PROBE_NONE ||
        config.channel[2] != PROBE_NONE ||
        config.channel[3] != PROBE_NONE) {
        
        agnus.scheduleImm<SLOT_PRO>(PRO_RECORD);
        
    } else {
        
        agnus.cancel<SLOT_PRO>();
    }
}

}
