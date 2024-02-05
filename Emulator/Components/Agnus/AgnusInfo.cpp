// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "IOUtils.h"
#include "CIA.h"
#include "CPU.h"

namespace vamiga {

const char *
Agnus::eventName(EventSlot slot, EventID id)
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
                case EVENT_NONE:                return "none";
                case DRAW_ODD:                  return "BPL [O]";
                case DRAW_EVEN:                 return "BPL [E]";
                case DRAW_ODD | DRAW_EVEN:      return "BPL [OE]";
                case BPL_L1:                    return "BPL_L1";
                case BPL_L1 | DRAW_ODD:         return "BPL_L1 [O]";
                case BPL_L1 | DRAW_EVEN:        return "BPL_L1 [E]";
                case BPL_L1 | DRAW_BOTH:        return "BPL_L1 [OE]";
                case BPL_L1_MOD:                return "BPL_L1*";
                case BPL_L1_MOD | DRAW_ODD:     return "BPL_L1*[O]";
                case BPL_L1_MOD | DRAW_EVEN:    return "BPL_L1*[E]";
                case BPL_L1_MOD | DRAW_BOTH:    return "BPL_L1*[OE]";
                case BPL_L2:                    return "BPL_L2";
                case BPL_L2 | DRAW_ODD:         return "BPL_L2 [O]";
                case BPL_L2 | DRAW_EVEN:        return "BPL_L2 [E]";
                case BPL_L2 | DRAW_BOTH:        return "BPL_L2 [OE]";
                case BPL_L2_MOD:                return "BPL_L2*";
                case BPL_L2_MOD | DRAW_ODD:     return "BPL_L2*[O]";
                case BPL_L2_MOD | DRAW_EVEN:    return "BPL_L2*[E]";
                case BPL_L2_MOD | DRAW_BOTH:    return "BPL_L2*[OE]";
                case BPL_L3:                    return "BPL_L3";
                case BPL_L3 | DRAW_ODD:         return "BPL_L3 [O]";
                case BPL_L3 | DRAW_EVEN:        return "BPL_L3 [E]";
                case BPL_L3 | DRAW_BOTH:        return "BPL_L3 [OE]";
                case BPL_L3_MOD:                return "BPL_L3*";
                case BPL_L3_MOD | DRAW_ODD:     return "BPL_L3*[O]";
                case BPL_L3_MOD | DRAW_EVEN:    return "BPL_L3*[E]";
                case BPL_L3_MOD | DRAW_BOTH:    return "BPL_L3*[OE]";
                case BPL_L4:                    return "BPL_L4";
                case BPL_L4 | DRAW_ODD:         return "BPL_L4 [O]";
                case BPL_L4 | DRAW_EVEN:        return "BPL_L4 [E]";
                case BPL_L4 | DRAW_BOTH:        return "BPL_L4 [OE]";
                case BPL_L4_MOD:                return "BPL_L4*";
                case BPL_L4_MOD | DRAW_ODD:     return "BPL_L4*[O]";
                case BPL_L4_MOD | DRAW_EVEN:    return "BPL_L4*[E]";
                case BPL_L4_MOD | DRAW_BOTH:    return "BPL_L4*[OE]";
                case BPL_L5:                    return "BPL_L5";
                case BPL_L5 | DRAW_ODD:         return "BPL_L5 [O]";
                case BPL_L5 | DRAW_EVEN:        return "BPL_L5 [E]";
                case BPL_L5 | DRAW_BOTH:        return "BPL_L5 [OE]";
                case BPL_L5_MOD:                return "BPL_L5*";
                case BPL_L5_MOD | DRAW_ODD:     return "BPL_L5*[O]";
                case BPL_L5_MOD | DRAW_EVEN:    return "BPL_L5*[E]";
                case BPL_L5_MOD | DRAW_BOTH:    return "BPL_L5*[OE]";
                case BPL_L6:                    return "BPL_L6";
                case BPL_L6 | DRAW_ODD:         return "BPL_L6 [O]";
                case BPL_L6 | DRAW_EVEN:        return "BPL_L6 [E]";
                case BPL_L6 | DRAW_BOTH:        return "BPL_L6 [OE]";
                case BPL_L6_MOD:                return "BPL_L6*";
                case BPL_L6_MOD | DRAW_ODD:     return "BPL_L6*[O]";
                case BPL_L6_MOD | DRAW_EVEN:    return "BPL_L6*[E]";
                case BPL_L6_MOD | DRAW_BOTH:    return "BPL_L6*[OE]";
                case BPL_H1:                    return "BPL_H1";
                case BPL_H1 | DRAW_ODD:         return "BPL_H1 [O]";
                case BPL_H1 | DRAW_EVEN:        return "BPL_H1 [E]";
                case BPL_H1 | DRAW_BOTH:        return "BPL_H1 [OE]";
                case BPL_H1_MOD:                return "BPL_H1*";
                case BPL_H1_MOD | DRAW_ODD:     return "BPL_H1*[O]";
                case BPL_H1_MOD | DRAW_EVEN:    return "BPL_H1*[E]";
                case BPL_H1_MOD | DRAW_BOTH:    return "BPL_H1*[OE]";
                case BPL_H2:                    return "BPL_H2";
                case BPL_H2 | DRAW_ODD:         return "BPL_H2 [O]";
                case BPL_H2 | DRAW_EVEN:        return "BPL_H2 [E]";
                case BPL_H2 | DRAW_BOTH:        return "BPL_H2 [OE]";
                case BPL_H2_MOD:                return "BPL_H2*";
                case BPL_H2_MOD | DRAW_ODD:     return "BPL_H2*[O]";
                case BPL_H2_MOD | DRAW_EVEN:    return "BPL_H2*[E]";
                case BPL_H2_MOD | DRAW_BOTH:    return "BPL_H2*[OE]";
                case BPL_H3:                    return "BPL_H3";
                case BPL_H3 | DRAW_ODD:         return "BPL_H3 [O]";
                case BPL_H3 | DRAW_EVEN:        return "BPL_H3 [E]";
                case BPL_H3 | DRAW_BOTH:        return "BPL_H3 [OE]";
                case BPL_H3_MOD:                return "BPL_H3*";
                case BPL_H3_MOD | DRAW_ODD:     return "BPL_H3*[O]";
                case BPL_H3_MOD | DRAW_EVEN:    return "BPL_H3*[E]";
                case BPL_H3_MOD | DRAW_BOTH:    return "BPL_H3*[OE]";
                case BPL_H4:                    return "BPL_H4";
                case BPL_H4 | DRAW_ODD:         return "BPL_H4 [O]";
                case BPL_H4 | DRAW_EVEN:        return "BPL_H4 [E]";
                case BPL_H4 | DRAW_BOTH:        return "BPL_H4 [OE]";
                case BPL_H4_MOD:                return "BPL_H4*";
                case BPL_H4_MOD | DRAW_ODD:     return "BPL_H4*[O]";
                case BPL_H4_MOD | DRAW_EVEN:    return "BPL_H4*[E]";
                case BPL_H4_MOD | DRAW_BOTH:    return "BPL_H4*[OE]";
                case BPL_S1:                    return "BPL_S1";
                case BPL_S1 | DRAW_ODD:         return "BPL_S1 [O]";
                case BPL_S1 | DRAW_EVEN:        return "BPL_S1 [E]";
                case BPL_S1 | DRAW_BOTH:        return "BPL_S1 [OE]";
                case BPL_S1_MOD:                return "BPL_S1*";
                case BPL_S1_MOD | DRAW_ODD:     return "BPL_S1*[O]";
                case BPL_S1_MOD | DRAW_EVEN:    return "BPL_S1*[E]";
                case BPL_S1_MOD | DRAW_BOTH:    return "BPL_S1*[OE]";
                case BPL_S2:                    return "BPL_S2";
                case BPL_S2 | DRAW_ODD:         return "BPL_S2 [O]";
                case BPL_S2 | DRAW_EVEN:        return "BPL_S2 [E]";
                case BPL_S2 | DRAW_BOTH:        return "BPL_S2 [OE]";
                case BPL_S2_MOD:                return "BPL_S2*";
                case BPL_S2_MOD | DRAW_ODD:     return "BPL_S2*[O]";
                case BPL_S2_MOD | DRAW_EVEN:    return "BPL_S2*[E]";
                case BPL_S2_MOD | DRAW_BOTH:    return "BPL_S2*[OE]";

                default:                        return "*** INVALID ***";
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
                case DAS_A2:        return "DAS_A2, DAS_HSYNC";
                case DAS_A3:        return "DAS_A3";
                case DAS_S0_1:      return "DAS_S0_1";
                case DAS_S0_2:      return "DAS_S0_2";
                case DAS_S1_1:      return "DAS_S1_1";
                case DAS_S1_2:      return "DAS_S1_2";
                case DAS_S2_1:      return "DAS_S2_1";
                case DAS_S2_2:      return "DAS_S2_2";
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
                    // case DAS_HSYNC:     return "DAS_HSYNC";
                case DAS_EOL:       return "DAS_EOL";
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
            
        case SLOT_HD0:
        case SLOT_HD1:
        case SLOT_HD2:
        case SLOT_HD3:

            switch (id) {

                case EVENT_NONE:    return "none";
                case HDR_IDLE:      return "HDR_IDLE";
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

        case SLOT_RSH:

            switch (id) {

                case EVENT_NONE:        return "none";
                case RSH_WAKEUP:        return "RSH_WAKEUP";
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

        case SLOT_WBT:

            switch (id) {

                case EVENT_NONE:        return "none";
                case WBT_DISABLE:       return "WBT_DISABLE";
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

        case SLOT_ALA:

            switch (id) {

                case EVENT_NONE:    return "none";
                case ALA_TRIGGER:   return "ALA_TRIGGER";
                default:            return "*** INVALID ***";
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
Agnus::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {

        os << tab("Chip Revison");
        os << AgnusRevisionEnum::key(config.revision) << std::endl;
        os << tab("Slow Ram mirror");
        os << bol(config.slowRamMirror) << std::endl;
        os << tab("Pointer drops");
        os << bol(config.ptrDrops) << std::endl;
    }
    
    if (category == Category::State) {
        
        os << tab("Clock");
        os << dec(clock) << std::endl;
        os << tab("Resolution");
        os << ResolutionEnum::key(res) << std::endl;
        os << tab("scrollOdd");
        os << dec(scrollOdd) << std::endl;
        os << tab("scrollEven");
        os << dec(scrollEven) << std::endl;
        os << tab("BLS signal");
        os << bol(bls) << std::endl;
        
        // sequencer.dump(Category::State, os);
    }

    if (category == Category::Beam) {

        os << tab("Frame");
        os << dec(pos.frame) << bol(isPAL(), " (PAL)", " (NTSC)") << std::endl;
        os << tab("Position");
        os << "(" << dec(pos.v) << "," << dec(pos.h) << ")" << std::endl;
        os << tab("Latched");
        os << "(" << dec(latchedPos.v) << "," << dec(latchedPos.h) << ")" << std::endl;
        os << tab("LOF");
        os << dec(pos.lof) << std::endl;
        os << tab("LOF toggle");
        os << dec(pos.lofToggle) << std::endl;
        os << tab("LOL");
        os << dec(pos.lol) << std::endl;
        os << tab("LOL toggle");
        os << dec(pos.lolToggle) << std::endl;
    }

    if (category == Category::Registers) {

        os << tab("DMACON") << hex(dmacon) << std::endl;
        os << tab("BPLCON0") << hex(bplcon0) << std::endl;
        os << std::endl;
        os << tab("DDFSTRT") << hex(sequencer.ddfstrt) << std::endl;
        os << tab("DDFSTOP") << hex(sequencer.ddfstop) << std::endl;
        os << std::endl;
        os << tab("DIWSTRT") << hex(sequencer.diwstrt) << std::endl;
        os << tab("DIWSTOP") << hex(sequencer.diwstop) << std::endl;
        os << std::endl;
        os << tab("DSKPT") << hex(dskpt) << std::endl;
        os << std::endl;
        os << tab("BPL0PT") << hex(bplpt[0]) << "  BPL1PT : " << hex(bplpt[1]) << std::endl;
        os << tab("BPL2PT") << hex(bplpt[2]) << "  BPL3PT : " << hex(bplpt[3]) << std::endl;
        os << tab("BPL4PT") << hex(bplpt[4]) << "  BPL5PT : " << hex(bplpt[5]) << std::endl;
        os << std::endl;
        os << tab("SPR0PT") << hex(sprpt[0]) << "  SPR1PT : " << hex(sprpt[1]) << std::endl;
        os << tab("SPR2PT") << hex(sprpt[2]) << "  SPR3PT : " << hex(sprpt[3]) << std::endl;
        os << tab("SPR4PT") << hex(sprpt[4]) << "  SPR5PT : " << hex(sprpt[5]) << std::endl;
        os << tab("SPR6PT") << hex(sprpt[6]) << "  SPR7PT : " << hex(sprpt[7]) << std::endl;
        os << std::endl;
        os << tab("AUD0PT") << hex(audpt[0]) << "  AUD1PT : " << hex(audpt[1]) << std::endl;
        os << tab("AUD2PT") << hex(audpt[2]) << "  AUD3PT : " << hex(audpt[3]) << std::endl;
        os << std::endl;
        os << tab("AUD0LC") << hex(audlc[0]) << "  AUD1LC : " << hex(audlc[1]) << std::endl;
        os << tab("AUD2LC") << hex(audlc[2]) << "  AUD3LC : " << hex(audlc[3]) << std::endl;
    }
    
    if (category == Category::Bus) {
        
        for (isize i = 0; i < HPOS_CNT_NTSC; i++) {
            
            isize cycle = (i / 6) + (i % 6) * ((HPOS_CNT_NTSC + 1) / 6);
            
            string key = std::to_string(cycle) + ":";
            os << std::left << std::setw(5) << key;
            
            string value = BusOwnerEnum::key(busOwner[cycle]);
            os << std::left << std::setw(8) << value;
            
            if (i % 6 == 5) { os << std::endl; } else { os << "  "; }
        }
        os << std::endl;
    }
    
    if (category == Category::Events) {

        inspect();
        
        os << std::left << std::setw(10) << "Slot";
        os << std::left << std::setw(14) << "Event";
        os << std::left << std::setw(18) << "Trigger position";
        os << std::left << std::setw(16) << "Trigger cycle" << std::endl;
        
        for (isize i = 0; i < SLOT_COUNT; i++) {
            
            EventSlotInfo &info = slotInfo[i];
            
            os << std::left << std::setw(10) << EventSlotEnum::key(info.slot);
            os << std::left << std::setw(14) << info.eventName;
            
            if (info.trigger != NEVER) {
                
                if (info.frameRel < 0) {
                    os << std::left << std::setw(18) << "previous frame";
                } else if (info.frameRel > 0) {
                    os << std::left << std::setw(18) << "upcoming frame";
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
    
    if (category == Category::Dma) {
        
        sequencer.dump(Category::Dma, os);
    }
    
    if (category == Category::Signals) {
        
        sequencer.dump(Category::Signals, os);
    }
}

void
Agnus::_inspect() const
{
    SYNCHRONIZED
    
    info.vpos     = pos.v;
    info.hpos     = pos.h;
    
    info.dmacon   = dmacon;
    info.bplcon0  = bplcon0;
    info.ddfstrt  = sequencer.ddfstrt;
    info.ddfstop  = sequencer.ddfstop;
    info.diwstrt  = sequencer.diwstrt;
    info.diwstop  = sequencer.diwstop;
    
    info.bpl1mod  = bpl1mod;
    info.bpl2mod  = bpl2mod;
    info.bltamod  = blitter.bltamod;
    info.bltbmod  = blitter.bltbmod;
    info.bltcmod  = blitter.bltcmod;
    info.bltdmod  = blitter.bltdmod;
    info.bltcon0  = blitter.bltcon0;
    info.bls      = bls;
    
    info.coppc0   = copper.coppc0 & ptrMask;
    info.dskpt    = dskpt & ptrMask;
    info.bltpt[0] = blitter.bltapt & ptrMask;
    info.bltpt[1] = blitter.bltbpt & ptrMask;
    info.bltpt[2] = blitter.bltcpt & ptrMask;
    info.bltpt[3] = blitter.bltdpt & ptrMask;
    for (isize i = 0; i < 6; i++) info.bplpt[i] = bplpt[i] & ptrMask;
    for (isize i = 0; i < 4; i++) info.audpt[i] = audpt[i] & ptrMask;
    for (isize i = 0; i < 4; i++) info.audlc[i] = audlc[i] & ptrMask;
    for (isize i = 0; i < 8; i++) info.sprpt[i] = sprpt[i] & ptrMask;
    
    eventInfo.cpuClock = cpu.getMasterClock();
    eventInfo.cpuCycles = cpu.getCpuClock();
    eventInfo.dmaClock = agnus.clock;
    eventInfo.ciaAClock = ciaa.getClock();
    eventInfo.ciaBClock  = ciab.getClock();
    eventInfo.frame = agnus.pos.frame;
    eventInfo.vpos = agnus.pos.v;
    eventInfo.hpos = agnus.pos.h;
    
    for (EventSlot i = 0; i < SLOT_COUNT; i++) {
        inspectSlot(i);
    }
}

void
Agnus::inspectSlot(EventSlot nr) const
{
    assert_enum(EventSlot, nr);
    
    auto &info = slotInfo[nr];
    auto cycle = trigger[nr];

    info.slot = nr;
    info.eventId = id[nr];
    info.trigger = cycle;
    info.triggerRel = cycle - agnus.clock;

    auto beam = pos + isize(AS_DMA_CYCLES(cycle - clock));

    info.vpos = beam.v;
    info.hpos = beam.h;
    info.frameRel = long(beam.frame - pos.frame);

    info.eventName = eventName((EventSlot)nr, id[nr]);
}

EventSlotInfo
Agnus::getSlotInfo(isize nr) const
{
    assert_enum(EventSlot, nr);
    
    {   SYNCHRONIZED
        
        if (!isRunning()) inspectSlot(nr);
        return slotInfo[nr];
    }
}


void
Agnus::clearStats()
{
    stats = { };
}

void
Agnus::updateStats()
{
    constexpr double w = 0.5;
    
    double copperUsage = stats.usage[BUS_COPPER];
    double blitterUsage = stats.usage[BUS_BLITTER];
    double diskUsage = stats.usage[BUS_DISK];
    
    double audioUsage =
    stats.usage[BUS_AUD0] +
    stats.usage[BUS_AUD1] +
    stats.usage[BUS_AUD2] +
    stats.usage[BUS_AUD3];

    double spriteUsage =
    stats.usage[BUS_SPRITE0] +
    stats.usage[BUS_SPRITE1] +
    stats.usage[BUS_SPRITE2] +
    stats.usage[BUS_SPRITE3] +
    stats.usage[BUS_SPRITE4] +
    stats.usage[BUS_SPRITE5] +
    stats.usage[BUS_SPRITE6] +
    stats.usage[BUS_SPRITE7];
    
    double bitplaneUsage =
    stats.usage[BUS_BPL1] +
    stats.usage[BUS_BPL2] +
    stats.usage[BUS_BPL3] +
    stats.usage[BUS_BPL4] +
    stats.usage[BUS_BPL5] +
    stats.usage[BUS_BPL6];

    stats.copperActivity = w * stats.copperActivity + (1 - w) * copperUsage;
    stats.blitterActivity = w * stats.blitterActivity + (1 - w) * blitterUsage;
    stats.diskActivity = w * stats.diskActivity + (1 - w) * diskUsage;
    stats.audioActivity = w * stats.audioActivity + (1 - w) * audioUsage;
    stats.spriteActivity = w * stats.spriteActivity + (1 - w) * spriteUsage;
    stats.bitplaneActivity = w * stats.bitplaneActivity + (1 - w) * bitplaneUsage;
    
    for (isize i = 0; i < BUS_COUNT; i++) stats.usage[i] = 0;
}

}
