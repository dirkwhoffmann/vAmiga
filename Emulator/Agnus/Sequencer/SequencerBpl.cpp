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

namespace vamiga {

void
Sequencer::initBplEvents()
{
    for (isize i = 0; i < HPOS_CNT; i++) bplEvent[i] = EVENT_NONE;
    for (isize i = 0; i < HPOS_CNT; i++) nextBplEvent[i] = HPOS_MAX;
}

void
Sequencer::initSigRecorder()
{
    sigRecorder.clear();

    sigRecorder.insert(0x18, SIG_SHW);
    sigRecorder.insert(ddfstrt, SIG_BPHSTART);
    sigRecorder.insert(ddfstop, SIG_BPHSTOP);
    sigRecorder.insert(0xD8, SIG_RHW);
    sigRecorder.insert(HPOS_CNT_NTSC, SIG_DONE);

    sigRecorder.modified = false;
}

void
Sequencer::computeBplEventTable(const SigRecorder &sr)
{
    agnus.isECS() ? computeBplEventTable <true> (sr) : computeBplEventTable <false> (sr);
}

template <bool ecs> void
Sequencer::computeBplEventTable(const SigRecorder &sr)
{
    trace(SEQ_DEBUG, "computeBplEvents\n");

    auto state = ddfInitial;

    // Update the DMA and BMCTL bits
    state.bmapen = agnus.bpldma(agnus.dmaconInitial);
    state.bplcon0 = agnus.bplcon0Initial;
    computeFetchUnit(state.bplcon0);
    
    // Evaluate the current state of the vertical DIW flipflop
    if (!state.bpv) { state.bprun = false; state.cnt = 0; }
    
    // Fill the event table
    if (sr.modified || (state.bpv && state.bmapen) || NO_SEQ_FASTPATH) {
        computeBplEventsSlow <ecs> (sr, state);
    } else {
        computeBplEventsFast <ecs> (sr, state);
    }
    
    // Update the jump table
    updateBplJumpTable();

    // Rectify the scheduled event
    agnus.scheduleBplEventForCycle(agnus.pos.h);
    
    // Write back the new ddf state
    ddf = state;

    // Check if we need to recompute all events in the next scanline
    if (state != ddfInitial) {

        trace(SEQ_DEBUG, "Recompute table in next line\n");
        hsyncActions |= UPDATE_BPL_TABLE;
    }
}

template <bool ecs> void
Sequencer::computeBplEventsFast(const SigRecorder &sr, DDFState &state)
{
    // This path can be taken only if bitplane DMA if off in the entire line
    assert(!sr.modified);
    assert(!state.bpv || !state.bmapen);

    trace(SEQ_DEBUG, "Fast path (no bitplane DMA in this line)\n");

    // Erase all events
    for (isize i = 0; i < HPOS_CNT; i++) bplEvent[i] = EVENT_NONE;

    // Add drawing flags
    auto odd = agnus.scrollOdd;
    auto even = agnus.scrollEven;
    switch (agnus.resolution(state.bplcon0)) {

        case LORES:

            odd &= 0b111;
            even &= 0b111;

            if (odd == even) {
                for (isize i = odd; i < HPOS_CNT; i += 8) bplEvent[i] = (EventID)3;
            } else {
                for (isize i = odd; i < HPOS_CNT; i += 8) bplEvent[i] = (EventID)1;
                for (isize i = even; i < HPOS_CNT; i += 8) bplEvent[i] = (EventID)2;
            }
            break;

        case HIRES:

            odd &= 0b11;
            even &= 0b11;

            if (odd == even) {
                for (isize i = odd; i < HPOS_CNT; i += 4) bplEvent[i] = (EventID)3;
            } else {
                for (isize i = odd; i < HPOS_CNT; i += 4) bplEvent[i] = (EventID)1;
                for (isize i = even; i < HPOS_CNT; i += 4) bplEvent[i] = (EventID)2;
            }
            break;

        case SHRES:

            odd &= 0b11;
            even &= 0b11;

            if (odd == even) {
                for (isize i = odd; i < HPOS_CNT; i += 2) bplEvent[i] = (EventID)3;
            } else {
                for (isize i = odd; i < HPOS_CNT; i += 2) bplEvent[i] = (EventID)1;
                for (isize i = even; i < HPOS_CNT; i += 2) bplEvent[i] = (EventID)2;
            }
            break;

        default:
            fatalError;
    }

    // Emulate all signal events
    for (isize i = 0;; i++) {
        
        auto signal = sigRecorder.elements[i];
        processSignal <ecs> (signal, state);
        
        if (signal & SIG_DONE) break;
    }
}

template <bool ecs> void
Sequencer::computeBplEventsSlow(const SigRecorder &sr, DDFState &state)
{
    trace(SEQ_DEBUG, "Slow path\n");
    
    // Iterate over all recorder signals
    for (isize i = 0, cycle = 0;; i++) {
        
        assert(i < sigRecorder.count());

        auto signal = sigRecorder.elements[i];
        isize trigger = (isize)sigRecorder.keys[i];
        
        assert(trigger <= HPOS_CNT_NTSC);
        
        // Emulate the display logic up to the next signal change
        computeBplEvents <ecs> (cycle, trigger, state);
        
        // Emulate the signal change
        processSignal <ecs> (signal, state);

        // Exit if the DONE signal has been processed
        if (signal & SIG_DONE) break;

        cycle = trigger;
    }
}

template <bool ecs> void
Sequencer::computeBplEvents(isize strt, isize stop, DDFState &state)
{
    isize mask;

    switch (agnus.resolution(state.bplcon0)) {

        case LORES: mask = 0b111; break;
        case HIRES: mask = 0b011; break;
        case SHRES: mask = 0b001; break;

        default:
            fatalError;
    }

    for (isize j = strt; j < stop; j++) {

        assert(j >= 0 && j <= HPOS_MAX);
        
        EventID id;

        auto counter = state.cnt << 1 | (j & 1);

        /*
         if (agnus.pos.v == 115) trace(true, "%d: %d %d %d %d %d %d %d %d %d %d\n", j, state.bpv, state.bmapen, state.shw, state.rhw, state.bphstart, state.bphstop, state.bprun, state.lastFu, state.bmctl, counter);
         */
        
        if (counter == 0) {

            if (state.lastFu) {

                // trace(1, "%d: STOP\n", j);
                state.bprun = false;
                state.lastFu = false;
                state.bphstop = false;
                if (!ecs) state.shw = false;
            }

            if (state.stopreq) {

                // trace(1, "%d: LASTFU\n", j);
                state.stopreq = false;
                state.lastFu = true;
            }
        }

        if (state.bprun) {

            id = fetch[state.lastFu ? 1 : 0][counter];
            if (IS_ODD(j)) state.cnt = (state.cnt + 1) & 3;
            
        } else {
            
            id = EVENT_NONE;
            state.cnt = 0;
        }

        // Superimpose drawing flags
        isize jj = j >= 1 ? j : HPOS_CNT_PAL + j;

        if ((jj & mask) == (agnus.scrollOdd & mask))  id = (EventID)(id | 1);
        if ((jj & mask) == (agnus.scrollEven & mask)) id = (EventID)(id | 2);
        
        bplEvent[j] = id;
    }
}

template <> void
Sequencer::processSignal <false> (u32 signal, DDFState &state)
{
    //
    // OCS logic
    //

    if (signal & SIG_CON) {
        
        state.bplcon0 = HI_WORD(signal);
        computeFetchUnit(state.bplcon0);
    }
    switch (signal & (SIG_BMAPEN_CLR | SIG_BMAPEN_SET)) {
            
        case SIG_BMAPEN_CLR:

            state.bmapen = false;
            state.bprun = false;
            state.cnt = 0;
            break;
            
        case SIG_BMAPEN_SET:

            state.bmapen = true;
            break;
    }
    switch (signal & (SIG_VFLOP_SET | SIG_VFLOP_CLR)) {
            
        case SIG_VFLOP_SET:

            state.bpv = true;
            break;
            
        case SIG_VFLOP_CLR:

            state.bpv = false;
            state.bprun = false;
            state.cnt = 0;
            break;
    }
    switch (signal & (SIG_SHW | SIG_RHW)) {
            
        case SIG_SHW:
            
            state.shw = true;
            break;
            
        case SIG_RHW:

            state.rhw |= state.bprun;
            state.stopreq |= state.bprun;
            break;
    }
    switch (signal & (SIG_BPHSTART | SIG_BPHSTOP)) {
            
        case SIG_BPHSTART | SIG_BPHSTOP:
            
            if (state.bprun) {
                
                state.bphstart &= !state.bprun;
                state.bphstop |= state.bprun;
                state.stopreq |= state.bprun;

            } else {
                
                state.bphstart = state.bphstart || state.shw;
                state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
            }
            break;

        case SIG_BPHSTART:

            state.bphstart |= state.shw && state.bmapen;
            state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
            break;
            
        case SIG_BPHSTOP:

            state.bphstart &= !state.bprun;
            state.bphstop |= state.bprun;
            state.stopreq |= state.bprun;
            break;
    }
    if (signal & SIG_DONE) {
        
        state.rhw = false;
        state.stopreq = false;
    }
}

template <> void
Sequencer::processSignal <true> (u32 signal, DDFState &state)
{
    //
    // ECS logic
    //
    
    if (signal & SIG_CON) {
        
        state.bplcon0 = HI_WORD(signal);
        computeFetchUnit(state.bplcon0);
    }
    switch (signal & (SIG_VFLOP_SET | SIG_VFLOP_CLR)) {
            
        case SIG_VFLOP_SET:

            state.bpv = true;
            break;
            
        case SIG_VFLOP_CLR:

            state.bpv = false;
            state.bprun = false;
            state.cnt = 0;
            break;
    }
    switch (signal & (SIG_SHW | SIG_RHW)) {
            
        case SIG_SHW:
            
            state.shw = true;
            state.bprun |= state.bphstart && !(signal & SIG_BPHSTOP);
            break;
            
        case SIG_RHW:

            state.rhw = true;
            state.stopreq |= state.bprun;
            // trace(1, "SIG_RHW: %d\n", state.bprun);
            break;
    }
    switch (signal & (SIG_BPHSTART | SIG_BPHSTOP | SIG_SHW | SIG_RHW)) {
            
        case SIG_BPHSTART | SIG_BPHSTOP | SIG_SHW:
            
            state.bphstart = true;
            state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
            break;
            
        case SIG_BPHSTART | SIG_BPHSTOP | SIG_RHW:
            
            state.bphstop |= state.bprun;
            state.stopreq |= state.bprun;
            state.bphstart = true;
            break;
            
        case SIG_BPHSTART | SIG_BPHSTOP:
            
            state.bphstop |= state.bprun;
            state.stopreq |= state.bprun;
            state.bphstart = true;
            state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
            break;

        case SIG_BPHSTART:
        case SIG_BPHSTART | SIG_SHW:
        case SIG_BPHSTART | SIG_RHW:

            state.bphstart = true;
            state.bprun = (state.bprun || state.shw) && state.bpv && state.bmapen;
            break;
            
        case SIG_BPHSTOP:
        case SIG_BPHSTOP | SIG_SHW:
        case SIG_BPHSTOP | SIG_RHW:

            // trace(1, "SIG_BPHSTOP\n");
            state.bphstart = false;
            state.bphstop |= state.bprun;
            state.stopreq |= state.bprun;
            break;
    }
    switch (signal & (SIG_BMAPEN_CLR | SIG_BMAPEN_SET)) {

        case SIG_BMAPEN_CLR:

            // trace(1, "SIG_BMAPEN_CLR\n");
            state.bmapen = false;
            state.bprun = false;
            state.cnt = 0;
            break;

        case SIG_BMAPEN_SET:

            state.bmapen = true;
            state.bprun = (state.bprun || state.shw) && state.bpv && state.bphstart;
            break;
    }
    if (signal & SIG_DONE) {
        
        state.rhw = false;
        state.shw = false;
        state.bphstop = false;
    }
}

void
Sequencer::updateBplJumpTable(i16 end)
{
    assert(end <= HPOS_MAX);
    assert(nextBplEvent[HPOS_MAX] == HPOS_MAX);

    u8 next = nextBplEvent[end];
    
    for (isize i = end; i >= 0; i--) {
        
        nextBplEvent[i] = next;
        if (bplEvent[i]) next = (i8)i;
    }
}

void
Sequencer::computeFetchUnit(u16 bplcon0)
{
    auto bpu = bplcon0 >> 12 & 0b111;

    switch (agnus.resolution(bplcon0)) {

        case LORES:

            switch (bpu) {

                case 0: computeLoresFetchUnit <0> (); break;
                case 1: computeLoresFetchUnit <1> (); break;
                case 2: computeLoresFetchUnit <2> (); break;
                case 3: computeLoresFetchUnit <3> (); break;
                case 4: computeLoresFetchUnit <4> (); break;
                case 5: computeLoresFetchUnit <5> (); break;
                case 6: computeLoresFetchUnit <6> (); break;
                case 7: computeLoresFetchUnit <4> (); break;
            }
            break;

        case HIRES:

            switch (bpu) {

                case 0: computeHiresFetchUnit <0> (); break;
                case 1: computeHiresFetchUnit <1> (); break;
                case 2: computeHiresFetchUnit <2> (); break;
                case 3: computeHiresFetchUnit <3> (); break;
                case 4: computeHiresFetchUnit <4> (); break;
                case 5: computeHiresFetchUnit <0> (); break;
                case 6: computeHiresFetchUnit <0> (); break;
                case 7: computeHiresFetchUnit <0> (); break;
            }
            break;

        case SHRES:

            switch (bpu) {

                case 0: computeShresFetchUnit <0> (); break;
                case 1: computeShresFetchUnit <1> (); break;
                case 2: computeShresFetchUnit <2> (); break;
                case 3: computeShresFetchUnit <0> (); break;
                case 4: computeShresFetchUnit <0> (); break;
                case 5: computeShresFetchUnit <0> (); break;
                case 6: computeShresFetchUnit <0> (); break;
                case 7: computeShresFetchUnit <0> (); break;
            }
            break;
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

template <u8 channels> void
Sequencer::computeShresFetchUnit()
{
    fetch[0][0] = channels < 2 ? 0 : BPL_S2;
    fetch[0][1] = channels < 1 ? 0 : BPL_S1;
    fetch[0][2] = channels < 2 ? 0 : BPL_S2;
    fetch[0][3] = channels < 1 ? 0 : BPL_S1;
    fetch[0][4] = channels < 2 ? 0 : BPL_S2;
    fetch[0][5] = channels < 1 ? 0 : BPL_S1;
    fetch[0][6] = channels < 2 ? 0 : BPL_S2;
    fetch[0][7] = channels < 1 ? 0 : BPL_S1;

    fetch[1][0] = channels < 2 ? 0 : BPL_S2;
    fetch[1][1] = channels < 1 ? 0 : BPL_S1;
    fetch[1][2] = channels < 2 ? 0 : BPL_S2;
    fetch[1][3] = channels < 1 ? 0 : BPL_S1;
    fetch[1][4] = channels < 2 ? 0 : BPL_S2;
    fetch[1][5] = channels < 1 ? 0 : BPL_S1;
    fetch[1][6] = channels < 2 ? 0 : BPL_S2_MOD;
    fetch[1][7] = channels < 1 ? 0 : BPL_S1_MOD;
}

}
