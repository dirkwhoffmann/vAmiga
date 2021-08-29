// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Scheduler.h"
#include "CIA.h"
#include "CPU.h"
#include "Agnus.h"
#include "Paula.h"
#include "Keyboard.h"
#include "IO.h"
#include <iomanip>

void
Scheduler::_initialize()
{

}

void
Scheduler::_reset(bool hard)
{
    auto insEvent = id[SLOT_INS];
    
    RESET_SNAPSHOT_ITEMS(hard)
    
    // Initialize all event slots
    for (isize i = 0; i < SLOT_COUNT; i++) {
        
        trigger[i] = NEVER;
        id[i] = (EventID)0;
        data[i] = 0;
    }
    
    // Reschedule the old inspection event if there was one
    if (insEvent) scheduleAbs <SLOT_INS> (0, insEvent);

}

void
Scheduler::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::State) {
                
        os << std::left << std::setw(10) << "Slot";
        os << std::left << std::setw(14) << "Event";
        os << std::left << std::setw(18) << "Trigger position";
        os << std::left << std::setw(16) << "Trigger cycle" << std::endl;
        
        for (isize i = 0; i < 23; i++) {
            
            EventSlotInfo &info = slotInfo[i];
                        
            os << std::left << std::setw(10) << EventSlotEnum::key(info.slot);
            os << std::left << std::setw(14) << info.eventName;
            
            if (info.trigger != NEVER) {
                
                if (info.frameRel == -1) {
                    os << std::left << std::setw(18) << "previous frame";
                } else if (info.frameRel > 0) {
                    os << std::left << std::setw(18) << "other frame";
                } else {
                    string vpos = std::to_string(info.vpos);
                    string hpos = std::to_string(info.hpos);
                    string pos = "(" + vpos + "," + hpos + ")";
                    os << std::left << std::setw(18) << pos;
                }
                
                if (info.triggerRel == 0) {
                    os << std::left << std::setw(16) << "due immediately";
                } else {
                    string cycle = std::to_string(info.triggerRel / 8);
                    os << std::left << std::setw(16) << "due in " + cycle + " DMA cycles";
                }
            }
            os << std::endl;
        }
    }
}

void
Scheduler::_inspect() const
{
    synchronized {
        
        info.cpuClock = cpu.getMasterClock();
        info.cpuCycles = cpu.getCpuClock();
        info.dmaClock = agnus.clock;
        info.ciaAClock = ciaa.getClock();
        info.ciaBClock  = ciab.getClock();
        info.frame = agnus.frame.nr;
        info.vpos = agnus.pos.v;
        info.hpos = agnus.pos.h;
        
        for (EventSlot i = 0; i < SLOT_COUNT; i++) {
            inspectSlot(i);
        }
    }
}

void
Scheduler::inspectSlot(EventSlot nr) const
{
    assert_enum(EventSlot, nr);

    // auto &slot = scheduler.slot;
    auto &i = slotInfo[nr];
    auto trigger = this->trigger[nr];

    i.slot = nr;
    i.eventId = id[nr];
    i.trigger = trigger;
    i.triggerRel = trigger - agnus.clock;

    if (agnus.belongsToCurrentFrame(trigger)) {
        
        Beam beam = agnus.cycleToBeam(trigger);
        i.vpos = beam.v;
        i.hpos = beam.h;
        i.frameRel = 0;
        
    } else if (agnus.belongsToNextFrame(trigger)) {
        
        i.vpos = 0;
        i.hpos = 0;
        i.frameRel = 1;
        
    } else {
        
        assert(agnus.belongsToPreviousFrame(trigger));
        i.vpos = 0;
        i.hpos = 0;
        i.frameRel = -1;
    }

    switch ((EventSlot)nr) {

        case SLOT_REG:
            
            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case REG_CHANGE:    i.eventName = "REG_CHANGE"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_CIAA:
        case SLOT_CIAB:

            switch (id[nr]) {
                case 0:             i.eventName = "none"; break;
                case CIA_EXECUTE:   i.eventName = "CIA_EXECUTE"; break;
                case CIA_WAKEUP:    i.eventName = "CIA_WAKEUP"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_BPL:

            switch (id[nr]) {
                case 0:                    i.eventName = "none"; break;
                case DRAW_ODD:             i.eventName = "BPL [O]"; break;
                case DRAW_EVEN:            i.eventName = "BPL [E]"; break;
                case DRAW_ODD | DRAW_EVEN: i.eventName = "BPL [OE]"; break;
                case BPL_L1:               i.eventName = "BPL_L1"; break;
                case BPL_L1 | DRAW_ODD:    i.eventName = "BPL_L1 [O]"; break;
                case BPL_L1 | DRAW_EVEN:   i.eventName = "BPL_L1 [E]"; break;
                case BPL_L1 | DRAW_BOTH:   i.eventName = "BPL_L1 [OE]"; break;
                case BPL_L2:               i.eventName = "BPL_L2"; break;
                case BPL_L2 | DRAW_ODD:    i.eventName = "BPL_L2 [O]"; break;
                case BPL_L2 | DRAW_EVEN:   i.eventName = "BPL_L2 [E]"; break;
                case BPL_L2 | DRAW_BOTH:   i.eventName = "BPL_L2 [OE]"; break;
                case BPL_L3:               i.eventName = "BPL_L3"; break;
                case BPL_L3 | DRAW_ODD:    i.eventName = "BPL_L3 [O]"; break;
                case BPL_L3 | DRAW_EVEN:   i.eventName = "BPL_L3 [E]"; break;
                case BPL_L3 | DRAW_BOTH:   i.eventName = "BPL_L3 [OE]"; break;
                case BPL_L4:               i.eventName = "BPL_L4"; break;
                case BPL_L4 | DRAW_ODD:    i.eventName = "BPL_L4 [O]"; break;
                case BPL_L4 | DRAW_EVEN:   i.eventName = "BPL_L4 [E]"; break;
                case BPL_L4 | DRAW_BOTH:   i.eventName = "BPL_L4 [OE]"; break;
                case BPL_L5:               i.eventName = "BPL_L5"; break;
                case BPL_L5 | DRAW_ODD:    i.eventName = "BPL_L5 [O]"; break;
                case BPL_L5 | DRAW_EVEN:   i.eventName = "BPL_L5 [E]"; break;
                case BPL_L5 | DRAW_BOTH:   i.eventName = "BPL_L5 [OE]"; break;
                case BPL_L6:               i.eventName = "BPL_L6"; break;
                case BPL_L6 | DRAW_ODD:    i.eventName = "BPL_L6 [O]"; break;
                case BPL_L6 | DRAW_EVEN:   i.eventName = "BPL_L6 [E]"; break;
                case BPL_L6 | DRAW_BOTH:   i.eventName = "BPL_L6 [OE]"; break;
                case BPL_H1:               i.eventName = "BPL_H1"; break;
                case BPL_H1 | DRAW_ODD:    i.eventName = "BPL_H1 [O]"; break;
                case BPL_H1 | DRAW_EVEN:   i.eventName = "BPL_H1 [E]"; break;
                case BPL_H1 | DRAW_BOTH:   i.eventName = "BPL_H1 [OE]"; break;
                case BPL_H2:               i.eventName = "BPL_H2"; break;
                case BPL_H2 | DRAW_ODD:    i.eventName = "BPL_H2 [O]"; break;
                case BPL_H2 | DRAW_EVEN:   i.eventName = "BPL_H2 [E]"; break;
                case BPL_H2 | DRAW_BOTH:   i.eventName = "BPL_H2 [OE]"; break;
                case BPL_H3:               i.eventName = "BPL_H3"; break;
                case BPL_H3 | DRAW_ODD:    i.eventName = "BPL_H3 [O]"; break;
                case BPL_H3 | DRAW_EVEN:   i.eventName = "BPL_H3 [E]"; break;
                case BPL_H3 | DRAW_BOTH:   i.eventName = "BPL_H3 [OE]"; break;
                case BPL_H4:               i.eventName = "BPL_H4"; break;
                case BPL_H4 | DRAW_ODD:    i.eventName = "BPL_H4 [O]"; break;
                case BPL_H4 | DRAW_EVEN:   i.eventName = "BPL_H4 [E]"; break;
                case BPL_H4 | DRAW_BOTH:   i.eventName = "BPL_H4 [OE]"; break;
                case BPL_EOL:              i.eventName = "BPL_EOL"; break;
                case BPL_EOL | DRAW_ODD:   i.eventName = "BPL_EOL [O]"; break;
                case BPL_EOL | DRAW_EVEN:  i.eventName = "BPL_EOL [E]"; break;
                case BPL_EOL | DRAW_BOTH:  i.eventName = "BPL_EOL [OE]"; break;
                default:                   i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_DAS:

            switch (id[nr]) {
                case 0:             i.eventName = "none"; break;
                case DAS_REFRESH:   i.eventName = "DAS_REFRESH"; break;
                case DAS_D0:        i.eventName = "DAS_D0"; break;
                case DAS_D1:        i.eventName = "DAS_D1"; break;
                case DAS_D2:        i.eventName = "DAS_D2"; break;
                case DAS_A0:        i.eventName = "DAS_A0"; break;
                case DAS_A1:        i.eventName = "DAS_A1"; break;
                case DAS_A2:        i.eventName = "DAS_A2"; break;
                case DAS_A3:        i.eventName = "DAS_A3"; break;
                case DAS_S0_1:      i.eventName = "DAS_S0_1"; break;
                case DAS_S0_2:      i.eventName = "DAS_S0_2"; break;
                case DAS_S1_1:      i.eventName = "DAS_S1_1"; break;
                case DAS_S1_2:      i.eventName = "DAS_S1_2"; break;
                case DAS_S2_1:      i.eventName = "DAS_S2_2"; break;
                case DAS_S3_1:      i.eventName = "DAS_S3_1"; break;
                case DAS_S3_2:      i.eventName = "DAS_S3_2"; break;
                case DAS_S4_1:      i.eventName = "DAS_S4_1"; break;
                case DAS_S4_2:      i.eventName = "DAS_S4_2"; break;
                case DAS_S5_1:      i.eventName = "DAS_S5_1"; break;
                case DAS_S5_2:      i.eventName = "DAS_S5_2"; break;
                case DAS_S6_1:      i.eventName = "DAS_S6_1"; break;
                case DAS_S6_2:      i.eventName = "DAS_S6_2"; break;
                case DAS_S7_1:      i.eventName = "DAS_S7_1"; break;
                case DAS_S7_2:      i.eventName = "DAS_S7_2"; break;
                case DAS_SDMA:      i.eventName = "DAS_SDMA"; break;
                case DAS_TICK:      i.eventName = "DAS_TICK"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_COP:

            switch (id[nr]) {

                case 0:                i.eventName = "none"; break;
                case COP_REQ_DMA:      i.eventName = "COP_REQ_DMA"; break;
                case COP_WAKEUP:       i.eventName = "COP_WAKEUP"; break;
                case COP_WAKEUP_BLIT:  i.eventName = "COP_WAKEUP_BLIT"; break;
                case COP_FETCH:        i.eventName = "COP_FETCH"; break;
                case COP_MOVE:         i.eventName = "COP_MOVE"; break;
                case COP_WAIT_OR_SKIP: i.eventName = "WAIT_OR_SKIP"; break;
                case COP_WAIT1:        i.eventName = "COP_WAIT1"; break;
                case COP_WAIT2:        i.eventName = "COP_WAIT2"; break;
                case COP_WAIT_BLIT:    i.eventName = "COP_WAIT_BLIT"; break;
                case COP_SKIP1:        i.eventName = "COP_SKIP1"; break;
                case COP_SKIP2:        i.eventName = "COP_SKIP1"; break;
                case COP_JMP1:         i.eventName = "COP_JMP1"; break;
                case COP_JMP2:         i.eventName = "COP_JMP2"; break;
                case COP_VBLANK:       i.eventName = "COP_VBLANK"; break;
                default:               i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_BLT:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case BLT_STRT1:     i.eventName = "BLT_STRT1"; break;
                case BLT_STRT2:     i.eventName = "BLT_STRT2"; break;
                case BLT_COPY_SLOW: i.eventName = "BLT_COPY_SLOW"; break;
                case BLT_COPY_FAKE: i.eventName = "BLT_COPY_FAKE"; break;
                case BLT_LINE_FAKE: i.eventName = "BLT_LINE_FAKE"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_SEC:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case SEC_TRIGGER:   i.eventName = "SEC_TRIGGER"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_CH0:
        case SLOT_CH1:
        case SLOT_CH2:
        case SLOT_CH3:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case CHX_PERFIN:    i.eventName = "CHX_PERFIN"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_DSK:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case DSK_ROTATE:    i.eventName = "DSK_ROTATE"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_DCH:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case DCH_INSERT:    i.eventName = "DCH_INSERT"; break;
                case DCH_EJECT:     i.eventName = "DCH_EJECT"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_VBL:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case VBL_STROBE0:   i.eventName = "VBL_STROBE0"; break;
                case VBL_STROBE1:   i.eventName = "VBL_STROBE1"; break;
                case VBL_STROBE2:   i.eventName = "VBL_STROBE2"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_IRQ:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case IRQ_CHECK:     i.eventName = "IRQ_CHECK"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_IPL:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case IPL_CHANGE:    i.eventName = "IPL_CHANGE"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_KBD:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case KBD_TIMEOUT:   i.eventName = "KBD_TIMEOUT"; break;
                case KBD_DAT:       i.eventName = "KBD_DAT"; break;
                case KBD_CLK0:      i.eventName = "KBD_CLK0"; break;
                case KBD_CLK1:      i.eventName = "KBD_CLK1"; break;
                case KBD_SYNC_DAT0: i.eventName = "KBD_SYNC_DAT0"; break;
                case KBD_SYNC_CLK0: i.eventName = "KBD_SYNC_CLK0"; break;
                case KBD_SYNC_DAT1: i.eventName = "KBD_SYNC_DAT1"; break;
                case KBD_SYNC_CLK1: i.eventName = "KBD_SYNC_CLK1"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_TXD:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case TXD_BIT:       i.eventName = "TXD_BIT"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_RXD:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case RXD_BIT:       i.eventName = "RXD_BIT"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_POT:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case POT_DISCHARGE: i.eventName = "POT_DISCHARGE"; break;
                case POT_CHARGE:    i.eventName = "POT_CHARGE"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;
            
        case SLOT_INS:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case INS_AMIGA:     i.eventName = "INS_AMIGA"; break;
                case INS_CPU:       i.eventName = "INS_CPU"; break;
                case INS_MEM:       i.eventName = "INS_MEM"; break;
                case INS_CIA:       i.eventName = "INS_CIA"; break;
                case INS_AGNUS:     i.eventName = "INS_AGNUS"; break;
                case INS_PAULA:     i.eventName = "INS_PAULA"; break;
                case INS_DENISE:    i.eventName = "INS_DENISE"; break;
                case INS_PORTS:     i.eventName = "INS_PORTS"; break;
                case INS_EVENTS:    i.eventName = "INS_EVENTS"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        case SLOT_RAS:

            switch (id[nr]) {

                case 0:             i.eventName = "none"; break;
                case RAS_HSYNC:     i.eventName = "RAS_HSYNC"; break;
                default:            i.eventName = "*** INVALID ***"; break;
            }
            break;

        default:
            fatalError;
    }
}

EventSlotInfo
Scheduler::getSlotInfo(isize nr) const
{
    assert_enum(EventSlot, nr);
    
    if (!isRunning()) inspectSlot(nr);
    
    synchronized { return slotInfo[nr]; }
    unreachable;
}

void
Scheduler::executeUntil(Cycle cycle) {

    //
    // Check primary slots
    //

    if (isDue<SLOT_REG>(cycle)) {
        agnus.serviceREGEvent(cycle);
    }
    if (isDue<SLOT_CIAA>(cycle)) {
        ciaa.serviceEvent(scheduler.id[SLOT_CIAA]);
    }
    if (isDue<SLOT_CIAB>(cycle)) {
        ciab.serviceEvent(scheduler.id[SLOT_CIAB]);
    }
    if (isDue<SLOT_BPL>(cycle)) {
        agnus.serviceBPLEvent(scheduler.id[SLOT_BPL]);
    }
    if (isDue<SLOT_DAS>(cycle)) {
        agnus.serviceDASEvent(scheduler.id[SLOT_DAS]);
    }
    if (isDue<SLOT_COP>(cycle)) {
        copper.serviceEvent(id[SLOT_COP]);
    }
    if (isDue<SLOT_BLT>(cycle)) {
        blitter.serviceEvent(id[SLOT_BLT]);
    }

    if (isDue<SLOT_SEC>(cycle)) {

        //
        // Check secondary slots
        //

        if (isDue<SLOT_CH0>(cycle)) {
            paula.channel0.serviceEvent();
        }
        if (isDue<SLOT_CH1>(cycle)) {
            paula.channel1.serviceEvent();
        }
        if (isDue<SLOT_CH2>(cycle)) {
            paula.channel2.serviceEvent();
        }
        if (isDue<SLOT_CH3>(cycle)) {
            paula.channel3.serviceEvent();
        }
        if (isDue<SLOT_DSK>(cycle)) {
            paula.diskController.serviceDiskEvent();
        }
        if (isDue<SLOT_DCH>(cycle)) {
            paula.diskController.serviceDiskChangeEvent();
        }
        if (isDue<SLOT_VBL>(cycle)) {
            agnus.serviceVblEvent(scheduler.id[SLOT_VBL]);
        }
        if (isDue<SLOT_IRQ>(cycle)) {
            paula.serviceIrqEvent();
        }
        if (isDue<SLOT_KBD>(cycle)) {
            keyboard.serviceKeyboardEvent(id[SLOT_KBD]);
        }
        if (isDue<SLOT_TXD>(cycle)) {
            uart.serviceTxdEvent(id[SLOT_TXD]);
        }
        if (isDue<SLOT_RXD>(cycle)) {
            uart.serviceRxdEvent(id[SLOT_RXD]);
        }
        if (isDue<SLOT_POT>(cycle)) {
            paula.servicePotEvent(id[SLOT_POT]);
        }
        if (isDue<SLOT_IPL>(cycle)) {
            paula.serviceIplEvent();
        }
        if (isDue<SLOT_RAS>(cycle)) {
            agnus.serviceRASEvent();
        }
        if (isDue<SLOT_INS>(cycle)) {
            agnus.serviceINSEvent(id[SLOT_INS]);
        }

        // Determine the next trigger cycle for all secondary slots
        Cycle next = trigger[SLOT_SEC + 1];
        for (isize i = SLOT_SEC + 2; i < SLOT_COUNT; i++) {
            if (trigger[i] < next) next = trigger[i];
        }
        rescheduleAbs<SLOT_SEC>(next);
    }

    // Determine the next trigger cycle for all primary slots
    Cycle next = trigger[0];
    for (isize i = 1; i <= SLOT_SEC; i++) {
        if (trigger[i] < next) next = trigger[i];
    }
    scheduler.nextTrigger = next;
}
