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
#include "Denise.h"

/* A central element in the emulation of the Amiga is the accurate modeling of
* the DMA timeslot allocation table (Fig. 6-9 im the HRM, 3rd revision). All
* bitplane related events are managed in the BPL_SLOT. All disk, audio, and
* sprite related events are managed in the DAS_SLOT.
*
* vAmiga utilizes two event tables to schedule events in the DAS_SLOT and
* BPL_SLOT. Assuming that sprite DMA is enabled and Denise draws 6 bitplanes
* in lores mode starting at 0x28, the tables would look like this:
*
*     bplEvent[0x00] = EVENT_NONE   dasEvent[0x00] = EVENT_NONE
*     bplEvent[0x01] = EVENT_NONE   dasEvent[0x01] = BUS_REFRESH
*         ...                           ...
*     bplEvent[0x28] = EVENT_NONE   dasEvent[0x28] = EVENT_NONE
*     bplEvent[0x29] = BPL_L4       dasEvent[0x29] = DAS_S5_1
*     bplEvent[0x2A] = BPL_L6       dasEvent[0x2A] = EVENT_NONE
*     bplEvent[0x2B] = BPL_L2       dasEvent[0x2B] = DAS_S5_2
*     bplEvent[0x2C] = EVENT_NONE   dasEvent[0x2C] = EVENT_NONE
*     bplEvent[0x2D] = BPL_L3       dasEvent[0x2D] = DAS_S6_1
*     bplEvent[0x2E] = BPL_L5       dasEvent[0x2E] = EVENT_NONE
*     bplEvent[0x2F] = BPL_L1       dasEvent[0x2F] = DAS_S6_2
*         ...                           ...
*     bplEvent[0xE2] = BPL_EOL      dasEvent[0xE2] = BUS_REFRESH
*
* The BPL_EOL event doesn't perform DMA. It concludes the current line.
*
* All events in the BPL_SLOT can be superimposed by two drawing flags (bit 0
* and bit 1) that trigger the transfer of the data registers into the shift
* registers at the correct DMA cycle. Bit 0 controls the odd bitplanes and
* bit 1 controls the even bitplanes.
*
* Each event table is accompanied by a jump table that points to the next
* event. Given the example tables above, the jump tables would look like this:
*
*     nextBplEvent[0x00] = 0x29     nextDasEvent[0x00] = 0x01
*     nextBplEvent[0x01] = 0x29     nextDasEvent[0x01] = 0x03
*           ...                           ...
*     nextBplEvent[0x28] = 0x29     nextDasEvent[0x28] = 0x29
*     nextBplEvent[0x29] = 0x2A     nextDasEvent[0x29] = 0x2B
*     nextBplEvent[0x2A] = 0x2B     nextDasEvent[0x2A] = 0x2B
*     nextBplEvent[0x2B] = 0x2D     nextDasEvent[0x2B] = 0x2D
*     nextBplEvent[0x2C] = 0x2D     nextDasEvent[0x2C] = 0x2D
*     nextBplEvent[0x2D] = 0x2E     nextDasEvent[0x2D] = 0x2F
*     nextBplEvent[0x2E] = 0x2F     nextDasEvent[0x2E] = 0x2F
*     nextBplEvent[0x2F] = 0x31     nextDasEvent[0x2F] = 0x31
*           ...                           ...
*     nextBplEvent[0xE2] = 0x00     nextDasEvent[0xE2] = 0x00
*
* Whenever one the DMA tables is modified, the corresponding jump table
* has to be updated, too.
*
* To quickly setup the event tables, vAmiga utilizes two static lookup
* tables. Depending on the current resoution, BPU value, and DMA status,
* segments of these lookup tables are copied to the event tables.
*
*      Table: bitplaneDMA[Resolution][Bitplanes][Cycle]
*
*             (Bitplane DMA events in a single rasterline)
*
*             Resolution : 0 or 1        (0 = LORES / 1 = HIRES)
*              Bitplanes : 0 .. 6        (Bitplanes in use, BPU)
*                  Cycle : 0 .. HPOS_MAX (DMA cycle)
*
*      Table: dasDMA[dmacon]
*
*             (Disk, Audio, and Sprite DMA events in a single rasterline)
*
*                 dmacon : Bits 0 .. 5 of register DMACON
*/

template <> bool Agnus::auddma<0>(u16 v) { return (v & DMAEN) && (v & AUD0EN); }
template <> bool Agnus::auddma<1>(u16 v) { return (v & DMAEN) && (v & AUD1EN); }
template <> bool Agnus::auddma<2>(u16 v) { return (v & DMAEN) && (v & AUD2EN); }
template <> bool Agnus::auddma<3>(u16 v) { return (v & DMAEN) && (v & AUD3EN); }

void
Agnus::initDasEventTable()
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
Agnus::enableBplDmaOCS()
{
#ifdef LEGACY_DDF
    
    if (pos.h + 2 < ddfstrtReached || bpldma(dmaconAtDDFStrt)) {
        
        updateBplEvents(dmacon, bplcon0);
        updateBplEvent();
    }
    
#endif
}

void
Agnus::disableBplDmaOCS()
{
#ifdef LEGACY_DDF
    
        updateBplEvents(dmacon, bplcon0);
        updateBplEvent();

#endif
}

void
Agnus::enableBplDmaECS()
{
#ifdef LEGACY_DDF
    
    if (pos.h + 2 < ddfstrtReached) {
        
        updateBplEvents(dmacon, bplcon0);
        updateBplEvent();
        return;
    }
    
    if (pos.h + 2 < ddfstopReached) {
        
        ddfLores.compute(std::max(pos.h + 4, ddfstrtReached), ddfstopReached);
        ddfHires.compute(std::max(pos.h + 4, ddfstrtReached), ddfstopReached);
        hsyncActions |= HSYNC_PREDICT_DDF;
        
        updateBplEvents();
        updateBplEvent();
        // updateLoresDrawingFlags(); // THIS CAN'T BE RIGHT
    }
    
#endif
}

void
Agnus::disableBplDmaECS()
{
#ifdef LEGACY_DDF
    
    updateBplEvents(dmacon, bplcon0);
    updateBplEvent();

#endif
}

template <BusOwner owner> bool
Agnus::busIsFree()
{
    // Deny if the bus is in use
    if (busOwner[pos.h] != BUS_NONE) return false;

    if constexpr (owner == BUS_COPPER) {
        
        // Deny if Copper DMA is disabled
        if (!copdma()) return false;
        
        // Deny in cycle E0
        if (pos.h == 0xE0) {
         
            // If the Copper wants the bus in E0, nobody can have it
            busOwner[pos.h] = BUS_BLOCKED;
            
            return false;
        }
        
        return true;
    }
    
    if constexpr (owner == BUS_BLITTER) {
        
        // Deny if Blitter DMA is disabled
        if (!bltdma()) return false;
        
        // Deny if the CPU has precedence
        if (bls && !bltpri()) return false;
        
        return true;
    }
}

void
Agnus::clearBplEvents()
{
    for (isize i = 0; i < HPOS_MAX; i++) bplEvent[i] = EVENT_NONE;
    for (isize i = 0; i < HPOS_MAX; i++) nextBplEvent[i] = HPOS_MAX;
}

#ifdef LEGACY_DDF
void
Agnus::updateBplEvents(u16 dmacon, u16 bplcon0)
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
Agnus::updateBplEvents(isize channels)
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
Agnus::computeBplEvents()
{
    // Predict all events for the current scanline
    sigRecorder.clear();
        
    sigRecorder.insert(0,        bplcon0 >> 12);
    sigRecorder.insert(0x18,     SIG_SHW);
    sigRecorder.insert(ddfstrt,  SIG_BPHSTART);
    sigRecorder.insert(ddfstop,  SIG_BPHSTOP);
    sigRecorder.insert(0xD8,     SIG_RHW);
    sigRecorder.insert(HPOS_CNT, SIG_NONE);
    
    computeBplEvents(sigRecorder);
}

void
Agnus::computeBplEvents(const SigRecorder &sr)
{
    auto state = ddfInitial;
    auto bmapen = dmaconInitial & (DMAEN | BPLEN);

    // REMOVE ASAP
    int min = 1000;
    int max = 0;
    bool hires = false;

    isize cnt = 0;
                
    // The fetch unit layout
    EventID fetch[2][8];
    computeFetchUnit((u8)(bplcon0Initial >> 12), fetch);
    
    i64 cycle = 0;
    for (isize i = 0, end = sigRecorder.end(); i < end; i++) {
        
        auto trigger = sigRecorder.keys[i];
        auto signal = sigRecorder.elements[i];
        
        //
        // Emulate the display logic up to the next signal change
        //
        
        for (isize j = cycle; j < trigger; j++) {
            
            if (state.ff5 && cnt == 0) {

                // if (pos.v == 0x80) trace(true, "%ld: ff5 AND 0\n", j);
                state.ff2 = false;
                state.ff3 = false;
                state.ff5 = false;
            }
            if (state.ff4 && cnt == 0) {

                // if (pos.v == 0x80) trace(true, "%ld: ff4 AND 0\n", j);
                state.ff5 = true;
                state.ff4 = false;
            }
            
            if (state.ff3) {
                
                // if (pos.v == 0x80) trace(true, "%ld: ff3\n", j);
                if (j < min) min = (int)j;
                if (j > max) max = (int)j;
                
                assert(j >= 0 && j < HPOS_CNT);
                bplEvent[j] = fetch[state.ff5][cnt];
                cnt = (cnt + 1) & 7;
                
            } else {

                assert(j >= 0 && j < HPOS_CNT);
                bplEvent[j] = 0;
            }
        }
        
        //
        // Emulate the next signal change
        //
        
        if (signal >= 0 && signal <= 15) {
            
            computeFetchUnit((u8)signal, fetch);
                                
        } else {
            
            switch (signal) {
                                        
                case SIG_BMAPEN_CLR:
                    
                    bmapen = false;
                    state.ff3 = false;
                    cnt = 0;
                    break;
                    
                case SIG_BMAPEN_SET:
                    
                    bmapen = true;
                    break;
                    
                case SIG_VFLOP_SET:
                    break;
                    
                case SIG_VFLOP_CLR:
                    break;
                    
                case SIG_BPHSTART:
                    
                    // if (pos.v == 0x80) trace(true, "SIG_BPHSTART\n");
                    if (state.ff2) state.ff3 = true;
                    if (!state.ff1) state.ff3 = false;
                    if (!bmapen) state.ff3 = false;
                    break;
                    
                case SIG_BPHSTOP:
                    
                    // if (pos.v == 0x80) trace(true, "SIG_BPHSTOP\n");
                    state.ff4 = true;
                    break;
                    
                case SIG_SHW:
                    
                    // if (pos.v == 0x80) trace(true, "SIG_SHW\n");
                    state.ff2 = true;
                    break;
                    
                case SIG_RHW:
                    
                    // if (pos.v == 0x80) trace(true, "SIG_RHW\n");
                    state.ff4 = true;
                    break;
                    
                default:
                    assert(signal == SIG_NONE);
            }
        }
        
        cycle = trigger;
    }
    
    // Add the End Of Line event
    bplEvent[HPOS_MAX] = BPL_EOL;
                
    // Superimpose the drawing flags
    hires ? updateHiresDrawingFlags() : updateLoresDrawingFlags();
            
    // Update the jump table
    updateBplJumpTable();

    // Write back the new ddf state
    ddf = state;
}

void
Agnus::computeFetchUnit(u8 dmacon, EventID id[2][8])
{
    switch (dmacon) {
            
        case SIG_CON_L7:
        case SIG_CON_L6:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_L4;   id[1][1] = BPL_L4_MOD;
            id[0][2] = BPL_L6;   id[1][2] = BPL_L6_MOD;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_L3;   id[1][5] = BPL_L3_MOD;
            id[0][6] = BPL_L5;   id[1][6] = BPL_L5_MOD;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case SIG_CON_L5:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_L4;   id[1][1] = BPL_L4_MOD;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_L3;   id[1][5] = BPL_L3_MOD;
            id[0][6] = BPL_L5;   id[1][6] = BPL_L5_MOD;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case SIG_CON_L4:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_L4;   id[1][1] = BPL_L4_MOD;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_L3;   id[1][5] = BPL_L3_MOD;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case SIG_CON_L3:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_L3;   id[1][5] = BPL_L3_MOD;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case SIG_CON_L2:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_L2;   id[1][3] = BPL_L2_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = 0;        id[1][5] = 0;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case SIG_CON_L1:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = 0;        id[1][3] = 0;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = 0;        id[1][5] = 0;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_L1;   id[1][7] = BPL_L1_MOD;
            break;

        case SIG_CON_H4:
            
            id[0][0] = BPL_H4;   id[1][0] = BPL_H4_MOD;
            id[0][1] = BPL_H2;   id[1][1] = BPL_H2_MOD;
            id[0][2] = BPL_H3;   id[1][2] = BPL_H3_MOD;
            id[0][3] = BPL_H1;   id[1][3] = BPL_H1_MOD;
            id[0][4] = BPL_H4;   id[1][4] = BPL_H4_MOD;
            id[0][5] = BPL_H2;   id[1][5] = BPL_H2_MOD;
            id[0][6] = BPL_H3;   id[1][6] = BPL_H3_MOD;
            id[0][7] = BPL_H1;   id[1][7] = BPL_H1_MOD;
            break;

        case SIG_CON_H3:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_H2;   id[1][1] = BPL_H2_MOD;
            id[0][2] = BPL_H3;   id[1][2] = BPL_H3_MOD;
            id[0][3] = BPL_H1;   id[1][3] = BPL_H1_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_H2;   id[1][5] = BPL_H2_MOD;
            id[0][6] = BPL_H3;   id[1][6] = BPL_H3_MOD;
            id[0][7] = BPL_H1;   id[1][7] = BPL_H1_MOD;
            break;

        case SIG_CON_H2:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = BPL_H2;   id[1][1] = BPL_H2_MOD;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_H1;   id[1][3] = BPL_H1_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = BPL_H2;   id[1][5] = BPL_H2_MOD;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_H1;   id[1][7] = BPL_H1_MOD;
            break;

        case SIG_CON_H1:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = BPL_H1;   id[1][3] = BPL_H1_MOD;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = 0;        id[1][5] = 0;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = BPL_H1;   id[1][7] = BPL_H1_MOD;
            break;
            
        case SIG_CON_L0:
        case SIG_CON_H0:
        case SIG_CON_H7:
        case SIG_CON_H6:
        case SIG_CON_H5:
            
            id[0][0] = 0;        id[1][0] = 0;
            id[0][1] = 0;        id[1][1] = 0;
            id[0][2] = 0;        id[1][2] = 0;
            id[0][3] = 0;        id[1][3] = 0;
            id[0][4] = 0;        id[1][4] = 0;
            id[0][5] = 0;        id[1][5] = 0;
            id[0][6] = 0;        id[1][6] = 0;
            id[0][7] = 0;        id[1][7] = 0;
            break;
    }
}

void
Agnus::updateDasEvents(u16 dmacon)
{
    assert(dmacon < 64);

    // Allocate slots
    for (isize i = 0; i < 0x38; i++) dasEvent[i] = dasDMA[dmacon][i];
    
    // Update the jump table
    updateDasJumpTable(0x38);
}

void
Agnus::updateBplJumpTable()
{
    u8 next = nextBplEvent[HPOS_MAX];
    
    for (isize i = HPOS_MAX; i >= 0; i--) {
        
        nextBplEvent[i] = next;
        if (bplEvent[i]) next = (i8)i;
    }
}

void
Agnus::updateDasJumpTable(i16 end)
{
    assert(end <= HPOS_MAX);

    u8 next = nextDasEvent[end];
    
    for (isize i = end; i >= 0; i--) {
        
        nextDasEvent[i] = next;
        if (dasEvent[i]) next = (i8)i;
    }
}

void
Agnus::updateHiresDrawingFlags()
{
    assert(scrollHiresEven < 8);
    assert(scrollHiresOdd  < 8);
    
    for (isize i = scrollHiresOdd; i < HPOS_CNT; i += 4)
        bplEvent[i] = (EventID)(bplEvent[i] | 1);
    
    for (isize i = scrollHiresEven; i < HPOS_CNT; i += 4)
        bplEvent[i] = (EventID)(bplEvent[i] | 2);
}

void
Agnus::updateLoresDrawingFlags()
{
    assert(scrollLoresEven < 8);
    assert(scrollLoresOdd  < 8);
    
    for (isize i = scrollLoresOdd; i < HPOS_CNT; i += 8)
        bplEvent[i] = (EventID)(bplEvent[i] | 1);
    
    for (isize i = scrollLoresEven; i < HPOS_CNT; i += 8)
        bplEvent[i] = (EventID)(bplEvent[i] | 2);
}

template <BusOwner owner> bool
Agnus::allocateBus()
{
    // Deny if the bus has been allocated already
    if (busOwner[pos.h] != BUS_NONE) return false;
    
    if constexpr (owner == BUS_COPPER) {
        
        // Assign bus to the Copper
        busOwner[pos.h] = BUS_COPPER;
        
        return true;
    }
    if constexpr (owner == BUS_BLITTER) {
        
        // Deny if Blitter DMA is off
        if (!bltdma()) return false;
        
        // Deny if the CPU has precedence
        if (bls && !bltpri()) return false;
        
        // Assign the bus to the Blitter
        busOwner[pos.h] = BUS_BLITTER;
        
        return true;
    }
}

u16
Agnus::doDiskDmaRead()
{
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (dskpt);
    dskpt += 2;

    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = result;
    stats.usage[BUS_DISK]++;

    return result;
}

template <int channel> u16
Agnus::doAudioDmaRead()
{
    constexpr BusOwner owner = BusOwner(BUS_AUD0 + channel);
    
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (audpt[channel]);
    audpt[channel] += 2;

    busOwner[pos.h] = owner;
    busValue[pos.h] = result;
    stats.usage[owner]++;

    return result;
}

template <int bitplane> u16
Agnus::doBitplaneDmaRead()
{
    assert(bitplane >= 0 && bitplane <= 5);
    constexpr BusOwner owner = BusOwner(BUS_BPL1 + bitplane);
    
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (bplpt[bitplane]);
    bplpt[bitplane] += 2;

    busOwner[pos.h] = owner;
    busValue[pos.h] = result;
    stats.usage[owner]++;

    return result;
}

template <int channel> u16
Agnus::doSpriteDmaRead()
{
    assert(channel >= 0 && channel <= 7);
    constexpr BusOwner owner = BusOwner(BUS_SPRITE0 + channel);

    u16 result = mem.peek16 <ACCESSOR_AGNUS> (sprpt[channel]);
    sprpt[channel] += 2;

    busOwner[pos.h] = owner;
    busValue[pos.h] = result;
    stats.usage[owner]++;

    return result;
}

u16
Agnus::doCopperDmaRead(u32 addr)
{
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (addr);

    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = result;
    stats.usage[BUS_COPPER]++;

    return result;
}

u16
Agnus::doBlitterDmaRead(u32 addr)
{
    // Assure that the Blitter owns the bus when this function is called
    assert(busOwner[pos.h] == BUS_BLITTER);

    u16 result = mem.peek16 <ACCESSOR_AGNUS> (addr);

    busOwner[pos.h] = BUS_BLITTER;
    busValue[pos.h] = result;
    stats.usage[BUS_BLITTER]++;

    return result;
}

void
Agnus::doDiskDmaWrite(u16 value)
{
    mem.poke16 <ACCESSOR_AGNUS> (dskpt, value);
    dskpt += 2;

    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = value;
    stats.usage[BUS_DISK]++;
}

void
Agnus::doCopperDmaWrite(u32 addr, u16 value)
{
    mem.pokeCustom16<ACCESSOR_AGNUS>(addr, value);
    
    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = value;
    stats.usage[BUS_COPPER]++;
}

void
Agnus::doBlitterDmaWrite(u32 addr, u16 value)
{
    mem.poke16 <ACCESSOR_AGNUS> (addr, value);

    assert(busOwner[pos.h] == BUS_BLITTER); // Bus is already allocated
    busValue[pos.h] = value;
    stats.usage[BUS_BLITTER]++;
}

template u16 Agnus::doAudioDmaRead<0>();
template u16 Agnus::doAudioDmaRead<1>();
template u16 Agnus::doAudioDmaRead<2>();
template u16 Agnus::doAudioDmaRead<3>();

template u16 Agnus::doBitplaneDmaRead<0>();
template u16 Agnus::doBitplaneDmaRead<1>();
template u16 Agnus::doBitplaneDmaRead<2>();
template u16 Agnus::doBitplaneDmaRead<3>();
template u16 Agnus::doBitplaneDmaRead<4>();
template u16 Agnus::doBitplaneDmaRead<5>();

template u16 Agnus::doSpriteDmaRead<0>();
template u16 Agnus::doSpriteDmaRead<1>();
template u16 Agnus::doSpriteDmaRead<2>();
template u16 Agnus::doSpriteDmaRead<3>();
template u16 Agnus::doSpriteDmaRead<4>();
template u16 Agnus::doSpriteDmaRead<5>();
template u16 Agnus::doSpriteDmaRead<6>();
template u16 Agnus::doSpriteDmaRead<7>();

template bool Agnus::allocateBus<BUS_COPPER>();
template bool Agnus::allocateBus<BUS_BLITTER>();

template bool Agnus::busIsFree<BUS_COPPER>();
template bool Agnus::busIsFree<BUS_BLITTER>();
