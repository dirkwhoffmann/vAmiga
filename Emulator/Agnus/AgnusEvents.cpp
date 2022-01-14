// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "Amiga.h"
#include "CIA.h"
#include "ControlPort.h"
#include "CPU.h"
#include "Denise.h"
#include "Paula.h"
#include "SerialPort.h"

void
Agnus::scheduleFirstBplEvent()
{
    assert(pos.h == 0 || pos.h == HPOS_MAX);
    
    u8 dmacycle = sequencer.nextBplEvent[0];
    assert(dmacycle != 0);
        
    if (pos.h == 0) {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(dmacycle), sequencer.bplEvent[dmacycle]);
    } else {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(dmacycle + 1), sequencer.bplEvent[dmacycle]);
    }
}

void
Agnus::scheduleNextBplEvent(isize hpos)
{
    assert(hpos >= 0 && hpos < HPOS_CNT);

    if (u8 next = sequencer.nextBplEvent[hpos]) {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(next - pos.h), sequencer.bplEvent[next]);
    }
    assert(scheduler.hasEvent<SLOT_BPL>());
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

    assert(scheduler.hasEvent<SLOT_BPL>());
}

void
Agnus::scheduleFirstDasEvent()
{
    assert(pos.h == 0 || pos.h == HPOS_MAX);
    
    u8 dmacycle = sequencer.nextDasEvent[0];
    assert(dmacycle != 0);
    
    if (pos.h == 0) {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(dmacycle), sequencer.dasEvent[dmacycle]);
    } else {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(dmacycle + 1), sequencer.dasEvent[dmacycle]);
    }
}

void
Agnus::scheduleNextDasEvent(isize hpos)
{
    assert(hpos >= 0 && hpos < HPOS_CNT);

    if (u8 next = sequencer.nextDasEvent[hpos]) {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(next - pos.h), sequencer.dasEvent[next]);
        assert(scheduler.hasEvent<SLOT_DAS>());
    } else {
        scheduler.cancel<SLOT_DAS>();
    }
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
    // Determine when the next register change happens
    Cycle nextTrigger = changeRecorder.trigger();

    // Schedule a register change event for that cycle
    scheduler.scheduleAbs<SLOT_REG>(nextTrigger, REG_CHANGE);
}

void
Agnus::scheduleStrobe0Event()
{
    schedulePos<SLOT_VBL>(frame.numLines() + vStrobeLine(), 0, VBL_STROBE0);
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
    assert(pos.h <= HPOS_MAX);

    // Iterate through all recorded register changes
    while (!changeRecorder.isEmpty()) {

        // We're done once the trigger cycle exceeds the target cycle
        if (changeRecorder.trigger() > until) return;

        // Apply the register change
        RegChange &change = changeRecorder.read();
        
        switch (change.addr) {
                
            case SET_BLTSIZE: blitter.setBLTSIZE(change.value); break;
            case SET_BLTSIZV: blitter.setBLTSIZV(change.value); break;
                
            case SET_BLTCON0: blitter.setBLTCON0(change.value); break;
            case SET_BLTCON0L: blitter.setBLTCON0L(change.value); break;
            case SET_BLTCON1: blitter.setBLTCON1(change.value); break;
                
            case SET_INTREQ: paula.setINTREQ(change.value); break;
            case SET_INTENA: paula.setINTENA(change.value); break;
                
            case SET_BPLCON0_AGNUS: setBPLCON0(bplcon0, change.value); break;
            case SET_BPLCON0_DENISE: denise.setBPLCON0(bplcon0, change.value); break;
            case SET_BPLCON1_AGNUS: setBPLCON1(bplcon1, change.value); break;
            case SET_BPLCON1_DENISE: denise.setBPLCON1(bplcon1, change.value); break;
            case SET_BPLCON2: denise.setBPLCON2(change.value); break;
            case SET_BPLCON3: denise.setBPLCON3(change.value); break;
                
            case SET_DMACON: setDMACON(dmacon, change.value); break;
                
            case SET_DIWSTRT_AGNUS: sequencer.setDIWSTRT(change.value); break;
            case SET_DIWSTRT_DENISE: denise.setDIWSTRT(change.value); break;
            case SET_DIWSTOP_AGNUS: sequencer.setDIWSTOP(change.value); break;
            case SET_DIWSTOP_DENISE: denise.setDIWSTOP(change.value); break;

            case SET_DDFSTRT: sequencer.setDDFSTRT(change.value); break;
            case SET_DDFSTOP: sequencer.setDDFSTOP(change.value); break;
                
            case SET_BPL1MOD: setBPL1MOD(change.value); break;
            case SET_BPL2MOD: setBPL2MOD(change.value); break;
                
            case SET_BPL1PTH: setBPLxPTH<1>(change.value); break;
            case SET_BPL2PTH: setBPLxPTH<2>(change.value); break;
            case SET_BPL3PTH: setBPLxPTH<3>(change.value); break;
            case SET_BPL4PTH: setBPLxPTH<4>(change.value); break;
            case SET_BPL5PTH: setBPLxPTH<5>(change.value); break;
            case SET_BPL6PTH: setBPLxPTH<6>(change.value); break;
                                
            case SET_BPL1PTL: setBPLxPTL<1>(change.value); break;
            case SET_BPL2PTL: setBPLxPTL<2>(change.value); break;
            case SET_BPL3PTL: setBPLxPTL<3>(change.value); break;
            case SET_BPL4PTL: setBPLxPTL<4>(change.value); break;
            case SET_BPL5PTL: setBPLxPTL<5>(change.value); break;
            case SET_BPL6PTL: setBPLxPTL<6>(change.value); break;
                                
            case SET_SPR0PTH: setSPRxPTH<0>(change.value); break;
            case SET_SPR1PTH: setSPRxPTH<1>(change.value); break;
            case SET_SPR2PTH: setSPRxPTH<2>(change.value); break;
            case SET_SPR3PTH: setSPRxPTH<3>(change.value); break;
            case SET_SPR4PTH: setSPRxPTH<4>(change.value); break;
            case SET_SPR5PTH: setSPRxPTH<5>(change.value); break;
            case SET_SPR6PTH: setSPRxPTH<6>(change.value); break;
            case SET_SPR7PTH: setSPRxPTH<7>(change.value); break;
                                
            case SET_SPR0PTL: setSPRxPTL<0>(change.value); break;
            case SET_SPR1PTL: setSPRxPTL<1>(change.value); break;
            case SET_SPR2PTL: setSPRxPTL<2>(change.value); break;
            case SET_SPR3PTL: setSPRxPTL<3>(change.value); break;
            case SET_SPR4PTL: setSPRxPTL<4>(change.value); break;
            case SET_SPR5PTL: setSPRxPTL<5>(change.value); break;
            case SET_SPR6PTL: setSPRxPTL<6>(change.value); break;
            case SET_SPR7PTL: setSPRxPTL<7>(change.value); break;
                                
            case SET_DSKPTH: setDSKPTH(change.value); break;
            case SET_DSKPTL: setDSKPTL(change.value); break;
                
            case SET_STRHOR: hsyncHandler(); break;
                
            default:
                fatalError;
        }
    }
    
    // Schedule next event
    scheduleNextREGEvent();
}

void
Agnus::serviceRASEvent()
{
    assert(scheduler.id[SLOT_RAS] == RAS_HSYNC);
    
    // Let the hsync handler be called at the beginning of the next DMA cycle
    agnus.recordRegisterChange(0, SET_STRHOR, 1);
    
    // Reset the horizontal counter (-1 to compensate for the increment to come)
    pos.h = -1;
    
    // Reschedule event
    rescheduleRel<SLOT_RAS>(DMA_CYCLES(HPOS_CNT));
}

#define LO_NONE(x)      { serviceBPLEventLores<x>(); }
#define LO_ODD(x)       { denise.drawLoresOdd();  LO_NONE(x) }
#define LO_EVEN(x)      { denise.drawLoresEven(); LO_NONE(x) }
#define LO_BOTH(x)      { denise.drawLoresBoth(); LO_NONE(x) }

#define LO_MOD(x)       { LO_NONE(x); bplpt[x] += (x & 1) ? bpl2mod : bpl1mod; }
#define LO_MOD_ODD(x)   { LO_ODD(x);  bplpt[x] += (x & 1) ? bpl2mod : bpl1mod; }
#define LO_MOD_EVEN(x)  { LO_EVEN(x); bplpt[x] += (x & 1) ? bpl2mod : bpl1mod; }
#define LO_MOD_BOTH(x)  { LO_BOTH(x); bplpt[x] += (x & 1) ? bpl2mod : bpl1mod; }

#define HI_NONE(x)      { serviceBPLEventHires<x>(); }
#define HI_ODD(x)       { denise.drawHiresOdd();  HI_NONE(x) }
#define HI_EVEN(x)      { denise.drawHiresEven(); HI_NONE(x) }
#define HI_BOTH(x)      { denise.drawHiresBoth(); HI_NONE(x) }

#define HI_MOD(x)       { HI_NONE(x); bplpt[x] += (x & 1) ? bpl2mod : bpl1mod; }
#define HI_MOD_ODD(x)   { HI_ODD(x);  bplpt[x] += (x & 1) ? bpl2mod : bpl1mod; }
#define HI_MOD_EVEN(x)  { HI_EVEN(x); bplpt[x] += (x & 1) ? bpl2mod : bpl1mod; }
#define HI_MOD_BOTH(x)  { HI_BOTH(x); bplpt[x] += (x & 1) ? bpl2mod : bpl1mod; }

void
Agnus::serviceBPLEvent(EventID id)
{
    switch (id) {

        case EVENT_NONE | DRAW_ODD:
            hires() ? denise.drawHiresOdd() : denise.drawLoresOdd();
            break;

        case EVENT_NONE | DRAW_EVEN:
            hires() ? denise.drawHiresEven() : denise.drawLoresEven();
            break;

        case EVENT_NONE | DRAW_BOTH:
            hires() ? denise.drawHiresBoth() : denise.drawLoresBoth();
            break;
            
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

        /*
        case BPL_H1:
            serviceBPLEventHires<0>();
            break;

        case BPL_H1 | DRAW_ODD:
            denise.drawHiresOdd();
            serviceBPLEventHires<0>();
            break;

        case BPL_H1 | DRAW_EVEN:
            denise.drawHiresEven();
            serviceBPLEventHires<0>();
            break;

        case BPL_H1 | DRAW_ODD | DRAW_EVEN:
            denise.drawHiresBoth();
            serviceBPLEventHires<0>();
            break;

        case BPL_L1:
            serviceBPLEventLores<0>();
            break;
            
        case BPL_L1 | DRAW_ODD:
            denise.drawLoresOdd();
            serviceBPLEventLores<0>();
            break;

        case BPL_L1 | DRAW_EVEN:
            denise.drawLoresEven();
            serviceBPLEventLores<0>();
            break;

        case BPL_L1 | DRAW_ODD | DRAW_EVEN:
            denise.drawLoresBoth();
            serviceBPLEventLores<0>();
            break;

        case BPL_H2:
            serviceBPLEventHires<1>();
            break;

        case BPL_H2 | DRAW_ODD:
            denise.drawHiresOdd();
            serviceBPLEventHires<1>();
            break;
            
        case BPL_H2 | DRAW_EVEN:
            denise.drawHiresEven();
            serviceBPLEventHires<1>();
            break;
            
        case BPL_H2 | DRAW_ODD | DRAW_EVEN:
            denise.drawHiresBoth();
            serviceBPLEventHires<1>();
            break;

        case BPL_L2:
            serviceBPLEventLores<1>();
            break;

        case BPL_L2 | DRAW_ODD:
            denise.drawLoresOdd();
            serviceBPLEventLores<1>();
            break;
            
        case BPL_L2 | DRAW_EVEN:
            denise.drawLoresEven();
            serviceBPLEventLores<1>();
            break;
            
        case BPL_L2 | DRAW_ODD | DRAW_EVEN:
            denise.drawLoresBoth();
            serviceBPLEventLores<1>();
            break;

        case BPL_H3:
            serviceBPLEventHires<2>();
            break;

        case BPL_H3 | DRAW_ODD:
            denise.drawHiresOdd();
            serviceBPLEventHires<2>();
            break;

        case BPL_H3 | DRAW_EVEN:
            denise.drawHiresEven();
            serviceBPLEventHires<2>();
            break;

        case BPL_H3 | DRAW_ODD | DRAW_EVEN:
            denise.drawHiresBoth();
            serviceBPLEventHires<2>();
            break;

        case BPL_L3:
            serviceBPLEventLores<2>();
            break;

        case BPL_L3 | DRAW_ODD:
            denise.drawLoresOdd();
            serviceBPLEventLores<2>();
            break;

        case BPL_L3 | DRAW_EVEN:
            denise.drawLoresEven();
            serviceBPLEventLores<2>();
            break;

        case BPL_L3 | DRAW_ODD | DRAW_EVEN:
            denise.drawLoresBoth();
            serviceBPLEventLores<2>();
            break;

        case BPL_H4:
            serviceBPLEventHires<3>();
            break;

        case BPL_H4 | DRAW_ODD:
            denise.drawHiresOdd();
            serviceBPLEventHires<3>();
            break;

        case BPL_H4 | DRAW_EVEN:
            denise.drawHiresEven();
            serviceBPLEventHires<3>();
            break;

        case BPL_H4 | DRAW_ODD | DRAW_EVEN:
            denise.drawHiresBoth();
            serviceBPLEventHires<3>();
            break;

        case BPL_L4:
            serviceBPLEventLores<3>();
            break;

        case BPL_L4 | DRAW_ODD:
            denise.drawLoresOdd();
            serviceBPLEventLores<3>();
            break;

        case BPL_L4 | DRAW_EVEN:
            denise.drawLoresEven();
            serviceBPLEventLores<3>();
            break;

        case BPL_L4 | DRAW_ODD | DRAW_EVEN:
            denise.drawLoresBoth();
            serviceBPLEventLores<3>();
            break;

        case BPL_L5:
            serviceBPLEventLores<4>();
            break;

        case BPL_L5 | DRAW_ODD:
            denise.drawLoresOdd();
            serviceBPLEventLores<4>();
            break;

        case BPL_L5 | DRAW_EVEN:
            denise.drawLoresEven();
            serviceBPLEventLores<4>();
            break;

        case BPL_L5 | DRAW_ODD | DRAW_EVEN:
            denise.drawLoresBoth();
            serviceBPLEventLores<4>();
            break;
            
        case BPL_L6:
            serviceBPLEventLores<5>();
            break;

        case BPL_L6 | DRAW_ODD:
            denise.drawLoresOdd();
            serviceBPLEventLores<5>();
            break;

        case BPL_L6 | DRAW_EVEN:
            denise.drawLoresEven();
            serviceBPLEventLores<5>();
            break;

        case BPL_L6 | DRAW_ODD | DRAW_EVEN:
            denise.drawLoresBoth();
            serviceBPLEventLores<5>();
            break;
        */
            
        case BPL_EOL:
            assert(pos.h == 0xE2);
            return;

        case BPL_EOL | DRAW_ODD:
            assert(pos.h == 0xE2);
            hires() ? denise.drawHiresOdd() : denise.drawLoresOdd();
            return;

        case BPL_EOL | DRAW_EVEN:
            assert(pos.h == 0xE2);
            hires() ? denise.drawHiresEven() : denise.drawLoresEven();
            return;

        case BPL_EOL | DRAW_BOTH:
            assert(pos.h == 0xE2);
            hires() ? denise.drawHiresBoth() : denise.drawLoresBoth();
            return;
            
        default:
            dump(dump::Dma);
            fatalError;
    }

    // Schedule next event
    scheduleNextBplEvent();
}

template <isize nr> void
Agnus::serviceBPLEventHires()
{
    // Perform bitplane DMA
    denise.setBPLxDAT<nr>(doBitplaneDmaRead<nr>());
}

template <isize nr> void
Agnus::serviceBPLEventLores()
{
    // Perform bitplane DMA
    denise.setBPLxDAT<nr>(doBitplaneDmaRead<nr>());
}

void
Agnus::serviceVblEvent(EventID id)
{
    switch (id) {

        case VBL_STROBE0:
            
            assert(pos.v == 0 || pos.v == 1);
            assert(pos.h == 0);
            
            // Trigger the vertical blank interrupt
            paula.raiseIrq(INT_VERTB);
            
            // Schedule next event
            scheduleStrobe1Event();
            break;

        case VBL_STROBE1:
            
            assert(pos.v == 5);
            assert(pos.h == 84);
            
            // Increment the TOD counter of CIA A
            ciaa.tod.increment();
            
            // Schedule next event
            scheduleStrobe2Event();
            break;
            
        case VBL_STROBE2:
            
            assert(pos.v == 5);
            assert(pos.h == 178);
            
            // Nothing is done here at the moment
            
            // Schedule next event
            scheduleStrobe0Event();
            break;
            
        default:
            fatalError;
    }
}

void
Agnus::serviceDASEvent(EventID id)
{
    assert(id == sequencer.dasEvent[pos.h]);

    switch (id) {

        case DAS_REFRESH:
            busOwner[0x01] = BUS_REFRESH;
            busOwner[0x03] = BUS_REFRESH;
            busOwner[0x05] = BUS_REFRESH;
            busOwner[0xE2] = BUS_REFRESH;
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

        default:
            fatalError;
    }

    // Schedule next event
    scheduleNextDasEvent();
}

void
Agnus::serviceINSEvent(EventID id)
{    
    switch (id) {

        case INS_AMIGA:
            
            amiga.inspect();
            break;
            
        case INS_CPU:
            
            cpu.inspect();
            break;
            
        case INS_MEM:
            
            mem.inspect();
            break;
            
        case INS_CIA:
            
            ciaa.inspect();
            ciab.inspect();
            break;
            
        case INS_AGNUS:
            
            inspect();
            break;
            
        case INS_PAULA:
            
            paula.inspect();
            break;
            
        case INS_DENISE:
            
            denise.inspect();
            break;
            
        case INS_PORTS:
            
            serialPort.inspect();
            paula.uart.inspect();
            controlPort1.inspect();
            controlPort2.inspect();
            break;
            
        case INS_EVENTS:
            
            scheduler.inspect();
            break;
                        
        default:
            fatalError;
    }

    // Reschedule event
    rescheduleRel<SLOT_INS>((Cycle)(inspectionInterval * 28000000));
}
