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

void
LogicAnalyzer::_pause()
{
    recordSignals();
}

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
        case OPT_LA_PROBE4: return (i64)config.channel[4];
        case OPT_LA_PROBE5: return (i64)config.channel[5];
        case OPT_LA_ADDR0: return (i64)config.addr[0];
        case OPT_LA_ADDR1: return (i64)config.addr[1];
        case OPT_LA_ADDR2: return (i64)config.addr[2];
        case OPT_LA_ADDR3: return (i64)config.addr[3];
        case OPT_LA_ADDR4: return (i64)config.addr[4];
        case OPT_LA_ADDR5: return (i64)config.addr[5];

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
        case OPT_LA_PROBE4:
        case OPT_LA_PROBE5:

            if (!ProbeEnum::isValid(value)) {
                throw Error(VAERROR_OPT_INV_ARG, ProbeEnum::keyList());
            }
            
        case OPT_LA_ADDR0:
        case OPT_LA_ADDR1:
        case OPT_LA_ADDR2:
        case OPT_LA_ADDR3:
        case OPT_LA_ADDR4:
        case OPT_LA_ADDR5:

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
            
        case OPT_LA_PROBE5: c++; [[fallthrough]];
        case OPT_LA_PROBE4: c++; [[fallthrough]];
        case OPT_LA_PROBE3: c++; [[fallthrough]];
        case OPT_LA_PROBE2: c++; [[fallthrough]];
        case OPT_LA_PROBE1: c++; [[fallthrough]];
        case OPT_LA_PROBE0:

            invalidate = config.channel[c] != (Probe)value;
            config.channel[c] = (Probe)value;
            break;
            
        case OPT_LA_ADDR5: c++; [[fallthrough]];
        case OPT_LA_ADDR4: c++; [[fallthrough]];
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

bool
LogicAnalyzer::recording() const
{
    return
    config.channel[0] != PROBE_NONE ||
    config.channel[1] != PROBE_NONE ||
    config.channel[2] != PROBE_NONE ||
    config.channel[3] != PROBE_NONE ||
    config.channel[4] != PROBE_NONE ||
    config.channel[5] != PROBE_NONE ;
}

void
LogicAnalyzer::recordSignals()
{
    recordSignals(agnus.pos.h);
}

void
LogicAnalyzer::recordSignals(isize hpos)
{
    /* This function is called at the beginning of each DMA cycle, just after
     * register change recorder has updated all registers. Hence, when reading
     * from memory at this point, we get the same value the CPU would get when
     * reading the same memory location. This is the value we want to see
     * when probing memory contents.
     * However, when probing the address or data bus, we see the value from
     * the previous cycle as nothing has been put on the buses, yet.
     */
    
    assert(hpos == agnus.pos.h);
    
    auto hprev = agnus.pos.hPrev();
    
    trace(true, "LogicAnalyzer::recordSignals\n");
    
    assert(hpos >= 0 && hpos < HPOS_CNT);
    
    // Only proceed if this is the main instance
    if (isRunAheadInstance()) { return; }
        
    // Record signals
    for (isize i = 0; i < 6; i++) {
        
        switch (config.channel[i]) {

            case PROBE_BUS_OWNER:
                
                if (agnus.busOwner[hprev] != BUS_NONE) {
                    record[i][hprev] = isize(agnus.busOwner[hprev]);
                } else {
                    record[i][hprev] = -1;
                }
                break;
                
            case PROBE_ADDR_BUS:

                if (agnus.busOwner[hprev] != BUS_NONE) {
                    record[i][hprev] = isize(agnus.busAddr[hprev]);
                } else {
                    record[i][hprev] = -1;
                }
                break;

            case PROBE_DATA_BUS:

                if (agnus.busOwner[hprev] != BUS_NONE) {
                    record[i][hprev] = isize(agnus.busData[hprev]);
                } else {
                    record[i][hprev] = -1;
                }
                break;

            case PROBE_MEMORY:
                
                record[i][hpos] = isize(mem.spypeek16<ACCESSOR_CPU>(config.addr[i]));
                break;
                
            default:
                break;
        }
    }
}

void
LogicAnalyzer::servicePROEvent()
{
    /*
    // Disable the logic analyzer if this is the run-ahead instance
    if (isRunAheadInstance()) { agnus.cancel<SLOT_PRO>(); return; }
        
    auto hpos = agnus.pos.h;
    
    // Record signals
    for (isize i = 0; i < 6; i++) {
        
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
    */
}

void
LogicAnalyzer::scheduleFirstProEvent()
{
    if (recording()) {
        agnus.syncEvent |= EVFL::PROBE;
    } else {
        agnus.syncEvent &= ~EVFL::PROBE;
    }
    

    /*
    for (isize i = 0; i < 6; i++) {
        
        if (config.channel[i] != PROBE_NONE) {
            agnus.scheduleRel<SLOT_PRO>(0, PRO_RECORD);
            return;
        }
    }
    
    agnus.cancel<SLOT_PRO>();
    */
}

}
