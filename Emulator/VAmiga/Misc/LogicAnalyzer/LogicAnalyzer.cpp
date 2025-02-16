// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "LogicAnalyzer.h"
#include "Amiga.h"

namespace vamiga {

void
LogicAnalyzer::_pause()
{
    recordDelayed(agnus.pos.h);
}

void
LogicAnalyzer::_didReset(bool hard)
{
    checkEnable();
}

void
LogicAnalyzer::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {
        
        dumpConfig(os);
    }
}

void
LogicAnalyzer::cacheInfo(LogicAnalyzerInfo &info) const
{
    {   SYNCHRONIZED
        
        info.busOwner = agnus.busOwner;
        info.addrBus = agnus.busAddr;
        info.dataBus = agnus.busData;
        
        for (isize i = 0; i < 4; i++) {
            info.channel[i] = record[i];
        }
    }
}

i64
LogicAnalyzer::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::LA_PROBE0: return (i64)config.channel[0];
        case Opt::LA_PROBE1: return (i64)config.channel[1];
        case Opt::LA_PROBE2: return (i64)config.channel[2];
        case Opt::LA_PROBE3: return (i64)config.channel[3];
        case Opt::LA_ADDR0: return (i64)config.addr[0];
        case Opt::LA_ADDR1: return (i64)config.addr[1];
        case Opt::LA_ADDR2: return (i64)config.addr[2];
        case Opt::LA_ADDR3: return (i64)config.addr[3];

        default:
            fatalError;
    }
}

void
LogicAnalyzer::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::LA_PROBE0:
        case Opt::LA_PROBE1:
        case Opt::LA_PROBE2:
        case Opt::LA_PROBE3:

            if (!ProbeEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, ProbeEnum::keyList());
            }
            
        case Opt::LA_ADDR0:
        case Opt::LA_ADDR1:
        case Opt::LA_ADDR2:
        case Opt::LA_ADDR3:

            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
LogicAnalyzer::setOption(Opt option, i64 value)
{
    isize c = 0;
    bool invalidate = false;
    
    switch (option) {
            
        case Opt::LA_PROBE3: c++; [[fallthrough]];
        case Opt::LA_PROBE2: c++; [[fallthrough]];
        case Opt::LA_PROBE1: c++; [[fallthrough]];
        case Opt::LA_PROBE0:

            invalidate = config.channel[c] != (Probe)value;
            config.channel[c] = (Probe)value;
            break;
            
        case Opt::LA_ADDR3: c++; [[fallthrough]];
        case Opt::LA_ADDR2: c++; [[fallthrough]];
        case Opt::LA_ADDR1: c++; [[fallthrough]];
        case Opt::LA_ADDR0:
            
            invalidate = config.addr[c] != (u32)value && config.channel[c] == Probe::MEMORY;
            config.addr[c] = (u32)value;
            break;

        default:
            fatalError;
    }

    // Wipe out prerecorded data if necessary
    if (invalidate) std::fill_n(record[c], HPOS_CNT, -1);
 
    // Enable or disable the logic analyzer
    checkEnable();
}

void
LogicAnalyzer::checkEnable()
{
    bool enable =
    config.channel[0] != Probe::NONE ||
    config.channel[1] != Probe::NONE ||
    config.channel[2] != Probe::NONE ||
    config.channel[3] != Probe::NONE ;
    
    enable ? agnus.syncEvent |= EVFL::PROBE : agnus.syncEvent &= ~EVFL::PROBE;
}

void
LogicAnalyzer::recordSignals()
{
    // Only proceed if this is the main instance
    if (isRunAheadInstance()) { return; }

    /*
     This function records all requested signal values when the logic analyzer
     is active. The function is called inside the REG slot handler after all
     pending register changes have been performed. As a result, the logic
     analyzer sees the Amiga's internal state just as the CPU would see it when
     reading from memory. This is fine for memory probing, as the obtained
     values are the ones we want to see in the logic analyzer.
          
     However, when examining other signals, we need to pay special attention.
     E.g., when probing the CPU's IPL lines, the function getIPL() provides us
     with the IPL line the CPU has seen in the previous cycle. This is because
     the IPL line is updated by a Paula event, which triggers after this
     function has been called but before the CPU queries the signal. The code
     below copes with this situation by splitting the recording code into two
     separate functions. The first one probes all signals whose values belong
     to the current DMA cycle. The second function probes all signals that need
     to be recorded with the timestamp of the previous DMA cycle.

     The second function is also called when the emulator pauses to complement
     the missing signal values.
     */
    
    recordCurrent(agnus.pos.h);
    recordDelayed(agnus.pos.hPrev());
}

void
LogicAnalyzer::recordCurrent(isize hpos)
{
    for (isize i = 0; i < 4; i++) {
        
        switch (config.channel[i]) {

            case Probe::MEMORY:
                
                record[i][hpos] = isize(mem.spypeek16<Accessor::CPU>(config.addr[i]));
                break;
                
            default:
                break;
        }
    }
}

void
LogicAnalyzer::recordDelayed(isize hpos)
{
    for (isize i = 0; i < 4; i++) {
        
        switch (config.channel[i]) {
                
            case Probe::IPL:

                record[i][hpos] = cpu.getIPL();
                break;

            default:
                break;
        }
    }
}

}
