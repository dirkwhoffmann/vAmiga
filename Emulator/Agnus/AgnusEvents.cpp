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
    
    u8 dmacycle = nextBplEvent[0];
    assert(dmacycle != 0);
        
    if (pos.h == 0) {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(dmacycle), bplEvent[dmacycle]);
    } else {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(dmacycle + 1), bplEvent[dmacycle]);
    }
}

void
Agnus::scheduleNextBplEvent(isize hpos)
{
    assert(hpos >= 0 && hpos < HPOS_CNT);

    if (u8 next = nextBplEvent[hpos]) {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(next - pos.h), bplEvent[next]);
    }
    assert(scheduler.hasEvent<SLOT_BPL>());
}

void
Agnus::scheduleBplEventForCycle(isize hpos)
{
    assert(hpos >= pos.h && hpos < HPOS_CNT);

    if (bplEvent[hpos] != EVENT_NONE) {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(hpos - pos.h), bplEvent[hpos]);
    } else {
        scheduleNextBplEvent(hpos);
    }

    assert(scheduler.hasEvent<SLOT_BPL>());
}

void
Agnus::scheduleFirstDasEvent()
{
    assert(pos.h == 0 || pos.h == HPOS_MAX);
    
    u8 dmacycle = nextDasEvent[0];
    assert(dmacycle != 0);
    
    if (pos.h == 0) {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(dmacycle), dasEvent[dmacycle]);
    } else {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(dmacycle + 1), dasEvent[dmacycle]);
    }
}

void
Agnus::scheduleNextDasEvent(isize hpos)
{
    assert(hpos >= 0 && hpos < HPOS_CNT);

    if (u8 next = nextDasEvent[hpos]) {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(next - pos.h), dasEvent[next]);
        assert(scheduler.hasEvent<SLOT_DAS>());
    } else {
        scheduler.cancel<SLOT_DAS>();
    }
}

void
Agnus::scheduleDasEventForCycle(isize hpos)
{
    assert(hpos >= pos.h && hpos < HPOS_CNT);

    if (dasEvent[hpos] != EVENT_NONE) {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(hpos - pos.h), dasEvent[hpos]);
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
                
            case SET_DIWSTRT: setDIWSTRT(change.value); break;
            case SET_DIWSTOP: setDIWSTOP(change.value); break;
                
            case SET_DDFSTRT: setDDFSTRT(ddfstrt, change.value); break;
            case SET_DDFSTOP: setDDFSTOP(ddfstop, change.value); break;
                
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
    agnus.recordRegisterChange(DMA_CYCLES(1), SET_STRHOR, 1);
    
    // Call the hsync handler
    // hsyncHandler();

    // Reset the horizontal counter (-1 to compensate for the increment to come)
    pos.h = -1;
    
    // Reschedule event
    rescheduleRel<SLOT_RAS>(DMA_CYCLES(HPOS_CNT));
}

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

        case EVENT_NONE | DRAW_ODD | DRAW_EVEN:
            hires() ? denise.drawHiresBoth() : denise.drawLoresBoth();
            break;
            
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
            serviceBPLEventHires<1>();
            denise.drawHiresOdd();
            break;
            
        case BPL_H2 | DRAW_EVEN:
            serviceBPLEventHires<1>();
            denise.drawHiresEven();
            break;
            
        case BPL_H2 | DRAW_ODD | DRAW_EVEN:
            serviceBPLEventHires<1>();
            denise.drawHiresBoth();
            break;

        case BPL_L2:
            serviceBPLEventLores<1>();
            break;

        case BPL_L2 | DRAW_ODD:
            serviceBPLEventLores<1>();
            denise.drawLoresOdd();
            break;
            
        case BPL_L2 | DRAW_EVEN:
            serviceBPLEventLores<1>();
            denise.drawLoresEven();
            break;
            
        case BPL_L2 | DRAW_ODD | DRAW_EVEN:
            serviceBPLEventLores<1>();
            denise.drawLoresBoth();
            break;

        case BPL_H3:
            serviceBPLEventHires<2>();
            break;

        case BPL_H3 | DRAW_ODD:
            serviceBPLEventHires<2>();
            denise.drawHiresOdd();
            break;

        case BPL_H3 | DRAW_EVEN:
            serviceBPLEventHires<2>();
            denise.drawHiresEven();
            break;

        case BPL_H3 | DRAW_ODD | DRAW_EVEN:
            serviceBPLEventHires<2>();
            denise.drawHiresBoth();
            break;

        case BPL_L3:
            serviceBPLEventLores<2>();
            break;

        case BPL_L3 | DRAW_ODD:
            serviceBPLEventLores<2>();
            denise.drawLoresOdd();
            break;

        case BPL_L3 | DRAW_EVEN:
            serviceBPLEventLores<2>();
            denise.drawLoresEven();
            break;

        case BPL_L3 | DRAW_ODD | DRAW_EVEN:
            serviceBPLEventLores<2>();
            denise.drawLoresBoth();
            break;

        case BPL_H4:
            serviceBPLEventHires<3>();
            break;

        case BPL_H4 | DRAW_ODD:
            serviceBPLEventHires<3>();
            denise.drawHiresOdd();
            break;

        case BPL_H4 | DRAW_EVEN:
            serviceBPLEventHires<3>();
            denise.drawHiresEven();
            break;

        case BPL_H4 | DRAW_ODD | DRAW_EVEN:
            serviceBPLEventHires<3>();
            denise.drawHiresBoth();
            break;

        case BPL_L4:
            serviceBPLEventLores<3>();
            break;

        case BPL_L4 | DRAW_ODD:
            serviceBPLEventLores<3>();
            denise.drawLoresOdd();
            break;

        case BPL_L4 | DRAW_EVEN:
            serviceBPLEventLores<3>();
            denise.drawLoresEven();
            break;

        case BPL_L4 | DRAW_ODD | DRAW_EVEN:
            serviceBPLEventLores<3>();
            denise.drawLoresBoth();
            break;

        case BPL_L5:
            serviceBPLEventLores<4>();
            break;

        case BPL_L5 | DRAW_ODD:
            serviceBPLEventLores<4>();
            denise.drawLoresOdd();
            break;

        case BPL_L5 | DRAW_EVEN:
            serviceBPLEventLores<4>();
            denise.drawLoresEven();
            break;

        case BPL_L5 | DRAW_ODD | DRAW_EVEN:
            serviceBPLEventLores<4>();
            denise.drawLoresBoth();
            break;
            
        case BPL_L6:
            serviceBPLEventLores<5>();
            break;

        case BPL_L6 | DRAW_ODD:
            serviceBPLEventLores<5>();
            denise.drawLoresOdd();
            break;

        case BPL_L6 | DRAW_EVEN:
            serviceBPLEventLores<5>();
            denise.drawLoresEven();
            break;

        case BPL_L6 | DRAW_ODD | DRAW_EVEN:
            serviceBPLEventLores<5>();
            denise.drawLoresBoth();
            break;

        case BPL_SR:
            denise.updateShiftRegisters();
            break;
            
        case BPL_SR | DRAW_ODD:
            hires() ? denise.drawHiresOdd() : denise.drawLoresOdd();
            break;
            
        case BPL_SR | DRAW_EVEN:
            hires() ? denise.drawHiresEven() : denise.drawLoresEven();
            break;
            
        case BPL_SR | DRAW_ODD | DRAW_EVEN:
            hires() ? denise.drawHiresBoth() : denise.drawLoresBoth();
            break;
            
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

        case BPL_EOL | DRAW_ODD | DRAW_EVEN:
            assert(pos.h == 0xE2);
            hires() ? denise.drawHiresBoth() : denise.drawLoresBoth();
            return;
            
        default:
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
    
    // Add modulo if this is the last fetch unit
    if (pos.h >= ddfHires.stop - 4) {

        // trace(BPLMOD_DEBUG,
        //       "Adding bpl%dmod = %d\n", (nr % 2), (nr % 2) ? bpl2mod : bpl1mod);

        bplpt[nr] += (nr % 2) ? bpl2mod : bpl1mod;
    }
}

template <isize nr> void
Agnus::serviceBPLEventLores()
{
    // Perform bitplane DMA
    denise.setBPLxDAT<nr>(doBitplaneDmaRead<nr>());

    // Add modulo if this is the last fetch unit
    if (pos.h >= ddfLores.stop - 8) {

        // trace(BPLMOD_DEBUG,
        //       "Adding bpl%dmod = %d\n", (nr % 2), (nr % 2) ? bpl2mod : bpl1mod);

        bplpt[nr] += (nr % 2) ? bpl2mod : bpl1mod;
    }
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
    assert(id == dasEvent[pos.h]);

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
