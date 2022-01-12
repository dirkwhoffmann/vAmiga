// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Sequencer.h"
#include "IOUtils.h"

void
Sequencer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    /*
    if (category & dump::State) {
     
        os << tab("Flipflop 1");
        os << bol(ddf.ff1) << " (" << bol(ddfInitial.ff1) << ")" << std::endl;
        os << tab("Flipflop 2");
        os << bol(ddf.ff2) << " (" << bol(ddfInitial.ff2) << ")" << std::endl;
        os << tab("Flipflop 3");
        os << bol(ddf.ff3) << " (" << bol(ddfInitial.ff3) << ")" << std::endl;
        os << tab("Flipflop 5");
        os << bol(ddf.ff5) << " (" << bol(ddfInitial.ff5) << ")" << std::endl;
        os << std::endl;
    }
        
    if (category & dump::Dma) {

        for (isize row = 0; row < HPOS_CNT; row++) {
            
            isize i = (row / 2) + ((row % 2) ? ((HPOS_CNT + 1) / 2) : 0);
            
            string cycle = std::to_string(i) + ":";
            os << std::left << std::setw(5) << cycle;

            string bpl = Scheduler::eventName(SLOT_BPL, bplEvent[i]);
            os << std::left << std::setw(12) << bpl;
            os << " + ";
            string das = Scheduler::eventName(SLOT_DAS, dasEvent[i]);
            os << std::left << std::setw(12) << das;
            
            string next = "-> ";
            next += std::to_string(nextBplEvent[i]) + ",";
            next += std::to_string(nextDasEvent[i]);
            os << std::left << std::setw(14) << next;

            if (row % 2) { os << std::endl; } else { os << "  "; }
        }
    }
    
    if (category & dump::Signals) {
              
        auto name = [](u16 signal) -> string {
        
            string result;
            
            if (signal & 0x10) {
               
                switch (signal & 0x1f) {

                    case SIG_CON_L1:        result += "CON_L1 "; break;
                    case SIG_CON_L2:        result += "CON_L2 "; break;
                    case SIG_CON_L3:        result += "CON_L3 "; break;
                    case SIG_CON_L4:        result += "CON_L4 "; break;
                    case SIG_CON_L5:        result += "CON_L5 "; break;
                    case SIG_CON_L6:        result += "CON_L6 "; break;
                    case SIG_CON_L7:        result += "CON_L7 "; break;
                    case SIG_CON_H1:        result += "CON_L1 "; break;
                    case SIG_CON_H2:        result += "CON_L2 "; break;
                    case SIG_CON_H3:        result += "CON_L3 "; break;
                    case SIG_CON_H4:        result += "CON_L4 "; break;
                    case SIG_CON_H5:        result += "CON_L5 "; break;
                    case SIG_CON_H6:        result += "CON_L6 "; break;
                    case SIG_CON_H7:        result += "CON_L7 "; break;
                }
            }
            
            if (signal & SIG_BMAPEN_CLR)    result += "BMAPEN_CLR ";
            if (signal & SIG_BMAPEN_SET)    result += "BMAPEN_SET ";
            if (signal & SIG_VFLOP_CLR)     result += "VFLOP_CLR ";
            if (signal & SIG_VFLOP_SET)     result += "VFLOP_SET ";
            if (signal & SIG_BPHSTART)      result += "BPHSTART ";
            if (signal & SIG_BPHSTOP)       result += "BPHSTOP ";
            if (signal & SIG_SHW)           result += "SHW ";
            if (signal & SIG_RHW)           result += "RHW ";

            return result != "" ? result : "NONE";
        };
        
        if (sigRecorder.count() == 0) os << "No signals recorded\n";
        
        for (isize i = 0; i < sigRecorder.count(); i++) {
            
            auto trigger = util::hexstr<2>(sigRecorder.keys[i]);
            auto signal = name(sigRecorder.elements[i]);
     
            os << tab("Event at $" + trigger) << signal << std::endl;
        }
    }
    */
}
