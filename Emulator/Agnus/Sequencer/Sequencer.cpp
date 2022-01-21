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
#include "Denise.h" // REMOVE ASAP

EventID Sequencer::dasDMA[64][HPOS_CNT];

Sequencer::Sequencer(Amiga& ref) : SubComponent(ref)
{
    initDasEventTable();
}

void
Sequencer::initDasEventTable()
{
    std::memset(dasDMA, 0, sizeof(dasDMA));

    for (isize enable = 0; enable < 64; enable++) {

        EventID *p = dasDMA[enable];

        p[0x01] = DAS_REFRESH;

        if (enable & DSKEN) {
            
            p[0x07] = DAS_D0;
            p[0x09] = DAS_D1;
            p[0x0B] = DAS_D2;
        }
        
        // Audio DMA is possible even in lines where the DMACON bits are false
        p[0x0D] = DAS_A0;
        p[0x0F] = DAS_A1;
        p[0x11] = DAS_A2;
        p[0x13] = DAS_A3;
        
        if (enable & SPREN) {
            
            p[0x15] = DAS_S0_1;
            p[0x17] = DAS_S0_2;
            p[0x19] = DAS_S1_1;
            p[0x1B] = DAS_S1_2;
            p[0x1D] = DAS_S2_1;
            p[0x1F] = DAS_S2_2;
            p[0x21] = DAS_S3_1;
            p[0x23] = DAS_S3_2;
            p[0x25] = DAS_S4_1;
            p[0x27] = DAS_S4_2;
            p[0x29] = DAS_S5_1;
            p[0x2B] = DAS_S5_2;
            p[0x2D] = DAS_S6_1;
            p[0x2F] = DAS_S6_2;
            p[0x31] = DAS_S7_1;
            p[0x33] = DAS_S7_2;
        }

        p[0xDF] = DAS_SDMA;
        p[0x66] = DAS_TICK;
    }
}

void
Sequencer::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    clearBplEvents();
    clearDasEvents();
}

void
Sequencer::clearBplEvents()
{
    for (isize i = 0; i < HPOS_MAX; i++) bplEvent[i] = EVENT_NONE;
    for (isize i = 0; i < HPOS_MAX; i++) nextBplEvent[i] = HPOS_MAX;
    bplEvent[HPOS_MAX] = BPL_EOL;
    nextBplEvent[HPOS_MAX] = 0;
}

void
Sequencer::computeBplEvents()
{
    agnus.isECS() ? computeBplEvents <true> () : computeBplEvents <false> ();
}

template <bool ecs> void
Sequencer::computeBplEvents()
{
    // Predict all events for the current scanline
    sigRecorder.clear();
        
    if (agnus.pos.v == diwVstop || agnus.inLastRasterline()) {
        sigRecorder.insert(0, SIG_VFLOP_CLR);
    } else if (agnus.pos.v == diwVstrt){
        sigRecorder.insert(0, SIG_VFLOP_SET);
    }
    
    sigRecorder.insert(0x18, SIG_SHW);
    sigRecorder.insert(ddfstrt, SIG_BPHSTART);
    sigRecorder.insert(ddfstop, SIG_BPHSTOP);
    sigRecorder.insert(0xD8, SIG_RHW);
    sigRecorder.insert(HPOS_MAX, SIG_DONE);
    
    computeBplEvents <ecs> (sigRecorder);
}

void
Sequencer::computeBplEvents(const SigRecorder &sr)
{
    agnus.isECS() ? computeBplEvents <true> (sr) : computeBplEvents <false> (sr);
}

template <bool ecs> void
Sequencer::computeBplEvents(const SigRecorder &sr)
{
    auto state = ddfInitial;
    
    i64 cycle = 0;
    i64 trigger = 0;
    u16 signal = 0;

    /*
    if (agnus.pos.v == 0x43 || agnus.pos.v == 0x42) {
    
        trace(true, "Signals:\n");
        dump(dump::Signals);
        dump(dump::State);
    }
    */
     
    for (isize i = 0; !(signal & SIG_DONE); i++) {
        
        assert(i < sigRecorder.count());
        
        signal = sigRecorder.elements[i];
        trigger = sigRecorder.keys[i];
        
        assert(trigger < HPOS_CNT);
        
        // Emulate the display logic up to the next signal change
        computeBplEvents <ecs> (cycle, trigger, state);
        
        // Emulate the signal change
        processSignal <ecs> (signal, state);
        
        cycle = trigger;
    }
    
    // Add the End Of Line event
    bplEvent[HPOS_MAX] = BPL_EOL;
                            
    // Update the jump table
    updateBplJumpTable();

    // Rectify the currently scheduled event
    agnus.scheduleBplEventForCycle(agnus.pos.h);
    
    // Write back the new ddf state
    ddf = state;

    // Check if we need to recompute all events in the next scanline
    if (state != ddfInitial) {
        agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    }
}

template <bool ecs> void
Sequencer::computeBplEvents(isize strt, isize stop, DDFState &state)
{
    isize mask = (state.bmctl & 0x8) ? 0b11 : 0b111;
    
    for (isize j = strt; j < stop; j++) {
    
        assert(j >= 0 && j <= HPOS_MAX);
        
        EventID id;

        if (state.cnt == 0 && state.bprun) {
    
            if (state.lastFu) {
                
                state.bprun = false;
                state.lastFu = false;
                state.bphstop = false;
                if (!ecs) state.shw = false;

            } else if (state.rhw || state.bphstop) {
                
                state.lastFu = true;
            }
        }
    
        if (state.bprun) {
                            
            id = fetch[state.lastFu ? 1 : 0][state.cnt];
            state.cnt = (state.cnt + 1) & 7;
            
        } else {
            
            id = EVENT_NONE;
            state.cnt = 0;
        }
        
        // Superimpose drawing flags
        if ((j & mask) == (agnus.scrollOdd & mask))  id = (EventID)(id | 1);
        if ((j & mask) == (agnus.scrollEven & mask)) id = (EventID)(id | 2);
        
        bplEvent[j] = id;
    }
}

template <bool ecs> void
Sequencer::processSignal(u16 signal, DDFState &state)
{
    if (signal & SIG_CON) {
        
        state.bmctl = (u8)(signal & 0xF);
        computeFetchUnit(state.bmctl);
    }
    if (signal & SIG_BMAPEN_CLR) {
        
        processSignal <ecs, SIG_BMAPEN_CLR> (state);
    }
    /*
        state.bmapen = false;
        state.bprun = false;
        state.cnt = 0;
    }
    */
    if (signal & SIG_BMAPEN_SET) {
     
        processSignal <ecs, SIG_BMAPEN_SET> (state);
        // state.bmapen = true;
    }
    if (signal & SIG_VFLOP_SET) {
        
        processSignal <ecs, SIG_VFLOP_SET> (state);
        /*
        state.bpv = true;
        lineIsBlank = false;
        */
    }
    if (signal & SIG_VFLOP_CLR) {
        
        processSignal <ecs, SIG_VFLOP_CLR> (state);
        /*
        state.bpv = false;
        state.bprun = false;
        state.cnt = 0;
        */
    }
    if (signal & SIG_SHW) {
        
        if (signal & SIG_BPHSTOP) {
            processSignal <ecs, SIG_SHW | SIG_BPHSTOP> (state);
        } else {
            processSignal <ecs, SIG_SHW> (state);
        }
    }
    if (signal & SIG_RHW) {
        
        processSignal <ecs, SIG_RHW> (state);
        /*
        if (ecs) {
            state.rhw = true;
        } else {
            if (state.bprun) state.rhw = true;
        }
        */
    }
    if (signal & (SIG_BPHSTART | SIG_BPHSTOP)) {
        
        if (ecs) {
            
            if ((signal & SIG_BPHSTART) && (signal & SIG_BPHSTOP)) {
                
                if (state.bprun && !(signal & SIG_SHW)) state.bphstop = true;
                
                state.bphstart = true;
                if (state.shw) {
                    state.bprun = true;
                }
                if (!state.bpv) state.bprun = false;
                if (!state.bmapen) state.bprun = false;
                
            } else if (signal & SIG_BPHSTART) {
                
                state.bphstart = true;
                if (state.shw) {
                    state.bprun = true;
                }
                if (!state.bpv) state.bprun = false;
                if (!state.bmapen) state.bprun = false;
                
            } else if (signal & SIG_BPHSTOP) {
                
                state.bphstart = false;
                if (state.bprun) {
                    state.bphstop = true;
                }
            }
            
        } else {
            
            if (state.bprun) {
                signal &= ~SIG_BPHSTART;
            } else {
                signal &= ~SIG_BPHSTOP;
            }
            
            if (signal & SIG_BPHSTART) {
                
                if (state.shw) {
                    state.bphstart = true;
                    state.bprun = true;
                }
                if (!state.bpv) state.bprun = false;
                if (!state.bmapen) state.bprun = false;
            }
            if (signal & SIG_BPHSTOP) {
                
                if (state.bprun) {
                    state.bphstart = false;
                    state.bphstop = true;
                }
            }
        }
    }
    if (signal & SIG_DONE) {
        
        state.rhw = false;
        if (ecs) state.shw = state.bphstop = false;
    }
}

template <> void
Sequencer::processSignal <false, SIG_BMAPEN_CLR> (DDFState &state)
{
    // OCS
    state.bmapen = false;
    state.bprun = false;
    state.cnt = 0;
}

template <> void
Sequencer::processSignal <true, SIG_BMAPEN_CLR> (DDFState &state)
{
    // ECS
    state.bmapen = false;
    state.bprun = false;
    state.cnt = 0;
}

template <> void
Sequencer::processSignal <false, SIG_BMAPEN_SET> (DDFState &state)
{
    // OCS
    state.bmapen = true;
}

template <> void
Sequencer::processSignal <true, SIG_BMAPEN_SET> (DDFState &state)
{
    // ECS
    state.bmapen = true;
}

template <> void
Sequencer::processSignal <false, SIG_VFLOP_SET> (DDFState &state)
{
    // OCS
    state.bpv = true;
    lineIsBlank = false;
}

template <> void
Sequencer::processSignal <true, SIG_VFLOP_SET> (DDFState &state)
{
    // ECS
    state.bpv = true;
    lineIsBlank = false;
}

template <> void
Sequencer::processSignal <false, SIG_VFLOP_CLR> (DDFState &state)
{
    // OCS
    state.bpv = false;
    state.bprun = false;
    state.cnt = 0;
}

template <> void
Sequencer::processSignal <true, SIG_VFLOP_CLR> (DDFState &state)
{
    // ECS
    state.bpv = false;
    state.bprun = false;
    state.cnt = 0;
}

template <> void
Sequencer::processSignal <false, SIG_SHW | SIG_BPHSTOP> (DDFState &state)
{
    // OCS
    state.shw = true;
}

template <> void
Sequencer::processSignal <true, SIG_SHW | SIG_BPHSTOP> (DDFState &state)
{
    // ECS
    state.shw = true;
}

template <> void
Sequencer::processSignal <false, SIG_SHW> (DDFState &state)
{
    // OCS
    state.shw = true;
}

template <> void
Sequencer::processSignal <true, SIG_SHW> (DDFState &state)
{
    // ECS
    state.shw = true;
    state.bprun |= state.bphstart;
}

template <> void
Sequencer::processSignal <false, SIG_RHW> (DDFState &state)
{
    // OCS
    state.rhw |= state.bprun;
}

template <> void
Sequencer::processSignal <true, SIG_RHW> (DDFState &state)
{
    // ECS
    state.rhw = true;
}

void
Sequencer::computeFetchUnit(u8 dmacon)
{
    if (dmacon & 0x8) {
        
        switch (dmacon & 0x7) {
        
            case 0: computeHiresFetchUnit <0> (); break;
            case 1: computeHiresFetchUnit <1> (); break;
            case 2: computeHiresFetchUnit <2> (); break;
            case 3: computeHiresFetchUnit <3> (); break;
            case 4: computeHiresFetchUnit <4> (); break;
            case 5: computeHiresFetchUnit <0> (); break;
            case 6: computeHiresFetchUnit <0> (); break;
            case 7: computeHiresFetchUnit <0> (); break;
        }

    } else {
      
        switch (dmacon & 0x7) {
        
            case 0: computeLoresFetchUnit <0> (); break;
            case 1: computeLoresFetchUnit <1> (); break;
            case 2: computeLoresFetchUnit <2> (); break;
            case 3: computeLoresFetchUnit <3> (); break;
            case 4: computeLoresFetchUnit <4> (); break;
            case 5: computeLoresFetchUnit <5> (); break;
            case 6: computeLoresFetchUnit <6> (); break;
            case 7: computeLoresFetchUnit <4> (); break;
        }
    }
}

template <u8 channels> void
Sequencer::computeLoresFetchUnit()
{
    fetch[0][0] = 0;
    fetch[0][1] = channels < 4 ? 0 : BPL_L4;
    fetch[0][2] = channels < 6 ? 0 : BPL_L6;
    fetch[0][3] = channels < 2 ? 0 : BPL_L2;
    fetch[0][4] = 0;
    fetch[0][5] = channels < 3 ? 0 : BPL_L3;
    fetch[0][6] = channels < 5 ? 0 : BPL_L5;
    fetch[0][7] = channels < 1 ? 0 : BPL_L1;

    fetch[1][0] = 0;
    fetch[1][1] = channels < 4 ? 0 : BPL_L4_MOD;
    fetch[1][2] = channels < 6 ? 0 : BPL_L6_MOD;
    fetch[1][3] = channels < 2 ? 0 : BPL_L2_MOD;
    fetch[1][4] = 0;
    fetch[1][5] = channels < 3 ? 0 : BPL_L3_MOD;
    fetch[1][6] = channels < 5 ? 0 : BPL_L5_MOD;
    fetch[1][7] = channels < 1 ? 0 : BPL_L1_MOD;
}

template <u8 channels> void
Sequencer::computeHiresFetchUnit()
{
    fetch[0][0] = channels < 4 ? 0 : BPL_H4;
    fetch[0][1] = channels < 2 ? 0 : BPL_H2;
    fetch[0][2] = channels < 3 ? 0 : BPL_H3;
    fetch[0][3] = channels < 1 ? 0 : BPL_H1;
    fetch[0][4] = channels < 4 ? 0 : BPL_H4;
    fetch[0][5] = channels < 2 ? 0 : BPL_H2;
    fetch[0][6] = channels < 3 ? 0 : BPL_H3;
    fetch[0][7] = channels < 1 ? 0 : BPL_H1;

    fetch[1][0] = channels < 4 ? 0 : BPL_H4;
    fetch[1][1] = channels < 2 ? 0 : BPL_H2;
    fetch[1][2] = channels < 3 ? 0 : BPL_H3;
    fetch[1][3] = channels < 1 ? 0 : BPL_H1;
    fetch[1][4] = channels < 4 ? 0 : BPL_H4_MOD;
    fetch[1][5] = channels < 2 ? 0 : BPL_H2_MOD;
    fetch[1][6] = channels < 3 ? 0 : BPL_H3_MOD;
    fetch[1][7] = channels < 1 ? 0 : BPL_H1_MOD;
}

void
Sequencer::updateBplJumpTable()
{
    u8 next = 0;
    
    for (isize i = HPOS_MAX; i >= 0; i--) {
        
        nextBplEvent[i] = next;
        if (bplEvent[i]) next = (i8)i;
    }
}

void
Sequencer::clearDasEvents()
{
    for (isize i = 0; i < HPOS_CNT; i++) dasEvent[i] = dasDMA[0][i];
    updateDasJumpTable();
}

void
Sequencer::updateDasEvents(u16 dmacon)
{
    assert(dmacon < 64);

    // Allocate slots
    for (isize i = 0; i < 0x38; i++) dasEvent[i] = dasDMA[dmacon][i];
    
    // Update the jump table
    updateDasJumpTable(0x38);
}

void
Sequencer::updateDasJumpTable(i16 end)
{
    assert(end <= HPOS_MAX);

    u8 next = nextDasEvent[end];
    
    for (isize i = end; i >= 0; i--) {
        
        nextDasEvent[i] = next;
        if (dasEvent[i]) next = (i8)i;
    }
}

void
Sequencer::hsyncHandler()
{
    diwVstrtInitial = diwVstrt;
    diwVstopInitial = diwVstop;
    ddfInitial = ddf;
    
    if (agnus.pos.v == diwVstrt) {
        
        trace(DDF_DEBUG, "DDF: FF1 = 1 (DIWSTRT)\n");
        agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    }
    if (agnus.pos.v == diwVstop) {
        
        trace(DDF_DEBUG, "DDF: FF1 = 0 (DIWSTOP)\n");
        agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    }
    if (agnus.inLastRasterline()) {
        
        trace(DDF_DEBUG, "DDF: FF1 = 0 (EOF)\n");
        agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    }

    lineIsBlank = !ddfInitial.bpv;
}

void
Sequencer::vsyncHandler()
{

}
