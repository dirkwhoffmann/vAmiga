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
#include "ControlPort.h"
#include "CPU.h"
#include "Agnus.h"
#include "Paula.h"
#include "Keyboard.h"
#include "Drive.h"
#include "IOUtils.h"
#include "RemoteManager.h"
#include <iomanip>

const char *
Scheduler::eventName(EventSlot slot, EventID id)
{
    assert_enum(EventSlot, slot);
    
    switch (slot) {

        case SLOT_REG:
            
            switch (id) {

                case EVENT_NONE:    return "none";
                case REG_CHANGE:    return "REG_CHANGE";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_CIAA:
        case SLOT_CIAB:

            switch (id) {
                case EVENT_NONE:    return "none";
                case CIA_EXECUTE:   return "CIA_EXECUTE";
                case CIA_WAKEUP:    return "CIA_WAKEUP";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_BPL:

            switch (id) {
                case EVENT_NONE:           return "none";
                case DRAW_ODD:             return "BPL [O]";
                case DRAW_EVEN:            return "BPL [E]";
                case DRAW_ODD | DRAW_EVEN: return "BPL [OE]";
                case BPL_L1:               return "BPL_L1";
                case BPL_L1 | DRAW_ODD:    return "BPL_L1 [O]";
                case BPL_L1 | DRAW_EVEN:   return "BPL_L1 [E]";
                case BPL_L1 | DRAW_BOTH:   return "BPL_L1 [OE]";
                case BPL_L2:               return "BPL_L2";
                case BPL_L2 | DRAW_ODD:    return "BPL_L2 [O]";
                case BPL_L2 | DRAW_EVEN:   return "BPL_L2 [E]";
                case BPL_L2 | DRAW_BOTH:   return "BPL_L2 [OE]";
                case BPL_L3:               return "BPL_L3";
                case BPL_L3 | DRAW_ODD:    return "BPL_L3 [O]";
                case BPL_L3 | DRAW_EVEN:   return "BPL_L3 [E]";
                case BPL_L3 | DRAW_BOTH:   return "BPL_L3 [OE]";
                case BPL_L4:               return "BPL_L4";
                case BPL_L4 | DRAW_ODD:    return "BPL_L4 [O]";
                case BPL_L4 | DRAW_EVEN:   return "BPL_L4 [E]";
                case BPL_L4 | DRAW_BOTH:   return "BPL_L4 [OE]";
                case BPL_L5:               return "BPL_L5";
                case BPL_L5 | DRAW_ODD:    return "BPL_L5 [O]";
                case BPL_L5 | DRAW_EVEN:   return "BPL_L5 [E]";
                case BPL_L5 | DRAW_BOTH:   return "BPL_L5 [OE]";
                case BPL_L6:               return "BPL_L6";
                case BPL_L6 | DRAW_ODD:    return "BPL_L6 [O]";
                case BPL_L6 | DRAW_EVEN:   return "BPL_L6 [E]";
                case BPL_L6 | DRAW_BOTH:   return "BPL_L6 [OE]";
                case BPL_H1:               return "BPL_H1";
                case BPL_H1 | DRAW_ODD:    return "BPL_H1 [O]";
                case BPL_H1 | DRAW_EVEN:   return "BPL_H1 [E]";
                case BPL_H1 | DRAW_BOTH:   return "BPL_H1 [OE]";
                case BPL_H2:               return "BPL_H2";
                case BPL_H2 | DRAW_ODD:    return "BPL_H2 [O]";
                case BPL_H2 | DRAW_EVEN:   return "BPL_H2 [E]";
                case BPL_H2 | DRAW_BOTH:   return "BPL_H2 [OE]";
                case BPL_H3:               return "BPL_H3";
                case BPL_H3 | DRAW_ODD:    return "BPL_H3 [O]";
                case BPL_H3 | DRAW_EVEN:   return "BPL_H3 [E]";
                case BPL_H3 | DRAW_BOTH:   return "BPL_H3 [OE]";
                case BPL_H4:               return "BPL_H4";
                case BPL_H4 | DRAW_ODD:    return "BPL_H4 [O]";
                case BPL_H4 | DRAW_EVEN:   return "BPL_H4 [E]";
                case BPL_H4 | DRAW_BOTH:   return "BPL_H4 [OE]";
                case BPL_EOL:              return "BPL_EOL";
                case BPL_EOL | DRAW_ODD:   return "BPL_EOL [O]";
                case BPL_EOL | DRAW_EVEN:  return "BPL_EOL [E]";
                case BPL_EOL | DRAW_BOTH:  return "BPL_EOL [OE]";
                default:                   return "*** INVALID ***";
            }
            break;

        case SLOT_DAS:

            switch (id) {
                case EVENT_NONE:    return "none";
                case DAS_REFRESH:   return "DAS_REFRESH";
                case DAS_D0:        return "DAS_D0";
                case DAS_D1:        return "DAS_D1";
                case DAS_D2:        return "DAS_D2";
                case DAS_A0:        return "DAS_A0";
                case DAS_A1:        return "DAS_A1";
                case DAS_A2:        return "DAS_A2";
                case DAS_A3:        return "DAS_A3";
                case DAS_S0_1:      return "DAS_S0_1";
                case DAS_S0_2:      return "DAS_S0_2";
                case DAS_S1_1:      return "DAS_S1_1";
                case DAS_S1_2:      return "DAS_S1_2";
                case DAS_S2_1:      return "DAS_S2_2";
                case DAS_S3_1:      return "DAS_S3_1";
                case DAS_S3_2:      return "DAS_S3_2";
                case DAS_S4_1:      return "DAS_S4_1";
                case DAS_S4_2:      return "DAS_S4_2";
                case DAS_S5_1:      return "DAS_S5_1";
                case DAS_S5_2:      return "DAS_S5_2";
                case DAS_S6_1:      return "DAS_S6_1";
                case DAS_S6_2:      return "DAS_S6_2";
                case DAS_S7_1:      return "DAS_S7_1";
                case DAS_S7_2:      return "DAS_S7_2";
                case DAS_SDMA:      return "DAS_SDMA";
                case DAS_TICK:      return "DAS_TICK";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_COP:

            switch (id) {

                case EVENT_NONE:       return "none";
                case COP_REQ_DMA:      return "COP_REQ_DMA";
                case COP_WAKEUP:       return "COP_WAKEUP";
                case COP_WAKEUP_BLIT:  return "COP_WAKEUP_BLIT";
                case COP_FETCH:        return "COP_FETCH";
                case COP_MOVE:         return "COP_MOVE";
                case COP_WAIT_OR_SKIP: return "WAIT_OR_SKIP";
                case COP_WAIT1:        return "COP_WAIT1";
                case COP_WAIT2:        return "COP_WAIT2";
                case COP_WAIT_BLIT:    return "COP_WAIT_BLIT";
                case COP_SKIP1:        return "COP_SKIP1";
                case COP_SKIP2:        return "COP_SKIP1";
                case COP_JMP1:         return "COP_JMP1";
                case COP_JMP2:         return "COP_JMP2";
                case COP_VBLANK:       return "COP_VBLANK";
                default:               return "*** INVALID ***";
            }
            break;

        case SLOT_BLT:

            switch (id) {

                case EVENT_NONE:    return "none";
                case BLT_STRT1:     return "BLT_STRT1";
                case BLT_STRT2:     return "BLT_STRT2";
                case BLT_COPY_SLOW: return "BLT_COPY_SLOW";
                case BLT_COPY_FAKE: return "BLT_COPY_FAKE";
                case BLT_LINE_SLOW: return "BLT_LINE_SLOW";
                case BLT_LINE_FAKE: return "BLT_LINE_FAKE";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_SEC:

            switch (id) {

                case EVENT_NONE:    return "none";
                case SEC_TRIGGER:   return "SEC_TRIGGER";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_CH0:
        case SLOT_CH1:
        case SLOT_CH2:
        case SLOT_CH3:

            switch (id) {

                case EVENT_NONE:    return "none";
                case CHX_PERFIN:    return "CHX_PERFIN";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_DSK:

            switch (id) {

                case EVENT_NONE:    return "none";
                case DSK_ROTATE:    return "DSK_ROTATE";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_VBL:

            switch (id) {

                case EVENT_NONE:    return "none";
                case VBL_STROBE0:   return "VBL_STROBE0";
                case VBL_STROBE1:   return "VBL_STROBE1";
                case VBL_STROBE2:   return "VBL_STROBE2";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_IRQ:

            switch (id) {

                case EVENT_NONE:    return "none";
                case IRQ_CHECK:     return "IRQ_CHECK";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_IPL:

            switch (id) {

                case EVENT_NONE:    return "none";
                case IPL_CHANGE:    return "IPL_CHANGE";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_KBD:

            switch (id) {

                case EVENT_NONE:    return "none";
                case KBD_TIMEOUT:   return "KBD_TIMEOUT";
                case KBD_DAT:       return "KBD_DAT";
                case KBD_CLK0:      return "KBD_CLK0";
                case KBD_CLK1:      return "KBD_CLK1";
                case KBD_SYNC_DAT0: return "KBD_SYNC_DAT0";
                case KBD_SYNC_CLK0: return "KBD_SYNC_CLK0";
                case KBD_SYNC_DAT1: return "KBD_SYNC_DAT1";
                case KBD_SYNC_CLK1: return "KBD_SYNC_CLK1";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_TXD:

            switch (id) {

                case EVENT_NONE:    return "none";
                case TXD_BIT:       return "TXD_BIT";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_RXD:

            switch (id) {

                case EVENT_NONE:    return "none";
                case RXD_BIT:       return "RXD_BIT";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_POT:

            switch (id) {

                case EVENT_NONE:    return "none";
                case POT_DISCHARGE: return "POT_DISCHARGE";
                case POT_CHARGE:    return "POT_CHARGE";
                default:            return "*** INVALID ***";
            }
            break;
            
        case SLOT_RAS:

            switch (id) {

                case EVENT_NONE:    return "none";
                case RAS_HSYNC:     return "RAS_HSYNC";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_TER:

            switch (id) {

                case EVENT_NONE:    return "none";
                case TER_TRIGGER:   return "TER_TRIGGER";
                default:            return "*** INVALID ***";
            }
            break;

        case SLOT_DC0:
        case SLOT_DC1:
        case SLOT_DC2:
        case SLOT_DC3:

            switch (id) {

                case EVENT_NONE:    return "none";
                case DCH_INSERT:    return "DCH_INSERT";
                case DCH_EJECT:     return "DCH_EJECT";
                default:            return "*** INVALID ***";
            }
            break;
            
        case SLOT_MSE1:
        case SLOT_MSE2:
            
            switch (id) {
                    
                case EVENT_NONE:        return "none";
                case MSE_PUSH_LEFT:     return "MSE_PUSH_LEFT";
                case MSE_RELEASE_LEFT:  return "MSE_RELEASE_LEFT";
                case MSE_PUSH_RIGHT:    return "MSE_PUSH_RIGHT";
                case MSE_RELEASE_RIGHT: return "MSE_RELEASE_RIGHT";
                default:                return "*** INVALID ***";
            }
            break;

        case SLOT_KEY:
            
            switch (id) {
                    
                case EVENT_NONE:        return "none";
                case KEY_PRESS:         return "KEY_PRESS";
                case KEY_RELEASE:       return "KEY_RELEASE";
                default:                return "*** INVALID ***";
            }
            break;

        case SLOT_SRV:
            
            switch (id) {
                    
                case EVENT_NONE:        return "none";
                case SRV_LAUNCH_DAEMON: return "SRV_LAUNCH_DAEMON";
                default:                return "*** INVALID ***";
            }
            break;

        case SLOT_SER:
            
            switch (id) {
                    
                case EVENT_NONE:        return "none";
                case SER_RECEIVE:       return "SER_RECEIVE";
                default:                return "*** INVALID ***";
            }
            break;
            
        case SLOT_INS:

            switch (id) {

                case EVENT_NONE:    return "none";
                case INS_AMIGA:     return "INS_AMIGA";
                case INS_CPU:       return "INS_CPU";
                case INS_MEM:       return "INS_MEM";
                case INS_CIA:       return "INS_CIA";
                case INS_AGNUS:     return "INS_AGNUS";
                case INS_PAULA:     return "INS_PAULA";
                case INS_DENISE:    return "INS_DENISE";
                case INS_PORTS:     return "INS_PORTS";
                case INS_EVENTS:    return "INS_EVENTS";
                default:            return "*** INVALID ***";
            }
            break;
            
        default:
            fatalError;
    }
}

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

    auto &info = slotInfo[nr];
    auto trigger = this->trigger[nr];

    info.slot = nr;
    info.eventId = id[nr];
    info.trigger = trigger;
    info.triggerRel = trigger - agnus.clock;

    if (agnus.belongsToCurrentFrame(trigger)) {
        
        Beam beam = agnus.cycleToBeam(trigger);
        info.vpos = beam.v;
        info.hpos = beam.h;
        info.frameRel = 0;
        
    } else if (agnus.belongsToNextFrame(trigger)) {
        
        info.vpos = 0;
        info.hpos = 0;
        info.frameRel = 1;
        
    } else {
        
        assert(agnus.belongsToPreviousFrame(trigger));
        info.vpos = 0;
        info.hpos = 0;
        info.frameRel = -1;
    }

    info.eventName = eventName((EventSlot)nr, id[nr]);
}

EventSlotInfo
Scheduler::getSlotInfo(isize nr) const
{
    assert_enum(EventSlot, nr);
    
    {   SYNCHRONIZED

        if (!isRunning()) inspectSlot(nr);
        return slotInfo[nr];
    }
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

        if (isDue<SLOT_TER>(cycle)) {

            //
            // Check tertiary slots
            //

            if (isDue<SLOT_DC0>(cycle)) {
                df0.serviceDiskChangeEvent <SLOT_DC0> ();
            }
            if (isDue<SLOT_DC1>(cycle)) {
                df1.serviceDiskChangeEvent <SLOT_DC1> ();
            }
            if (isDue<SLOT_DC2>(cycle)) {
                df2.serviceDiskChangeEvent <SLOT_DC2> ();
            }
            if (isDue<SLOT_DC3>(cycle)) {
                df3.serviceDiskChangeEvent <SLOT_DC3> ();
            }
            if (isDue<SLOT_MSE1>(cycle)) {
                controlPort1.mouse.serviceMouseEvent <SLOT_MSE1> ();
            }
            if (isDue<SLOT_MSE2>(cycle)) {
                controlPort2.mouse.serviceMouseEvent <SLOT_MSE2> ();
            }
            if (isDue<SLOT_KEY>(cycle)) {
                keyboard.serviceKeyEvent();
            }
            if (isDue<SLOT_SRV>(cycle)) {
                remoteManager.serviceServerEvent();
            }
            if (isDue<SLOT_SER>(cycle)) {
                remoteManager.serServer.serviceSerEvent();
            }
            if (isDue<SLOT_INS>(cycle)) {
                agnus.serviceINSEvent(id[SLOT_INS]);
            }

            // Determine the next trigger cycle for all tertiary slots
            Cycle next = trigger[SLOT_TER + 1];
            for (isize i = SLOT_TER + 2; i < SLOT_COUNT; i++) {
                if (trigger[i] < next) next = trigger[i];
            }
            rescheduleAbs<SLOT_TER>(next);
        }
        
        // Determine the next trigger cycle for all secondary slots
        Cycle next = trigger[SLOT_SEC + 1];
        for (isize i = SLOT_SEC + 2; i <= SLOT_TER; i++) {
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
