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
    scheduleFirstProEvent();
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
    isize c = 0;
    bool invalidate = false;
    
    switch (option) {
            
        case OPT_LA_PROBE3: c++; [[fallthrough]];
        case OPT_LA_PROBE2: c++; [[fallthrough]];
        case OPT_LA_PROBE1: c++; [[fallthrough]];
        case OPT_LA_PROBE0:

            invalidate = config.channel[c] != (Probe)value;
            config.channel[c] = (Probe)value;
            break;
            
        case OPT_LA_ADDR3: c++; [[fallthrough]];
        case OPT_LA_ADDR2: c++; [[fallthrough]];
        case OPT_LA_ADDR1: c++; [[fallthrough]];
        case OPT_LA_ADDR0:
            
            invalidate = config.addr[c] != (u32)value && config.channel[c] == PROBE_MEMORY;
            config.addr[c] = (u32)value;
            break;

        default:
            fatalError;
    }

    // Wipe out prerecorded data if necessary
    if (invalidate) std::fill_n(record[c], HPOS_CNT, -1);
    
    scheduleFirstProEvent();
}

void
LogicAnalyzer::servicePROEvent()
{
    // Disable the logic analyzer if this is the run-ahead instance
    if (isRunAheadInstance()) { agnus.cancel<SLOT_PRO>(); return; }
        
    auto hpos = agnus.pos.h;
    
    // Record signals
    for (isize i = 0; i < 4; i++) {
        
        switch (config.channel[i]) {

            case PROBE_BUS_OWNER:
                
                if (agnus.busOwner[hpos] != BUS_NONE) {
                    record[i][hpos] = isize(agnus.busOwner[hpos]);
                } else {
                    record[i][hpos] = -1;
                }
                break;
                
            case PROBE_ADDR_BUS:

                if (agnus.busOwner[hpos] != BUS_NONE) {
                    record[i][hpos] = isize(agnus.busAddr[hpos]);
                } else {
                    record[i][hpos] = -1;
                }
                break;

            case PROBE_DATA_BUS:

                if (agnus.busOwner[hpos] != BUS_NONE) {
                    record[i][hpos] = isize(agnus.busData[hpos]);
                } else {
                    record[i][hpos] = -1;
                }
                break;

            case PROBE_MEMORY:
                
                record[i][hpos] = isize(mem.spypeek16<ACCESSOR_CPU>(config.addr[i]));
                break;
                
            default:
                break;
        }
    }
    
    // Schedule next event
    agnus.scheduleRel<SLOT_PRO>(0, PRO_RECORD);
}

void
LogicAnalyzer::scheduleFirstProEvent()
{
    if (config.channel[0] != PROBE_NONE ||
        config.channel[1] != PROBE_NONE ||
        config.channel[2] != PROBE_NONE ||
        config.channel[3] != PROBE_NONE) {
        
        agnus.scheduleRel<SLOT_PRO>(0, PRO_RECORD);
        
    } else {
        
        agnus.cancel<SLOT_PRO>();
    }
}

}
