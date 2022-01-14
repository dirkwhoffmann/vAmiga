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

#ifdef LEGACY_DDF
void
Sequencer::updateBplEvents(u16 dmacon, u16 bplcon0)
{
    // Determine the number of active bitplanes
    auto channels = bpu(bplcon0);

    // Set the number to zero if no bitplane DMA takes place
    if (!inBplDmaLine(dmacon, bplcon0)) channels = 0;

    // Do the same if DDFSTRT is never reached in this line
    if (ddfstrtReached == -1) channels = 0;
    
    if (Denise::hires(bplcon0)) {
        updateBplEvents <true> (channels);
    } else {
        updateBplEvents <false> (channels);
    }
}

template <bool hi> void
Sequencer::updateBplEvents(isize channels)
{
    // Get the DDF window size
    auto strt = hi ? ddfHires.strt : ddfLores.strt;
    auto stop = hi ? ddfHires.stop : ddfLores.stop;
    
    assert(strt >= 0 && stop >= strt && stop <= 0xE0);

    // Determine the layout of a single fetch unit
    EventID slice[8]= { 0, 0, 0, 0, 0, 0, 0, 0 };
    
    if constexpr (hi) {
        
        switch(channels) {
                
            case 6:
            case 5:
            case 4: slice[0] = slice[4] = BPL_H4;
            case 3: slice[2] = slice[6] = BPL_H3;
            case 2: slice[1] = slice[5] = BPL_H2;
            case 1: slice[3] = slice[7] = BPL_H1;
        }
        
    } else {
        
        switch (channels) {
                
            case 6: slice[2] = BPL_L6;
            case 5: slice[6] = BPL_L5;
            case 4: slice[1] = BPL_L4;
            case 3: slice[5] = BPL_L3;
            case 2: slice[3] = BPL_L2;
            case 1: slice[7] = BPL_L1;
        }
    }
    
    // Update the event table
    for (isize i = 0; i < strt; i++) {
        bplEvent[i] = EVENT_NONE;
    }
    // trace(true, "strt = %d stop = %d\n", strt, stop);
    
    for (isize i = strt; i <= stop; i++) {
        bplEvent[i] = slice[(i - strt) & 7];
    }
    for (isize i = stop; i < HPOS_MAX; i++) {
        bplEvent[i] = EVENT_NONE;
    }
    bplEvent[HPOS_MAX] = BPL_EOL;
        
    // Superimpose the drawing flags
    hi ? updateHiresDrawingFlags() : updateLoresDrawingFlags();
            
    // Update the jump table
    updateBplJumpTable();
}
#endif

void
Sequencer::computeBplEvents()
{
    // Predict all events for the current scanline
    sigRecorder.clear();
        
    sigRecorder.insert(0, SIG_CON_L0 | agnus.bplcon0 >> 12);
    sigRecorder.insert(0x18, SIG_SHW);
    sigRecorder.insert(ddfstrt, SIG_BPHSTART);
    sigRecorder.insert(ddfstop, SIG_BPHSTOP);
    sigRecorder.insert(0xD8, SIG_RHW);
    sigRecorder.insert(HPOS_CNT, SIG_NONE);
    
    computeBplEvents(sigRecorder);
}

void
Sequencer::computeBplEvents(const SigRecorder &sr)
{
    auto state = ddfInitial;
    auto bmapen = agnus.dmaconInitial & (DMAEN | BPLEN);

    isize cnt = 0;
                
    // trace(true, "computeBplEvents\n");
    // dump(dump::Signals);
    // if (pos.v == 48) dump(dump::Dma);
    
    // The fetch unit layout
    EventID fetch[2][8];
    computeFetchUnit((u8)(agnus.bplcon0Initial >> 12), fetch);
    isize mask = (agnus.bplcon0Initial & 0x8000) ? 0b11 : 0b111;

    i64 cycle = 0;
    for (isize i = 0; i < sigRecorder.count(); i++) {
        
        auto signal = sigRecorder.elements[i];
        auto trigger = sigRecorder.keys[i];

        if (trigger > HPOS_CNT) break;
        
        //
        // Emulate the display logic up to the next signal change
        //
        
        for (isize j = cycle; j < trigger; j++) {
        
            assert(j >= 0 && j <= HPOS_MAX);
            
            EventID id;

            if (cnt == 0 && state.ff5) {

                // if (pos.v == 0x80) trace(true, "%ld: ff5 AND 0\n", j);
                state.ff2 = false;
                state.ff3 = false;
                state.ff5 = false;
            }
            if (cnt == 0 && state.ff4) {

                // if (pos.v == 0x80) trace(true, "%ld: ff4 AND 0\n", j);
                state.ff5 = true;
                state.ff4 = false;
            }
            if (state.ff3) {
                                
                id = fetch[state.ff5][cnt];
                cnt = (cnt + 1) & 7;
                
            } else {
                
                id = EVENT_NONE;
                cnt = 0;
            }
            
            // Superimpose drawing flags
            if ((j & mask) == (agnus.scrollOdd & mask))  id = (EventID)(id | 1);
            if ((j & mask) == (agnus.scrollEven & mask)) id = (EventID)(id | 2);
            
            bplEvent[j] = id;
        }
        
        //
        // Emulate the next signal change
        //
        
        if (signal & SIG_CON_L0) {
            
            computeFetchUnit((u8)(signal & 0xF), fetch);
            mask = (signal & 0x8) ? 0b11 : 0b111;
        }
        if (signal & SIG_BMAPEN_CLR) {
            
            bmapen = false;
            state.ff3 = false;
            cnt = 0;
        }
        if (signal & SIG_BMAPEN_SET) {
            
            bmapen = true;
        }
        if (signal & SIG_VFLOP_SET) {
            
        }
        if (signal & SIG_VFLOP_CLR) {
            
        }
        if (signal & SIG_SHW) {
            
            state.ff2 = true;
        }
        if (signal & SIG_RHW) {
            
            if (state.ff3) state.ff4 = true;
        }
        if (signal & (SIG_BPHSTART | SIG_BPHSTOP)) {
        
            if ((signal & SIG_BPHSTART) && (signal & SIG_BPHSTOP)) {

                // trace(true, "DDFSTRT && DDFSTOP\n");
                
                // OCS: BPHSTART wins
                if (state.ff3) {
                    signal &= ~SIG_BPHSTART;
                } else {
                    signal &= ~SIG_BPHSTOP;
                }
            }
            if (signal & SIG_BPHSTART) {
                
                if (state.ff2) state.ff3 = true;
                if (!state.ff1) state.ff3 = false;
                if (!bmapen) state.ff3 = false;
            }
            if (signal & SIG_BPHSTOP) {

                if (state.ff3) state.ff4 = true;
            }
        }
        
        cycle = trigger;
    }
    
    // Add the End Of Line event
    bplEvent[HPOS_MAX] = BPL_EOL;
                            
    // Update the jump table
    updateBplJumpTable();

    // Write back the new ddf state
    ddf = state;

    // Check if we need to recompute all event in the next scanline
    if (state != ddfInitial) {
        agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    }
}

void
Sequencer::computeFetchUnit(u8 dmacon, EventID id[2][8])
{
    switch (dmacon) {
            
        case 0x7: // L7
        case 0x6: // L6
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_L4;   id[1][1] = BPL_L4_MOD;
            id[0][2] = BPL_L6;   id[1][2] = BPL_L6_MOD;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_L3;   id[1][5] = BPL_L3_MOD;
            id[0][6] = BPL_L5;   id[1][6] = BPL_L5_MOD;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case 0x5: // L5
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_L4;   id[1][1] = BPL_L4_MOD;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_L3;   id[1][5] = BPL_L3_MOD;
            id[0][6] = BPL_L5;   id[1][6] = BPL_L5_MOD;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case 0x4: // L4
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_L4;   id[1][1] = BPL_L4_MOD;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_L3;   id[1][5] = BPL_L3_MOD;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case 0x3: // L3
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_L3;   id[1][5] = BPL_L3_MOD;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case 0x2: // L2
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = 0;        id[1][5] = 0;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case 0x1: // L1
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = 0;        id[1][3] = 0;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = 0;        id[1][5] = 0;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case 0xC: // H4
            
            id[0][0] = BPL_H4;   id[1][0] = BPL_H4;
            id[0][1] = BPL_H2;   id[1][1] = BPL_H2;
            id[0][2] = BPL_H3;   id[1][2] = BPL_H3;
            id[0][3] = BPL_H1;   id[1][3] = BPL_H1;
            id[0][4] = BPL_H4;   id[1][4] = BPL_H4_MOD;
            id[0][5] = BPL_H2;   id[1][5] = BPL_H2_MOD;
            id[0][6] = BPL_H3;   id[1][6] = BPL_H3_MOD;
            id[0][7] = BPL_H1;   id[1][7] = BPL_H1_MOD;
            break;

        case 0xB: // H3
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_H2;   id[1][1] = BPL_H2;
            id[0][2] = BPL_H3;   id[1][2] = BPL_H3;
            id[0][3] = BPL_H1;   id[1][3] = BPL_H1;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_H2;   id[1][5] = BPL_H2_MOD;
            id[0][6] = BPL_H3;   id[1][6] = BPL_H3_MOD;
            id[0][7] = BPL_H1;   id[1][7] = BPL_H1_MOD;
            break;

        case 0xA: // H2
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_H2;   id[1][1] = BPL_H2;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_H1;   id[1][3] = BPL_H1;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_H2;   id[1][5] = BPL_H2_MOD;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_H1;   id[1][7] = BPL_H1_MOD;
            break;

        case 0x9: // SIG_CON_H1:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_H1;   id[1][3] = BPL_H1;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = 0;        id[1][5] = 0;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_H1;   id[1][7] = BPL_H1_MOD;
            break;
            
        case 0x0: // L0
        case 0x8: // H0
        case 0xF: // H7
        case 0xE: // H6
        case 0xD: // H5
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = 0;        id[1][3] = 0;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = 0;        id[1][5] = 0;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = 0;        id[1][7] = 0;
            break;
            
        default:
            fatalError;
    }
}

void
Sequencer::updateBplJumpTable()
{
    u8 next = 0; // nextBplEvent[HPOS_MAX];
    
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
