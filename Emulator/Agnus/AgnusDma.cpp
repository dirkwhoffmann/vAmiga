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

template <> bool Agnus::auddma<0>(u16 v) { return (v & DMAEN) && (v & AUD0EN); }
template <> bool Agnus::auddma<1>(u16 v) { return (v & DMAEN) && (v & AUD1EN); }
template <> bool Agnus::auddma<2>(u16 v) { return (v & DMAEN) && (v & AUD2EN); }
template <> bool Agnus::auddma<3>(u16 v) { return (v & DMAEN) && (v & AUD3EN); }

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
    for (isize i = 0; i < HPOS_MAX; i++) sequencer.bplEvent[i] = EVENT_NONE;
    for (isize i = 0; i < HPOS_MAX; i++) sequencer.nextBplEvent[i] = HPOS_MAX;
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
        
    sigRecorder.insert(0, SIG_CON_L0 | bplcon0 >> 12);
    sigRecorder.insert(0x18, SIG_SHW);
    sigRecorder.insert(ddfstrt, SIG_BPHSTART);
    sigRecorder.insert(ddfstop, SIG_BPHSTOP);
    sigRecorder.insert(0xD8, SIG_RHW);
    sigRecorder.insert(HPOS_CNT, SIG_NONE);
    
    computeBplEvents(sigRecorder);
}

void
Agnus::computeBplEvents(const SigRecorder &sr)
{
    auto state = ddfInitial;
    auto bmapen = dmaconInitial & (DMAEN | BPLEN);

    isize cnt = 0;
                
    // trace(true, "computeBplEvents\n");
    // dump(dump::Signals);
    // if (pos.v == 48) dump(dump::Dma);
    
    // The fetch unit layout
    EventID fetch[2][8];
    computeFetchUnit((u8)(bplcon0Initial >> 12), fetch);
    isize mask = (bplcon0Initial & 0x8000) ? 0b11 : 0b111;

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
            if ((j & mask) == (scrollOdd & mask))  id = (EventID)(id | 1);
            if ((j & mask) == (scrollEven & mask)) id = (EventID)(id | 2);
            
            sequencer.bplEvent[j] = id;
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
    sequencer.bplEvent[HPOS_MAX] = BPL_EOL;
                            
    // Update the jump table
    updateBplJumpTable();

    // Write back the new ddf state
    ddf = state;

    // Check if we need to recompute all event in the next scanline
    if (state != ddfInitial) {
        hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
    }
}

void
Agnus::computeFetchUnit(u8 dmacon, EventID id[2][8])
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
Agnus::updateDasEvents(u16 dmacon)
{
    assert(dmacon < 64);

    // Allocate slots
    for (isize i = 0; i < 0x38; i++) sequencer.dasEvent[i] = sequencer.dasDMA[dmacon][i];
    
    // Update the jump table
    updateDasJumpTable(0x38);
}

void
Agnus::updateBplJumpTable()
{
    u8 next = sequencer.nextBplEvent[HPOS_MAX];
    
    for (isize i = HPOS_MAX; i >= 0; i--) {
        
        sequencer.nextBplEvent[i] = next;
        if (sequencer.bplEvent[i]) next = (i8)i;
    }
}

void
Agnus::updateDasJumpTable(i16 end)
{
    assert(end <= HPOS_MAX);

    u8 next = sequencer.nextDasEvent[end];
    
    for (isize i = end; i >= 0; i--) {
        
        sequencer.nextDasEvent[i] = next;
        if (sequencer.dasEvent[i]) next = (i8)i;
    }
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
