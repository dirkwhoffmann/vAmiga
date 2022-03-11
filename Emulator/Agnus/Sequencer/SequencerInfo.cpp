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
#include "Agnus.h"
#include "IOUtils.h"

void
Sequencer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::State) {

        os << tab("BPV");
        os << bol(ddf.bpv) << " (" << bol(ddfInitial.bpv) << ")" << std::endl;
        os << tab("BMAPEN");
        os << bol(ddf.bmapen) << " (" << bol(ddfInitial.bmapen) << ")" << std::endl;
        os << tab("SHW");
        os << bol(ddf.shw) << " (" << bol(ddfInitial.shw) << ")" << std::endl;
        os << tab("RHW");
        os << bol(ddf.rhw) << " (" << bol(ddfInitial.rhw) << ")" << std::endl;
        os << tab("BPHSTART");
        os << bol(ddf.bphstart) << " (" << bol(ddfInitial.bphstart) << ")" << std::endl;
        os << tab("BPHSTOP");
        os << bol(ddf.bphstop) << " (" << bol(ddfInitial.bphstop) << ")" << std::endl;
        os << tab("BPRRUN");
        os << bol(ddf.bprun) << " (" << bol(ddfInitial.bprun) << ")" << std::endl;
        os << tab("LASTFU");
        os << bol(ddf.lastFu) << " (" << bol(ddfInitial.lastFu) << ")" << std::endl;        
        os << tab("BMCTL");
        os << hex(ddf.bmctl) << " (" << hex(ddfInitial.bmctl) << ")" << std::endl;
        os << tab("CNT");
        os << dec(ddf.cnt) << " (" << dec(ddfInitial.cnt) << ")" << std::endl;
    }
        
    if (category & dump::Registers) {
        
        os << tab("DDFSTRT");
        os << hex(ddfstrt) << std::endl;
        os << tab("DDFSTOP");
        os << hex(ddfstop) << std::endl;
        os << tab("DIWSTART");
        os << hex(diwstrt) << std::endl;
        os << tab("DIWSTOP");
        os << hex(diwstop) << std::endl;
    }
        
    if (category & dump::Dma) {

        for (isize row = 0; row < HPOS_CNT; row++) {
            
            isize i = (row / 2) + ((row % 2) ? ((HPOS_CNT + 1) / 2) : 0);
            
            string cycle = std::to_string(i) + ":";
            os << std::left << std::setw(5) << cycle;

            string bpl = Agnus::eventName(SLOT_BPL, bplEvent[i]);
            os << std::left << std::setw(12) << bpl;
            os << " + ";
            string das = Agnus::eventName(SLOT_DAS, dasEvent[i]);
            os << std::left << std::setw(12) << das;
            
            string next = "-> ";
            next += std::to_string(nextBplEvent[i]) + ",";
            next += std::to_string(nextDasEvent[i]);
            os << std::left << std::setw(14) << next;

            if (row % 2) { os << std::endl; } else { os << "  "; }
        }
        os << std::endl; 
    }
    
    if (category & dump::Signals) {
              
        auto name = [](u16 signal) -> string {
        
            string result;
            
            if (signal & 0x10) {
               
                switch (signal & 0x1f) {

                    case SIG_CON_L0:        result += "CON_L0 "; break;
                    case SIG_CON_L1:        result += "CON_L1 "; break;
                    case SIG_CON_L2:        result += "CON_L2 "; break;
                    case SIG_CON_L3:        result += "CON_L3 "; break;
                    case SIG_CON_L4:        result += "CON_L4 "; break;
                    case SIG_CON_L5:        result += "CON_L5 "; break;
                    case SIG_CON_L6:        result += "CON_L6 "; break;
                    case SIG_CON_L7:        result += "CON_L7 "; break;
                    case SIG_CON_H0:        result += "CON_H0 "; break;
                    case SIG_CON_H1:        result += "CON_H1 "; break;
                    case SIG_CON_H2:        result += "CON_H2 "; break;
                    case SIG_CON_H3:        result += "CON_H3 "; break;
                    case SIG_CON_H4:        result += "CON_H4 "; break;
                    case SIG_CON_H5:        result += "CON_H5 "; break;
                    case SIG_CON_H6:        result += "CON_H6 "; break;
                    case SIG_CON_H7:        result += "CON_H7 "; break;
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
            if (signal & SIG_DONE)          result += "DONE ";

            return result != "" ? result : "NONE";
        };
        
        if (sigRecorder.count() == 0) os << "No signals recorded\n";
        
        for (isize i = 0; i < sigRecorder.count(); i++) {
            
            auto trigger = util::hexstr<2>((isize)sigRecorder.keys[i]);
            auto signal = name(sigRecorder.elements[i]);
     
            os << tab("Event at $" + trigger) << signal << std::endl;
        }
    }
}
