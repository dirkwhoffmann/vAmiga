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
 
    sigRecorder.modified = false;
    
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
    if (state != ddfInitial) hsyncActions |= UPDATE_BPL_TABLE;
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
    switch (signal & (SIG_BMAPEN_CLR | SIG_BMAPEN_SET)) {
            
        case SIG_BMAPEN_CLR:
        
            processSignal <ecs, SIG_BMAPEN_CLR> (state);
            break;
            
        case SIG_BMAPEN_SET:
        
            processSignal <ecs, SIG_BMAPEN_SET> (state);
            break;
    }
    switch (signal & (SIG_VFLOP_SET | SIG_VFLOP_CLR)) {
            
        case SIG_VFLOP_SET:
        
            processSignal <ecs, SIG_VFLOP_SET> (state);
            break;
            
        case SIG_VFLOP_CLR:
        
            processSignal <ecs, SIG_VFLOP_CLR> (state);
            break;
    }
    switch (signal & (SIG_SHW | SIG_RHW)) {
            
        case SIG_SHW:
            
            if (signal & SIG_BPHSTOP) {
                processSignal <ecs, SIG_SHW | SIG_BPHSTOP> (state);
            } else {
                processSignal <ecs, SIG_SHW> (state);
            }
            break;
            
        case SIG_RHW:

            processSignal <ecs, SIG_RHW> (state);
            break;
    }
    switch (signal & (SIG_BPHSTART | SIG_BPHSTOP | SIG_SHW | SIG_RHW)) {
            
        case SIG_BPHSTART | SIG_BPHSTOP | SIG_SHW:
            
            processSignal <ecs, SIG_BPHSTART | SIG_BPHSTOP | SIG_SHW> (state);
            break;
            
        case SIG_BPHSTART | SIG_BPHSTOP | SIG_RHW:
            
            processSignal <ecs, SIG_BPHSTART | SIG_BPHSTOP | SIG_RHW> (state);
            break;
            
        case SIG_BPHSTART | SIG_BPHSTOP:
            
            processSignal <ecs, SIG_BPHSTART | SIG_BPHSTOP> (state);
            break;
                        
        case SIG_BPHSTART:
        case SIG_BPHSTART | SIG_SHW:
        case SIG_BPHSTART | SIG_RHW:

            processSignal <ecs, SIG_BPHSTART> (state);
            break;
            
        case SIG_BPHSTOP:
        case SIG_BPHSTOP | SIG_SHW:
        case SIG_BPHSTOP | SIG_RHW:

            processSignal <ecs, SIG_BPHSTOP> (state);
            break;
    }
    if (signal & SIG_DONE) {
        
        processSignal <ecs, SIG_DONE> (state);
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

template <> void
Sequencer::processSignal <false, SIG_BPHSTART> (DDFState &state)
{
    // OCS
    state.bphstart = state.bphstart || state.shw;
    state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
}

template <> void
Sequencer::processSignal <true, SIG_BPHSTART> (DDFState &state)
{
    // ECS
    state.bphstart = true;
    state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
}

template <> void
Sequencer::processSignal <false, SIG_BPHSTOP> (DDFState &state)
{
    // OCS
    state.bphstart &= !state.bprun;
    state.bphstop |= state.bprun;
}

template <> void
Sequencer::processSignal <true, SIG_BPHSTOP> (DDFState &state)
{
    // ECS
    state.bphstart = false;
    state.bphstop |= state.bprun;
}

template <> void
Sequencer::processSignal <false, SIG_BPHSTART | SIG_BPHSTOP> (DDFState &state)
{
    // OCS
    if (state.bprun) {
        processSignal <false, SIG_BPHSTOP> (state);
    } else {
        processSignal <false, SIG_BPHSTART> (state);
    }
}

template <> void
Sequencer::processSignal <true, SIG_BPHSTART | SIG_BPHSTOP> (DDFState &state)
{
    // ECS
    state.bphstop |= state.bprun;
    state.bphstart = true;
    state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
}

template <> void
Sequencer::processSignal <false, SIG_BPHSTART | SIG_BPHSTOP | SIG_SHW> (DDFState &state)
{
    // OCS
    processSignal <false, SIG_BPHSTART | SIG_BPHSTOP> (state);
}

template <> void
Sequencer::processSignal <true, SIG_BPHSTART | SIG_BPHSTOP | SIG_SHW> (DDFState &state)
{
    // ECS
    state.bphstart = true;
    state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
}

template <> void
Sequencer::processSignal <false, SIG_BPHSTART | SIG_BPHSTOP | SIG_RHW> (DDFState &state)
{
    // OCS
    processSignal <false, SIG_BPHSTART | SIG_BPHSTOP> (state);
}

template <> void
Sequencer::processSignal <true, SIG_BPHSTART | SIG_BPHSTOP | SIG_RHW> (DDFState &state)
{
    // ECS
    state.bphstop |= state.bprun;
    state.bphstart = true;
}

template <> void
Sequencer::processSignal <false, SIG_DONE> (DDFState &state)
{
    // OCS
    state.rhw = false;
}

template <> void
Sequencer::processSignal <true, SIG_DONE> (DDFState &state)
{
    // ECS
    state.rhw = false;
    state.shw = false;
    state.bphstop = false;
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
