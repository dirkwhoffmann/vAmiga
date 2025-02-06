// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "Agnus.h"
#include "Amiga.h"
#include "CIA.h"
#include "ControlPort.h"
#include "CPU.h"
#include "Denise.h"
#include "Paula.h"
#include "SerialPort.h"

namespace vamiga {

/* About the event scheduler
 *
 * vAmiga is an event triggered emulator. If an action has to be performed at
 * a specific DMA cycle (e.g., activating the Copper at a certain beam
 * position), the action is scheduled via the event handling API and executed
 * when the trigger cycle has been reached.
 * Scheduled events are stored in so called event slots. Each slot is either
 * empty or contains a single event and is bound to a specific component. E.g.,
 * there is slot for Copper events, a slot for the Blitter events, a slot
 * for UART event, and so forth.
 * From a theoretical point of view, each event slot represents a state machine
 * running in parallel to the ones in the other slots. Keep in mind that the
 * state machines interact with each other in various ways (e.g., by blocking
 * the DMA bus). As a result, the slot ordering is important: If two events
 * trigger at the same cycle, the the slot with a smaller number is served
 * first.
 * To optimize speed, the event slots are categorized into primary, secondary,
 * and tertiary slots. Primary slots manage frequently occurring events (CIA
 * execution, DMA operations, etc.). Secondary slots manage events that
 * occurr occasionally (iterrupts, disk activity etc.). Tertiary slots manage
 * very rare events (inserting a disk, inspecting a component, etc.).
 * Accordingly, we call an event primary, secondary, or tertiary if it is
 * scheduled in a primary, secondary, or tertiary slot, respectively.
 * By default, the event handler only checks the primary event slots. To make
 * the event handler check the secondary slots, too, a special event has to be
 * scheduled in the SEC_SLOT (which is a primary slot and therefore
 * always checked). Triggering this event works like a wakeup call by telling
 * the event handler to check for secondary events as well. Hence, whenever an
 * event is schedules in a secondary slot, it has to be ensured that SEC_SLOT
 * contains a wakeup with a trigger cycle matching the smallest trigger cycle
 * of all secondary events.
 * Scheduling the wakeup event in SEC_SLOT is transparant for the callee. When
 * an event is scheduled, the event handler automatically checks if the
 * selected slot is primary or secondary and schedules the SEC_SLOT
 * automatically in the latter case. The same holds for tertiary events. When
 * such an event is scheduled, the event scheduler automatically schedules a
 * wakup event in the TER_SLOT.
 *
 * To schedule an event, an event slot, a trigger cycle, and an event id
 * need to be provided. The trigger cycle is measured in master cycles. It can
 * be specified in multiple ways:
 *
 *   Absolute (Abs):
 *   The trigger cycle is specified as an absolute value.
 *
 *   Immediate (Imm):
 *   The trigger cycle is the next DMA cycle.
 *
 *   Incremental (Inc):
 *   The trigger cycle is specified relative to the current slot value.
 *
 *   Relative (Rel): (Implemented by Agnus)
 *   The trigger cycle is specified relative to the current DMA clock.
 *
 *   Positional (Pos): (Implemented by Agnus)
 *   The trigger cycle is specified in form of a beam position.
 *
 * Events can also be rescheduled or canceled:
 *
 *   Rescheduling means that the event ID in the selected event slot
 *   remains unchanged.
 *
 *   Canceling means that the slot is emptied by deleting the setting the
 *   event ID and the event data to zero and the trigger cycle to NEVER.
 */

void
Agnus::scheduleFirstBplEvent()
{
    assert(pos.h == 0);
    
    u8 dmacycle = sequencer.bplEvent[0] ? 0 : sequencer.nextBplEvent[0];
    scheduleRel<SLOT_BPL>(DMA_CYCLES(dmacycle), sequencer.bplEvent[dmacycle]);
}

void
Agnus::scheduleNextBplEvent(isize hpos)
{
    assert(hpos >= 0 && hpos < HPOS_CNT);

    u8 next = sequencer.nextBplEvent[hpos];
    scheduleRel<SLOT_BPL>(DMA_CYCLES(next - pos.h), sequencer.bplEvent[next]);
}

void
Agnus::scheduleBplEventForCycle(isize hpos)
{
    assert(hpos >= pos.h && hpos < HPOS_CNT);

    if (sequencer.bplEvent[hpos] != EVENT_NONE) {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(hpos - pos.h), sequencer.bplEvent[hpos]);
    } else {
        scheduleNextBplEvent(hpos);
    }
}

void
Agnus::scheduleFirstDasEvent()
{
    assert(pos.h == 0);
    
    u8 dmacycle = sequencer.nextDasEvent[0];
    assert(dmacycle != 0);
    scheduleRel<SLOT_DAS>(DMA_CYCLES(dmacycle), sequencer.dasEvent[dmacycle]);
}

void
Agnus::scheduleNextDasEvent(isize hpos)
{
    assert(hpos >= 0 && hpos < HPOS_CNT);

    u8 next = sequencer.nextDasEvent[hpos];
    assert(next >= 0 && next < HPOS_CNT);

    scheduleRel<SLOT_DAS>(DMA_CYCLES(next - pos.h), sequencer.dasEvent[next]);
    assert(hasEvent<SLOT_DAS>());
}

void
Agnus::scheduleDasEventForCycle(isize hpos)
{
    assert(hpos >= pos.h && hpos < HPOS_CNT);

    if (sequencer.dasEvent[hpos] != EVENT_NONE) {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(hpos - pos.h), sequencer.dasEvent[hpos]);
    } else {
        scheduleNextDasEvent(hpos);
    }
}

void
Agnus::scheduleNextREGEvent()
{
    if (syncEvent) {
        
        // Schedule an event for the next cycle as there are pending events
        scheduleRel <SLOT_REG> (DMA_CYCLES(1), REG_CHANGE);
        
    } else {
        
        // Determine when the next register change happens
        Cycle next = changeRecorder.trigger();
        
        // Schedule a register change event for that cycle
        scheduleAbs <SLOT_REG> (next, REG_CHANGE);
    }
}

void
Agnus::scheduleStrobe0Event()
{
    schedulePos<SLOT_VBL>(vStrobeLine(), 0, VBL_STROBE0);
}

void
Agnus::scheduleStrobe1Event()
{
    schedulePos<SLOT_VBL>(5, 84, VBL_STROBE1);
}

void
Agnus::scheduleStrobe2Event()
{
    schedulePos<SLOT_VBL>(5, 178, VBL_STROBE2);
}

void
Agnus::serviceREGEvent(Cycle until)
{
    assert(pos.type != TV::PAL || pos.h <= PAL::HPOS_CNT);
    assert(pos.type != TV::NTSC || pos.h <= NTSC::HPOS_CNT);

    if (syncEvent) {

        // Call the EOL handler if requested
        if (syncEvent & EVFL::EOL) { eolHandler(); syncEvent &= ~EVFL::EOL; }

        // Call the HSYNC handler if requested
        if (syncEvent & EVFL::HSYNC) { hsyncHandler(); syncEvent &= ~EVFL::HSYNC; }
    }

    // Iterate through all recorded register changes
    while (!changeRecorder.isEmpty()) {

        // We're done once the trigger cycle exceeds the target cycle
        if (changeRecorder.trigger() > until) break;

        // Apply the register change
        RegChange &change = changeRecorder.read();

        switch (change.reg) {

            case Reg::BLTSIZE: blitter.setBLTSIZE(change.value); break;
            case Reg::BLTSIZV: blitter.setBLTSIZV(change.value); break;
                
            case Reg::BLTCON0: blitter.setBLTCON0(change.value); break;
            case Reg::BLTCON0L: blitter.setBLTCON0L(change.value); break;
            case Reg::BLTCON1: blitter.setBLTCON1(change.value); break;
                
            case Reg::INTREQ: paula.setINTREQ(change.value); break;
            case Reg::INTENA: paula.setINTENA(change.value); break;

            case Reg::BPLCON0:
                assert(change.accessor == Accessor::AGNUS || change.accessor == Accessor::DENISE);
                change.accessor == Accessor::AGNUS ?
                setBPLCON0(bplcon0, change.value) : denise.setBPLCON0(bplcon0, change.value);
                break;
            case Reg::BPLCON1:
                assert(change.accessor == Accessor::AGNUS || change.accessor == Accessor::DENISE);
                change.accessor == Accessor::AGNUS ?
                setBPLCON1(bplcon1, change.value) : denise.setBPLCON1(bplcon1, change.value);
                break;
                break;
            case Reg::BPLCON2: denise.setBPLCON2(change.value); break;
            case Reg::BPLCON3: denise.setBPLCON3(change.value); break;
                
            case Reg::DMACON: setDMACON(dmacon, change.value); break;

            case Reg::DIWSTRT:
                change.accessor == Accessor::AGNUS ?
                sequencer.setDIWSTRT(change.value) : denise.setDIWSTRT(change.value);
                break;
            case Reg::DIWSTOP:
                change.accessor == Accessor::AGNUS ?
                sequencer.setDIWSTOP(change.value) : denise.setDIWSTOP(change.value);
                break;
            case Reg::DIWHIGH:
                change.accessor == Accessor::AGNUS ?
                sequencer.setDIWHIGH(change.value) : denise.setDIWHIGH(change.value);
                break;
            case Reg::DDFSTRT: sequencer.setDDFSTRT(change.value); break;
            case Reg::DDFSTOP: sequencer.setDDFSTOP(change.value); break;
                
            case Reg::BPL1MOD: setBPL1MOD(change.value); break;
            case Reg::BPL2MOD: setBPL2MOD(change.value); break;
                
            case Reg::BPL1PTH: setBPLxPTH<1>(change.value); break;
            case Reg::BPL2PTH: setBPLxPTH<2>(change.value); break;
            case Reg::BPL3PTH: setBPLxPTH<3>(change.value); break;
            case Reg::BPL4PTH: setBPLxPTH<4>(change.value); break;
            case Reg::BPL5PTH: setBPLxPTH<5>(change.value); break;
            case Reg::BPL6PTH: setBPLxPTH<6>(change.value); break;

            case Reg::BPL1PTL: setBPLxPTL<1>(change.value); break;
            case Reg::BPL2PTL: setBPLxPTL<2>(change.value); break;
            case Reg::BPL3PTL: setBPLxPTL<3>(change.value); break;
            case Reg::BPL4PTL: setBPLxPTL<4>(change.value); break;
            case Reg::BPL5PTL: setBPLxPTL<5>(change.value); break;
            case Reg::BPL6PTL: setBPLxPTL<6>(change.value); break;

            case Reg::SPR0POS: setSPRxPOS<0>(change.value); break;
            case Reg::SPR1POS: setSPRxPOS<1>(change.value); break;
            case Reg::SPR2POS: setSPRxPOS<2>(change.value); break;
            case Reg::SPR3POS: setSPRxPOS<3>(change.value); break;
            case Reg::SPR4POS: setSPRxPOS<4>(change.value); break;
            case Reg::SPR5POS: setSPRxPOS<5>(change.value); break;
            case Reg::SPR6POS: setSPRxPOS<6>(change.value); break;
            case Reg::SPR7POS: setSPRxPOS<7>(change.value); break;

            case Reg::SPR0CTL: setSPRxCTL<0>(change.value); break;
            case Reg::SPR1CTL: setSPRxCTL<1>(change.value); break;
            case Reg::SPR2CTL: setSPRxCTL<2>(change.value); break;
            case Reg::SPR3CTL: setSPRxCTL<3>(change.value); break;
            case Reg::SPR4CTL: setSPRxCTL<4>(change.value); break;
            case Reg::SPR5CTL: setSPRxCTL<5>(change.value); break;
            case Reg::SPR6CTL: setSPRxCTL<6>(change.value); break;
            case Reg::SPR7CTL: setSPRxCTL<7>(change.value); break;

            case Reg::SPR0PTH: setSPRxPTH<0>(change.value); break;
            case Reg::SPR1PTH: setSPRxPTH<1>(change.value); break;
            case Reg::SPR2PTH: setSPRxPTH<2>(change.value); break;
            case Reg::SPR3PTH: setSPRxPTH<3>(change.value); break;
            case Reg::SPR4PTH: setSPRxPTH<4>(change.value); break;
            case Reg::SPR5PTH: setSPRxPTH<5>(change.value); break;
            case Reg::SPR6PTH: setSPRxPTH<6>(change.value); break;
            case Reg::SPR7PTH: setSPRxPTH<7>(change.value); break;

            case Reg::SPR0PTL: setSPRxPTL<0>(change.value); break;
            case Reg::SPR1PTL: setSPRxPTL<1>(change.value); break;
            case Reg::SPR2PTL: setSPRxPTL<2>(change.value); break;
            case Reg::SPR3PTL: setSPRxPTL<3>(change.value); break;
            case Reg::SPR4PTL: setSPRxPTL<4>(change.value); break;
            case Reg::SPR5PTL: setSPRxPTL<5>(change.value); break;
            case Reg::SPR6PTL: setSPRxPTL<6>(change.value); break;
            case Reg::SPR7PTL: setSPRxPTL<7>(change.value); break;

            case Reg::DSKPTH: setDSKPTH(change.value); break;
            case Reg::DSKPTL: setDSKPTL(change.value); break;

            case Reg::SERDAT: uart.setSERDAT(change.value); break;

            default:
                fatalError;
        }
    }

    // Let the logic analyzer probe all observed signals
    if (syncEvent & EVFL::PROBE) { logicAnalyzer.recordSignals(); }

    scheduleNextREGEvent();
}

#define LO_NONE(x)      { serviceBPLEventLores<x>(); }
#define LO_ODD(x)       { denise.drawLoresOdd();  LO_NONE(x) }
#define LO_EVEN(x)      { denise.drawLoresEven(); LO_NONE(x) }
#define LO_BOTH(x)      { denise.drawLoresBoth(); LO_NONE(x) }

#define LO_MOD(x)       { LO_NONE(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define LO_MOD_ODD(x)   { LO_ODD(x);  U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define LO_MOD_EVEN(x)  { LO_EVEN(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define LO_MOD_BOTH(x)  { LO_BOTH(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }

#define HI_NONE(x)      { serviceBPLEventHires<x>(); }
#define HI_ODD(x)       { denise.drawHiresOdd();  HI_NONE(x) }
#define HI_EVEN(x)      { denise.drawHiresEven(); HI_NONE(x) }
#define HI_BOTH(x)      { denise.drawHiresBoth(); HI_NONE(x) }

#define HI_MOD(x)       { HI_NONE(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define HI_MOD_ODD(x)   { HI_ODD(x);  U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define HI_MOD_EVEN(x)  { HI_EVEN(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define HI_MOD_BOTH(x)  { HI_BOTH(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }

#define SH_NONE(x)      { serviceBPLEventShres<x>(); }
#define SH_ODD(x)       { denise.drawShresOdd();  SH_NONE(x) }
#define SH_EVEN(x)      { denise.drawShresEven(); SH_NONE(x) }
#define SH_BOTH(x)      { denise.drawShresBoth(); SH_NONE(x) }

#define SH_MOD(x)       { SH_NONE(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define SH_MOD_ODD(x)   { SH_ODD(x);  U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define SH_MOD_EVEN(x)  { SH_EVEN(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }
#define SH_MOD_BOTH(x)  { SH_BOTH(x); U32_INC(bplpt[x], (x & 1) ? bpl2mod : bpl1mod); }

void
Agnus::serviceBPLEvent(EventID id)
{
    switch (isize(id)) {

        case EVENT_NONE:
            assert(pos.h == HPOS_MAX);
            break;

        case EVENT_NONE | DRAW_ODD:

            switch (res) {
                case Resolution::LORES: denise.drawLoresOdd(); break;
                case Resolution::HIRES: denise.drawHiresOdd(); break;
                case Resolution::SHRES: denise.drawShresOdd(); break;
            }
            break;

        case EVENT_NONE | DRAW_EVEN:
            switch (res) {
                case Resolution::LORES: denise.drawLoresEven(); break;
                case Resolution::HIRES: denise.drawHiresEven(); break;
                case Resolution::SHRES: denise.drawShresEven(); break;
            }
            break;

        case EVENT_NONE | DRAW_BOTH:
            switch (res) {
                case Resolution::LORES: denise.drawLoresBoth(); break;
                case Resolution::HIRES: denise.drawHiresBoth(); break;
                case Resolution::SHRES: denise.drawShresBoth(); break;
            }
            break;

        case BPL_L1:                    LO_NONE(0);     break;
        case BPL_L1     | DRAW_ODD:     LO_ODD(0);      break;
        case BPL_L1     | DRAW_EVEN:    LO_EVEN(0);     break;
        case BPL_L1     | DRAW_BOTH:    LO_BOTH(0);     break;
        case BPL_L1_MOD:                LO_MOD(0);      break;
        case BPL_L1_MOD | DRAW_ODD:     LO_MOD_ODD(0);  break;
        case BPL_L1_MOD | DRAW_EVEN:    LO_MOD_EVEN(0); break;
        case BPL_L1_MOD | DRAW_BOTH:    LO_MOD_BOTH(0); break;

        case BPL_L2:                    LO_NONE(1);     break;
        case BPL_L2     | DRAW_ODD:     LO_ODD(1);      break;
        case BPL_L2     | DRAW_EVEN:    LO_EVEN(1);     break;
        case BPL_L2     | DRAW_BOTH:    LO_BOTH(1);     break;
        case BPL_L2_MOD:                LO_MOD(1);      break;
        case BPL_L2_MOD | DRAW_ODD:     LO_MOD_ODD(1);  break;
        case BPL_L2_MOD | DRAW_EVEN:    LO_MOD_EVEN(1); break;
        case BPL_L2_MOD | DRAW_BOTH:    LO_MOD_BOTH(1); break;

        case BPL_L3:                    LO_NONE(2);     break;
        case BPL_L3     | DRAW_ODD:     LO_ODD(2);      break;
        case BPL_L3     | DRAW_EVEN:    LO_EVEN(2);     break;
        case BPL_L3     | DRAW_BOTH:    LO_BOTH(2);     break;
        case BPL_L3_MOD:                LO_MOD(2);      break;
        case BPL_L3_MOD | DRAW_ODD:     LO_MOD_ODD(2);  break;
        case BPL_L3_MOD | DRAW_EVEN:    LO_MOD_EVEN(2); break;
        case BPL_L3_MOD | DRAW_BOTH:    LO_MOD_BOTH(2); break;

        case BPL_L4:                    LO_NONE(3);     break;
        case BPL_L4     | DRAW_ODD:     LO_ODD(3);      break;
        case BPL_L4     | DRAW_EVEN:    LO_EVEN(3);     break;
        case BPL_L4     | DRAW_BOTH:    LO_BOTH(3);     break;
        case BPL_L4_MOD:                LO_MOD(3);      break;
        case BPL_L4_MOD | DRAW_ODD:     LO_MOD_ODD(3);  break;
        case BPL_L4_MOD | DRAW_EVEN:    LO_MOD_EVEN(3); break;
        case BPL_L4_MOD | DRAW_BOTH:    LO_MOD_BOTH(3); break;

        case BPL_L5:                    LO_NONE(4);     break;
        case BPL_L5     | DRAW_ODD:     LO_ODD(4);      break;
        case BPL_L5     | DRAW_EVEN:    LO_EVEN(4);     break;
        case BPL_L5     | DRAW_BOTH:    LO_BOTH(4);     break;
        case BPL_L5_MOD:                LO_MOD(4);      break;
        case BPL_L5_MOD | DRAW_ODD:     LO_MOD_ODD(4);  break;
        case BPL_L5_MOD | DRAW_EVEN:    LO_MOD_EVEN(4); break;
        case BPL_L5_MOD | DRAW_BOTH:    LO_MOD_BOTH(4); break;

        case BPL_L6:                    LO_NONE(5);     break;
        case BPL_L6     | DRAW_ODD:     LO_ODD(5);      break;
        case BPL_L6     | DRAW_EVEN:    LO_EVEN(5);     break;
        case BPL_L6     | DRAW_BOTH:    LO_BOTH(5);     break;
        case BPL_L6_MOD:                LO_MOD(5);      break;
        case BPL_L6_MOD | DRAW_ODD:     LO_MOD_ODD(5);  break;
        case BPL_L6_MOD | DRAW_EVEN:    LO_MOD_EVEN(5); break;
        case BPL_L6_MOD | DRAW_BOTH:    LO_MOD_BOTH(5); break;

        case BPL_H1:                    HI_NONE(0);     break;
        case BPL_H1     | DRAW_ODD:     HI_ODD(0);      break;
        case BPL_H1     | DRAW_EVEN:    HI_EVEN(0);     break;
        case BPL_H1     | DRAW_BOTH:    HI_BOTH(0);     break;
        case BPL_H1_MOD:                HI_MOD(0);      break;
        case BPL_H1_MOD | DRAW_ODD:     HI_MOD_ODD(0);  break;
        case BPL_H1_MOD | DRAW_EVEN:    HI_MOD_EVEN(0); break;
        case BPL_H1_MOD | DRAW_BOTH:    HI_MOD_BOTH(0); break;

        case BPL_H2:                    HI_NONE(1);     break;
        case BPL_H2     | DRAW_ODD:     HI_ODD(1);      break;
        case BPL_H2     | DRAW_EVEN:    HI_EVEN(1);     break;
        case BPL_H2     | DRAW_BOTH:    HI_BOTH(1);     break;
        case BPL_H2_MOD:                HI_MOD(1);      break;
        case BPL_H2_MOD | DRAW_ODD:     HI_MOD_ODD(1);  break;
        case BPL_H2_MOD | DRAW_EVEN:    HI_MOD_EVEN(1); break;
        case BPL_H2_MOD | DRAW_BOTH:    HI_MOD_BOTH(1); break;

        case BPL_H3:                    HI_NONE(2);     break;
        case BPL_H3     | DRAW_ODD:     HI_ODD(2);      break;
        case BPL_H3     | DRAW_EVEN:    HI_EVEN(2);     break;
        case BPL_H3     | DRAW_BOTH:    HI_BOTH(2);     break;
        case BPL_H3_MOD:                HI_MOD(2);      break;
        case BPL_H3_MOD | DRAW_ODD:     HI_MOD_ODD(2);  break;
        case BPL_H3_MOD | DRAW_EVEN:    HI_MOD_EVEN(2); break;
        case BPL_H3_MOD | DRAW_BOTH:    HI_MOD_BOTH(2); break;

        case BPL_H4:                    HI_NONE(3);     break;
        case BPL_H4     | DRAW_ODD:     HI_ODD(3);      break;
        case BPL_H4     | DRAW_EVEN:    HI_EVEN(3);     break;
        case BPL_H4     | DRAW_BOTH:    HI_BOTH(3);     break;
        case BPL_H4_MOD:                HI_MOD(3);      break;
        case BPL_H4_MOD | DRAW_ODD:     HI_MOD_ODD(3);  break;
        case BPL_H4_MOD | DRAW_EVEN:    HI_MOD_EVEN(3); break;
        case BPL_H4_MOD | DRAW_BOTH:    HI_MOD_BOTH(3); break;

        case BPL_S1:                    SH_NONE(0);     break;
        case BPL_S1     | DRAW_ODD:     SH_ODD(0);      break;
        case BPL_S1     | DRAW_EVEN:    SH_EVEN(0);     break;
        case BPL_S1     | DRAW_BOTH:    SH_BOTH(0);     break;
        case BPL_S1_MOD:                SH_MOD(0);      break;
        case BPL_S1_MOD | DRAW_ODD:     SH_MOD_ODD(0);  break;
        case BPL_S1_MOD | DRAW_EVEN:    SH_MOD_EVEN(0); break;
        case BPL_S1_MOD | DRAW_BOTH:    SH_MOD_BOTH(0); break;

        case BPL_S2:                    SH_NONE(1);     break;
        case BPL_S2     | DRAW_ODD:     SH_ODD(1);      break;
        case BPL_S2     | DRAW_EVEN:    SH_EVEN(1);     break;
        case BPL_S2     | DRAW_BOTH:    SH_BOTH(1);     break;
        case BPL_S2_MOD:                SH_MOD(1);      break;
        case BPL_S2_MOD | DRAW_ODD:     SH_MOD_ODD(1);  break;
        case BPL_S2_MOD | DRAW_EVEN:    SH_MOD_EVEN(1); break;
        case BPL_S2_MOD | DRAW_BOTH:    SH_MOD_BOTH(1); break;

        default:
            dump(Category::Dma);
            fatalError;
    }

    // Schedule next event
    scheduleNextBplEvent();
}

template <isize nr> void
Agnus::serviceBPLEventLores()
{
    // Perform bitplane DMA
    denise.setBPLxDAT<nr>(doBitplaneDmaRead<nr>());
}

template <isize nr> void
Agnus::serviceBPLEventHires()
{
    // Perform bitplane DMA
    denise.setBPLxDAT<nr>(doBitplaneDmaRead<nr>());
}

template <isize nr> void
Agnus::serviceBPLEventShres()
{
    // Perform bitplane DMA
    denise.setBPLxDAT<nr>(doBitplaneDmaRead<nr>());
}

void
Agnus::serviceVBLEvent(EventID id)
{
    switch (id) {

        case VBL_STROBE0:

            assert(!isPAL() || (pos.v == 0 || pos.v == 1));
            assert(!isPAL() || pos.h == 0);
            
            // Trigger the vertical blank interrupt
            paula.raiseIrq(IrqSource::VERTB);
            
            // Schedule next event
            scheduleStrobe1Event();
            break;

        case VBL_STROBE1:

            assert(!isPAL() || pos.v == 5);
            assert(!isPAL() || pos.h == 84);

            // Increment the TOD counter of CIA A
            ciaa.tod.increment();
            
            // Schedule next event
            scheduleStrobe2Event();
            break;
            
        case VBL_STROBE2:

            assert(!isPAL() || pos.v == 5);
            assert(!isPAL() || pos.h == 178);

            // Nothing is done here at the moment
            cancel<SLOT_VBL>();
            break;
            
        default:
            fatalError;
    }
}

void
Agnus::rectifyVBLEvent()
{
    switch (id[SLOT_VBL]) {

        case VBL_STROBE0: scheduleStrobe0Event(); break;
        case VBL_STROBE1: scheduleStrobe1Event(); break;
        case VBL_STROBE2: scheduleStrobe2Event(); break;

        default: break;
    }
}

void
Agnus::serviceDASEvent(EventID id)
{
    assert(id == sequencer.dasEvent[pos.h]);

    switch (id) {

        case DAS_REFRESH:

            busOwner[0x01] = BusOwner::REFRESH;
            busOwner[0x03] = BusOwner::REFRESH;
            busOwner[0x05] = BusOwner::REFRESH;
            busOwner[pos.lol ? 0xE3 : 0xE2] = BusOwner::REFRESH;

            busAddr[0x01] = 0;
            busAddr[0x03] = 0;
            busAddr[0x05] = 0;
            busAddr[pos.lol ? 0xE3 : 0xE2] = 0;
            
            busData[0x01] = 0;
            busData[0x03] = 0;
            busData[0x05] = 0;
            busData[pos.lol ? 0xE3 : 0xE2] = 0;

            stats.usage[BUS_REFRESH] += 4;
            break;

        case DAS_D0:
        case DAS_D1:
        case DAS_D2:

            paula.diskController.performDMA();
            break;

        case DAS_A0:

            if (audxDR[0]) {
                audxDR[0] = false;
                paula.channel0.pokeAUDxDAT(doAudioDmaRead<0>());
            }
            break;

        case DAS_A1:

            if (audxDR[1]) {
                audxDR[1] = false;
                paula.channel1.pokeAUDxDAT(doAudioDmaRead<1>());
            }
            break;

        case DAS_A2:

            /* Ask the REG slot handler to call the HSYNC handler at the
             * beginning of the next cycle. We utilize the REG handler,
             * because it is the first one to execute. Hence, we can assure
             * that the the HSYNC handler is executed before any other
             * operation is performed in this cycle.
             */
            syncEvent |= EVFL::HSYNC;
            scheduleRel <SLOT_REG> (DMA_CYCLES(1), REG_CHANGE);

            if (audxDR[2]) {
                audxDR[2] = false;
                paula.channel2.pokeAUDxDAT(doAudioDmaRead<2>());
            }
            break;

        case DAS_A3:

            if (audxDR[3]) {
                audxDR[3] = false;
                paula.channel3.pokeAUDxDAT(doAudioDmaRead<3>());
            }
            break;

        case DAS_S0_1:

            executeFirstSpriteCycle<0>();
            break;

        case DAS_S0_2:

            executeSecondSpriteCycle<0>();
            break;

        case DAS_S1_1:

            executeFirstSpriteCycle<1>();
            break;

        case DAS_S1_2:

            executeSecondSpriteCycle<1>();
            break;

        case DAS_S2_1:

            executeFirstSpriteCycle<2>();
            break;

        case DAS_S2_2:

            executeSecondSpriteCycle<2>();
            break;

        case DAS_S3_1:

            executeFirstSpriteCycle<3>();
            break;

        case DAS_S3_2:

            executeSecondSpriteCycle<3>();
            break;

        case DAS_S4_1:

            executeFirstSpriteCycle<4>();
            break;

        case DAS_S4_2:

            executeSecondSpriteCycle<4>();
            break;

        case DAS_S5_1:

            executeFirstSpriteCycle<5>();
            break;

        case DAS_S5_2:

            executeSecondSpriteCycle<5>();
            break;

        case DAS_S6_1:

            executeFirstSpriteCycle<6>();
            break;

        case DAS_S6_2:

            executeSecondSpriteCycle<6>();
            break;

        case DAS_S7_1:

            executeFirstSpriteCycle<7>();
            break;

        case DAS_S7_2:

            executeSecondSpriteCycle<7>();
            break;

        case DAS_SDMA:

            updateSpriteDMA();
            break;

        case DAS_TICK:
            
            ciab.tod.increment();
            break;

        case DAS_EOL:

            assert(pos.h == PAL::HPOS_MAX || pos.h == NTSC::HPOS_MAX);

            if (pos.h == PAL::HPOS_MAX && pos.lol) {

                // Run for an additional cycle (long line)
                break;

            } else {

                /* Ask the REG slot handler to call the EOL handler at the
                 * beginning of the next cycle. We utilize the REG handler,
                 * because it is the first one to execute. Hence, we can assure
                 * that the the EOL handler is executed before any other
                 * operation is performed in this cycle.
                 */
                syncEvent |= EVFL::EOL;
                scheduleRel <SLOT_REG> (DMA_CYCLES(1), REG_CHANGE);
            }
            break;

        default:
            fatalError;
    }

    // Schedule next event
    scheduleNextDasEvent();
}

void
Agnus::serviceINSEvent()
{
    u64 mask = data[SLOT_INS];

    // Analyze bit mask
    if (mask & 1LL << long(Class::Agnus))           { agnus.record(); }
    if (mask & 1LL << long(Class::Amiga))           { amiga.record(); }
    if (mask & 1LL << long(Class::Blitter))         { blitter.record(); }
    if (mask & 1LL << long(Class::Copper))          { copper.record(); }
    if (mask & 1LL << long(Class::CIA))             { ciaa.record(); ciab.record(); }
    if (mask & 1LL << long(Class::CPU))             { cpu.record(); }
    if (mask & 1LL << long(Class::Denise))          { denise.record(); }
    if (mask & 1LL << long(Class::Memory))          { mem.record(); }
    if (mask & 1LL << long(Class::Paula))           { paula.record(); }
    if (mask & 1LL << long(Class::UART))            { uart.record(); }
    if (mask & 1LL << long(Class::ControlPort))     { controlPort1.record(); controlPort2.record(); }
    if (mask & 1LL << long(Class::SerialPort))      { serialPort.record(); }

    // Reschedule the event
    rescheduleRel<SLOT_INS>((Cycle)(inspectionInterval * 28000007));
}

}
