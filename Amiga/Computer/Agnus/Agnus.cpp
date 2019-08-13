// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Agnus::Agnus()
{
    setDescription("Agnus");
    
    subComponents = vector<HardwareComponent *> {
        
        &copper,
        &blitter,
        &dmaDebugger
    };
    
    initLookupTables();
}

void
Agnus::initLookupTables()
{
    initLoresBplEventTable();
    initHiresBplEventTable();
    initDASTables();
}

void
Agnus::initLoresBplEventTable()
{
    memset(bitplaneDMA[0], 0, sizeof(bitplaneDMA[0]));
    memset(fetchUnitNr[0], 0, sizeof(fetchUnitNr[0]));

    for (int bpu = 0; bpu < 7; bpu++) {

        EventID *p = &bitplaneDMA[0][bpu][0];

        // Iterate through all 22 fetch units
        for (int i = 0; i <= 0xD8; i += 8, p += 8) {

            switch(bpu) {
                case 6: p[2] = BPL_L6;
                case 5: p[6] = BPL_L5;
                case 4: p[1] = BPL_L4;
                case 3: p[5] = BPL_L3;
                case 2: p[3] = BPL_L2;
                case 1: p[7] = BPL_L1;
            }
        }
    }

    for (int i = 0; i <= 0xD8; i++) {
        fetchUnitNr[0][i] = i % 8;
    }
}

void
Agnus::initHiresBplEventTable()
{
    memset(bitplaneDMA[1], 0, sizeof(bitplaneDMA[1]));
    memset(fetchUnitNr[1], 0, sizeof(fetchUnitNr[1]));

    for (int bpu = 0; bpu < 7; bpu++) {

        EventID *p = &bitplaneDMA[1][bpu][0];

        for (int i = 0; i <= 0xD8; i += 8, p += 8) {

            switch(bpu) {
                case 6:
                case 5:
                case 4: p[0] = p[4] = BPL_H4;
                case 3: p[2] = p[6] = BPL_H3;
                case 2: p[1] = p[5] = BPL_H2;
                case 1: p[3] = p[7] = BPL_H1;
            }
        }
    }

    for (int i = 0; i <= 0xD8; i++) {
        fetchUnitNr[0][i] = i % 4;
    }
}

void
Agnus::initDASTables()
{
    EventID table[0x34];

    // Start from scratch
    memset(table, EVENT_NONE, sizeof(table));
    memset(nextDASEvent, 0, sizeof(nextDASEvent));
    memset(nextDASDelay, 0, sizeof(nextDASDelay));

    // Iterate through all possible DMA enable / disable combinations
    for (unsigned dma = 0; dma < 64; dma++) {

        // Setup the time slot allocation table for this combination
        table[0x01] = DAS_REFRESH;
        table[0x07] = dma ? DAS_D0 : EVENT_NONE; // enable if master bit is set
        table[0x09] = dma ? DAS_D1 : EVENT_NONE; // enable if master bit is set
        table[0x0B] = dma ? DAS_D2 : EVENT_NONE; // enable if master bit is set
        table[0x0D] = (dma & AU0EN) ? DAS_A0 : EVENT_NONE;
        table[0x0F] = (dma & AU1EN) ? DAS_A1 : EVENT_NONE;
        table[0x11] = (dma & AU2EN) ? DAS_A2 : EVENT_NONE;
        table[0x13] = (dma & AU3EN) ? DAS_A3 : EVENT_NONE;
        table[0x15] = (dma & SPREN) ? DAS_S0_1 : EVENT_NONE;
        table[0x17] = (dma & SPREN) ? DAS_S0_2 : EVENT_NONE;
        table[0x19] = (dma & SPREN) ? DAS_S1_1 : EVENT_NONE;
        table[0x1B] = (dma & SPREN) ? DAS_S1_2 : EVENT_NONE;
        table[0x1D] = (dma & SPREN) ? DAS_S2_1 : EVENT_NONE;
        table[0x1F] = (dma & SPREN) ? DAS_S2_2 : EVENT_NONE;
        table[0x21] = (dma & SPREN) ? DAS_S3_1 : EVENT_NONE;
        table[0x23] = (dma & SPREN) ? DAS_S3_2 : EVENT_NONE;
        table[0x25] = (dma & SPREN) ? DAS_S4_1 : EVENT_NONE;
        table[0x27] = (dma & SPREN) ? DAS_S4_2 : EVENT_NONE;
        table[0x29] = (dma & SPREN) ? DAS_S5_1 : EVENT_NONE;
        table[0x2B] = (dma & SPREN) ? DAS_S5_2 : EVENT_NONE;
        table[0x2D] = (dma & SPREN) ? DAS_S6_1 : EVENT_NONE;
        table[0x2F] = (dma & SPREN) ? DAS_S6_2 : EVENT_NONE;
        table[0x31] = (dma & SPREN) ? DAS_S7_1 : EVENT_NONE;
        table[0x33] = (dma & SPREN) ? DAS_S7_2 : EVENT_NONE;

        // Determine the successor event for any RAS event
        for (unsigned id = 1; id < DAS_EVENT_COUNT; id++) {

            // Determine the DMA cycle of this event
            int16_t cycle = DASEventCycle((EventID)id);
            int16_t next = cycle;

            // Find the next event starting from that position
            do { next = (next + 1) % 0x34; } while (table[next] == EVENT_NONE);

            // Setup the table entries
            nextDASEvent[id][dma] = table[next];
            nextDASDelay[id][dma] = next - cycle;
            if (next <= cycle) nextDASDelay[id][dma] += DMACyclesPerLine();
        }
    }

    // Dump the table (for debugging)
    /*
    unsigned dma = 0b010000;
    for (unsigned id = 0; id < DAS_EVENT_CNT; id++) {
        if (nextDASEvent[id][dma] != EVENT_NONE) {
            plainmsg("Event %d -> Event %d in %d DMA cycles\n",
                     id, nextDASEvent[id][dma], nextDASDelay[id][dma]);
        }
    }
    */
}

void
Agnus::_initialize()
{
    ciaA = &amiga->ciaA;
    ciaB = &amiga->ciaB;
    mem = &amiga->mem;
    denise = &amiga->denise;
    paula = &amiga->paula;
}

void
Agnus::_powerOn()
{
}

void Agnus::_reset()
{
    RESET_SNAPSHOT_ITEMS

    // Start with a long frame
    lof = 1;
    frameInfo.numLines = 313;

    // Initialize lookup tables
    clearDMAEventTable();

    // Clear the event table
    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        slot[i].triggerCycle = NEVER;
        slot[i].id = (EventID)0;
        slot[i].data = 0;
    }

    // Initialize the SEC_SLOT
    scheduleAbs<SEC_SLOT>(NEVER, SEC_TRIGGER);

    // Schedule the first CIA A and CIA B events
    scheduleAbs<CIAA_SLOT>(CIA_CYCLES(1), CIA_EXECUTE);
    scheduleAbs<CIAB_SLOT>(CIA_CYCLES(1), CIA_EXECUTE);

    // Schedule first DAS event
    scheduleAbs<DAS_SLOT>(DMA_CYCLES(1), DAS_REFRESH);

    // Schedule first SYNC event
    scheduleAbs<SYNC_SLOT>(DMA_CYCLES(HPOS_MAX), SYNC_EOL);
}

void
Agnus::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.dmacon  = dmacon;
    info.diwstrt = diwstrt;
    info.diwstop = diwstop;
    info.ddfstrt = ddfstrt;
    info.ddfstop = ddfstop;
    
    info.bpl1mod = bpl1mod;
    info.bpl2mod = bpl2mod;
    info.numBpls = activeBitplanes;
    
    info.dskpt   = dskpt;
    for (unsigned i = 0; i < 4; i++) info.audlc[i] = audlc[i];
    for (unsigned i = 0; i < 6; i++) info.bplpt[i] = bplpt[i];
    for (unsigned i = 0; i < 8; i++) info.sprpt[i] = sprpt[i];

    pthread_mutex_unlock(&lock);
}

void
Agnus::_dump()
{
    plainmsg("  dskpt = %X\n", dskpt);
    for (unsigned i = 0; i < 4; i++) plainmsg("audlc[%d] = %X\n", i, audlc[i]);
    for (unsigned i = 0; i < 6; i++) plainmsg("bplpt[%d] = %X\n", i, bplpt[i]);
    for (unsigned i = 0; i < 8; i++) plainmsg("bplpt[%d] = %X\n", i, sprpt[i]);
    
    plainmsg("  hstrt : %d\n", diwHstrt);
    plainmsg("  hstop : %d\n", diwHstop);
    plainmsg("  vstrt : %d\n", diwVstrt);
    plainmsg("  vstop : %d\n", diwVstop);

    plainmsg("\nDMA time slot allocation:\n\n");

    dumpEvents();
    dumpDMAEventTable();
}

DMAInfo
Agnus::getInfo()
{
    DMAInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

Cycle
Agnus::cyclesInCurrentFrame()
{
    // TODO: Distinguish between long and short frames
    return DMA_CYCLES(313 * DMACyclesPerLine());
}

bool
Agnus::belongsToCurrentFrame(Cycle cycle)
{
    Cycle diff = cycle - startOfCurrentFrame();
    return diff >= 0 && diff < cyclesInCurrentFrame();
}

Cycle
Agnus::startOfCurrentFrame()
{
    return clock - DMA_CYCLES(pos.v * DMACyclesPerLine() + pos.h);
}

Cycle
Agnus::startOfNextFrame()
{
    return startOfCurrentFrame() + cyclesInCurrentFrame();
}

bool
Agnus::inBplDmaArea() {

    return
    diwVFlop                              // Vertical DIW flipflop
    && pos.v >= 26                     // Outside VBLANK area
    && pos.v < frameInfo.numLines - 1  // Not in last line of frame
    && activeBitplanes                 // At least one bitplane enabled
    && bplDMA();                       // Bitplane DMA enabled
}

Cycle
Agnus::beamToCycle(Beam beam)
{
    return startOfCurrentFrame() + DMA_CYCLES(beam.v * DMACyclesPerLine() + beam.h);
}

Beam
Agnus::cycleToBeam(Cycle cycle)
{
    Beam result;

    Cycle diff = AS_DMA_CYCLES(cycle - startOfCurrentFrame());
    assert(diff >= 0);

    result.v = diff / DMACyclesPerLine();
    result.h = diff % DMACyclesPerLine();
    return result;
}

Beam
Agnus::addToBeam(Beam beam, Cycle cycles)
{
    Beam result;

    Cycle cycle = beam.v * DMACyclesPerLine() + beam.h + cycles;
    result.v = cycle / DMACyclesPerLine();
    result.h = cycle % DMACyclesPerLine();

    return result;
}

DMACycle
Agnus::DMACyclesInCurrentFrame()
{
    // TODO: Distinguish between short frames and long frames
    /*
    if (screenBuffer2) {
        return 312 * cyclesPerLine();
    } else {
        return 313 * cyclesPerLine();
    }
    */
    return 313 * DMACyclesPerLine();
}

Cycle
Agnus::beamDiff(int16_t vStart, int16_t hStart, int16_t vEnd, int16_t hEnd)
{
    // We assume that the function is called with a valid horizontal position
    assert(hEnd <= HPOS_MAX);
    
    // Bail out if the end position is unreachable
    if (vEnd > 312) return NEVER;
    
    // Compute vertical and horizontal difference
    int32_t vDiff  = vEnd - vStart;
    int32_t hDiff  = hEnd - hStart;
    debug("vdiff: %d hdiff: %d\n", vDiff, hDiff);

    // In PAL mode, all lines have the same length (227 color clocks)
    return DMA_CYCLES(vDiff * 227 + hDiff);
}

bool
Agnus::copperCanRun()
{
    // Deny access if Copper DMA is disabled
    if (!copDMA()) return false;

    // Deny access if the bus is already in use
    if (busOwner[pos.h] != BUS_NONE) {
        debug(COP_DEBUG, "Copper blocked (bus busy)\n");
        return false;
    }

    return true;
}

bool
Agnus::copperCanDoDMA()
{
    // Deny access in cycle $E0
    if (unlikely(pos.h == 0xE0)) {
        debug(COP_DEBUG, "Copper blocked (at $E0)\n");
        return false;
    }

    return copperCanRun();
}

uint16_t
Agnus::doDiskDMA()
{
    uint16_t result = mem->peekChip16(dskpt);
    INC_DMAPTR(dskpt);

    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = result;

    return result;
}

void
Agnus::doDiskDMA(uint16_t value)
{
    mem->pokeChip16(dskpt, value);
    INC_DMAPTR(dskpt);

    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = value;
}

uint16_t
Agnus::doAudioDMA(int channel)
{
    uint16_t result = mem->peekChip16(audlc[channel]);
    INC_DMAPTR(audlc[channel]);

    // We have to fake the horizontal position here, because this function
    // is not yet executed at the correct DMA cycle.
    int hpos = 0xD + (2 * channel);

    busOwner[hpos] = BUS_AUDIO;
    busValue[hpos] = result;

    return result;
}

template <int channel> uint16_t
Agnus::doSpriteDMA()
{
    uint16_t result = mem->peekChip16(sprpt[channel]);
    INC_DMAPTR(sprpt[channel]);

    busOwner[pos.h] = BUS_SPRITE;
    busValue[pos.h] = result;

    return result;
}

uint16_t
Agnus::doSpriteDMA(int channel)
{
    uint16_t result = mem->peekChip16(sprpt[channel]);
    INC_DMAPTR(sprpt[channel]);

    busOwner[pos.h] = BUS_SPRITE;
    busValue[pos.h] = result;

    return result; 
}

template <int bitplane> uint16_t
Agnus::doBitplaneDMA()
{
    uint16_t result = mem->peekChip16(bplpt[bitplane]);
    INC_DMAPTR(bplpt[bitplane]);

    busOwner[pos.h] = BUS_BITPLANE;
    busValue[pos.h] = result;

    return result;
}

uint16_t
Agnus::copperRead(uint32_t addr)
{
    uint16_t result = mem->peek16(addr);

    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = result;

    return result;
}

void
Agnus::copperWrite(uint32_t addr, uint16_t value)
{
    mem->pokeCustom16<POKE_COPPER>(addr, value);

    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = value;
}

void
Agnus::clearDMAEventTable()
{
    // Clear the event table
    memset(dmaEvent, 0, sizeof(dmaEvent));
    
    // Clear the jump table
    memset(nextDmaEvent, 0, sizeof(nextDmaEvent));
}

void
Agnus::allocateBplSlots(int bpu, bool hires, int first, int last)
{
    assert(bpu >= 0 && bpu <= 6);
    assert(hires == 0 || hires == 1);

    if (hires) {
        for (unsigned i = first; i <= last; i++) {
            dmaEvent[i] = inHiresDmaArea(i) ? bitplaneDMA[1][bpu][i] : EVENT_NONE;
        }
    } else {
        for (unsigned i = first; i <= last; i++) {
            dmaEvent[i] = inLoresDmaArea(i) ? bitplaneDMA[0][bpu][i] : EVENT_NONE;
        }
    }

    updateJumpTable();
}

void
Agnus::allocateBplSlots(int bpu, bool hires, int first)
{
    allocateBplSlots(bpu, hires, first, HPOS_MAX);
}

void
Agnus::switchBitplaneDmaOn()
{
    int16_t start;
    int16_t stop;

    bool hires = denise->hires();

    // Determine the range that is covered by fetch units
    if (hires) {

        start = dmaStrtHires;
        stop = dmaStopHires;
        assert((stop - start) % 4 == 0);

    } else {

        start = dmaStrtLores;
        stop = dmaStopLores;
        assert((stop - start) % 8 == 0);
    }

    debug(BPL_DEBUG, "switchBitplaneDmaOn()\n");
    debug(BPL_DEBUG, "hires = %d start = %d stop = %d\n", hires, start, stop);

    assert(start >= 0 && start <= HPOS_MAX);
    assert(stop >= 0 && stop <= HPOS_MAX);

    // Wipe out all events outside the fetch unit window
    for (int i = 0; i < start; i++) dmaEvent[i] = EVENT_NONE;
    for (int i = stop; i < HPOS_MAX; i++) dmaEvent[i] = EVENT_NONE;

    // Copy events from the proper lookup table
    for (int i = start; i < stop; i++) {
        dmaEvent[i] = bitplaneDMA[hires][activeBitplanes][i];
    }

    // Link everything together
    updateJumpTable();

    // debug("EventTable:\n");
    // dumpDMAEventTable();
}


void
Agnus::switchBitplaneDmaOff()
{
    // Quick-exit if the event table is free of any bitplane DMA events
    // TODO: We need a reliable quick-exit condition
    // The old variable (dmaFirstBpl1Event) has been removed.

    debug(BPL_DEBUG, "switchBitplaneDmaOff: \n");

    // Clear the event table
    for (int i = 0x18; i < HPOS_CNT; dmaEvent[i++] = (EventID)0);
    updateJumpTable();

    // Clear any pending event in the BPL slot
    cancel(BPL_SLOT);
}

void
Agnus::updateBitplaneDma()
{
    debug(BPL_DEBUG, "updateBitplaneDma()\n");

    // Determine if bitplane DMA has to be on or off
    bool bplDma = inBplDmaArea();

    // Update the event table accordingly
    bplDma ? switchBitplaneDmaOn() : switchBitplaneDmaOff();
}

/*
void
Agnus::computeBplVstrtVstop()
{
    bplVstrt = MAX(diwVstrt, 26); // 0 .. 25 is VBLANK area
    bplVstop = MIN(diwVstop, frameInfo.numLines - 1);
}
*/

void
Agnus::updateJumpTable(int16_t to)
{
    assert(to <= HPOS_MAX);
    assert(dmaEvent[HPOS_MAX] == 0);
    
    // Build the jump table
    uint8_t next = nextDmaEvent[to];
    for (int i = to; i >= 0; i--) {
        nextDmaEvent[i] = next;
        if (dmaEvent[i]) next = i;
    }
}

bool
Agnus::isLastLx(int16_t dmaCycle)
{
    return (pos.h >= dmaStopLores - 8);
}

bool
Agnus::isLastHx(int16_t dmaCycle)
{
    return (pos.h >= dmaStopHires - 4);
}

bool
Agnus::inLastFetchUnit(int16_t dmaCycle)
{
    return denise->hires() ? isLastHx(dmaCycle) : isLastLx(dmaCycle);
}


void
Agnus::dumpDMAEventTable(int from, int to)
{
    char r1[256], r2[256], r3[256], r4[256];
    int i;

    for (i = 0; i <= (to - from); i++) {
        
        int digit1 = (from + i) / 16;
        int digit2 = (from + i) % 16;
        
        r1[i] = (digit1 < 10) ? digit1 + '0' : (digit1 - 10) + 'A';
        r2[i] = (digit2 < 10) ? digit2 + '0' : (digit2 - 10) + 'A';
        
        switch(dmaEvent[i + from]) {
            case EVENT_NONE:   r3[i] = '.'; r4[i] = '.'; break;
            case BPL_L1:       r3[i] = 'L'; r4[i] = '1'; break;
            case BPL_L2:       r3[i] = 'L'; r4[i] = '2'; break;
            case BPL_L3:       r3[i] = 'L'; r4[i] = '3'; break;
            case BPL_L4:       r3[i] = 'L'; r4[i] = '4'; break;
            case BPL_L5:       r3[i] = 'L'; r4[i] = '5'; break;
            case BPL_L6:       r3[i] = 'L'; r4[i] = '6'; break;
            case BPL_H1:       r3[i] = 'H'; r4[i] = '1'; break;
            case BPL_H2:       r3[i] = 'H'; r4[i] = '2'; break;
            case BPL_H3:       r3[i] = 'H'; r4[i] = '3'; break;
            case BPL_H4:       r3[i] = 'H'; r4[i] = '4'; break;
            default:           assert(false);
        }
    }
    r1[i] = r2[i] = r3[i] = r4[i] = 0;
    plainmsg("%s\n", r1);
    plainmsg("%s\n", r2);
    plainmsg("%s\n", r3);
    plainmsg("%s\n", r4);
}

void
Agnus::dumpDMAEventTable()
{
    // Dump the event table
    plainmsg("Event table:\n\n");
    plainmsg("ddfstrt = %X dffstop = %X\n",
             ddfstrt, ddfstop);
    plainmsg("dmaStrtLores = %X dmaStrtHires = %X\n",
             dmaStrtLores, dmaStrtHires);
    plainmsg("dmaStopLores = %X dmaStopHires = %X\n",
             dmaStopLores, dmaStopHires);

    dumpDMAEventTable(0x00, 0x4F);
    dumpDMAEventTable(0x50, 0x9F);
    dumpDMAEventTable(0xA0, 0xE2);

    // Dump the jump table
    plainmsg("\nJump table:\n\n");
    int i = nextDmaEvent[0];
    plainmsg("%d", i);
    while (i) {
        i = nextDmaEvent[i];
        plainmsg(" -> %X", i);
    }
    plainmsg("\n");

    // for (i = 0; nextDmaEvent[i]; i++) { plainmsg("%d, ", nextDmaEvent[i]); }
    plainmsg("\n");

}

uint16_t
Agnus::peekDMACONR()
{
    uint16_t result = dmacon;

    assert((result & ((1 << 14) | (1 << 13))) == 0);
    
    if (blitter.bbusy) result |= (1 << 14);
    if (blitter.bzero) result |= (1 << 13);

    debug(2, "peekDMACONR: %X\n", result);
    return result;
}

void
Agnus::pokeDMACON(uint16_t value)
{
    debug(DMA_DEBUG, "pokeDMACON(%X)\n", value);

    // Compute the real value (Bit 15 determines if bits are set or cleared)
    if (value & 0x8000) {
        value = (dmacon | value) & 0x07FF;
    } else {
        value = (dmacon & ~value) & 0x07FF;
    }

    if (dmacon != value) {

        pokeDMACON(dmacon, value);
        dmacon = value;
    }
}

void
Agnus::pokeDMACON(uint16_t oldValue, uint16_t newValue)
{
    assert(oldValue != newValue);

    bool oldDMAEN = (oldValue & DMAEN);
    bool oldBPLEN = (oldValue & BPLEN) && oldDMAEN;
    bool oldCOPEN = (oldValue & COPEN) && oldDMAEN;
    bool oldBLTEN = (oldValue & BLTEN) && oldDMAEN;
    bool oldSPREN = (oldValue & SPREN) && oldDMAEN;
    // bool oldDSKEN = (oldValue & DSKEN) && oldDMAEN;
    bool oldAU0EN = (oldValue & AU0EN) && oldDMAEN;
    bool oldAU1EN = (oldValue & AU1EN) && oldDMAEN;
    bool oldAU2EN = (oldValue & AU2EN) && oldDMAEN;
    bool oldAU3EN = (oldValue & AU3EN) && oldDMAEN;

    bool newDMAEN = (newValue & DMAEN);
    bool newBPLEN = (newValue & BPLEN) && newDMAEN;
    bool newCOPEN = (newValue & COPEN) && newDMAEN;
    bool newBLTEN = (newValue & BLTEN) && newDMAEN;
    bool newSPREN = (newValue & SPREN) && newDMAEN;
    // bool newDSKEN = (newValue & DSKEN) && newDMAEN;
    bool newAU0EN = (newValue & AU0EN) && newDMAEN;
    bool newAU1EN = (newValue & AU1EN) && newDMAEN;
    bool newAU2EN = (newValue & AU2EN) && newDMAEN;
    bool newAU3EN = (newValue & AU3EN) && newDMAEN;

    // Inform the delegates
    blitter.pokeDMACON(oldValue, newValue);

    // Bitplane DMA
    if (oldBPLEN ^ newBPLEN) {

        // Update the DMA allocation table in the next rasterline
        hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;

        if (newBPLEN) {

            // Bitplane DMA is switched off

            // Check if the current line is affected by the change
            if (pos.h + 2 < ddfstrtReached) {

                bool hires = Denise::hires(bplcon0);
                // We have to assign dmacon here, because if we don't, the
                // the call to inBplDmaArea() gives us the wrong result.
                // This is very ugly. Think about a better way to judge if
                // DMA should be on or off
                dmacon = newValue;
                allocateBplSlots(inBplDmaArea() ? activeBitplanes : 0, hires, pos.h + 2);
                scheduleNextBplEvent();
            }

        } else {

            // Bitplane DMA is switched off
            allocateBplSlots(0, 0, pos.h + 2);
            cancel(BPL_SLOT);
        }

        // Let Denise know about the change
        denise->pokeDMACON(oldValue, newValue);
    }
    
    // Copper DMA
    if (oldCOPEN ^ newCOPEN) {
        
        if (newCOPEN) {
            
            // Copper DMA on
            debug(DMA_DEBUG, "Copper DMA switched on\n");
            
            // Determine trigger cycle for the first Copper event
            // (the next even DMA cycle)
            Cycle trigger = (clock + 15) & ~15;
            
            scheduleAbs<COP_SLOT>(trigger, COP_FETCH);
            
        } else {
            
            // Copper DMA off
            debug(DMA_DEBUG, "Copper DMA switched off\n");
            cancel<COP_SLOT>();
        }
    }
    
    // Blitter DMA
    if (oldBLTEN ^ newBLTEN) {
        
        if (newBLTEN) {
            // Blitter DMA on
            debug(DMA_DEBUG, "Blitter DMA switched on\n");
            // amiga->agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_EXECUTE);
    
        } else {
            
            // Blitter DMA off
            debug(DMA_DEBUG, "Blitter DMA switched off\n");
            cancel<BLT_SLOT>();
        }
    }
    
    // Sprite DMA
    if (oldSPREN ^ newSPREN) {

        /*
        if (newSPREN) {
            // Sprite DMA on
            debug(DMA_DEBUG, "Sprite DMA switched on\n");
            
        } else {
            
            // Sprite DMA off
            debug(DMA_DEBUG, "Sprite DMA switched off\n");
        }
        */
    }
    
    // Disk DMA (only the master bit is checked)
    // if (oldDSKEN ^ newDSKEN) {
    if (oldDMAEN ^ newDMAEN) {

        /*
        if (newDMAEN) {
            
            // Disk DMA on
            debug(DMA_DEBUG, "Disk DMA switched on\n");
            
        } else {
            
            // Disk DMA off
            debug(DMA_DEBUG, "Disk DMA switched off\n");
        }
        */
    }
    
    // Audio DMA
    if (oldAU0EN ^ newAU0EN) {
        
        if (newAU0EN) {
            
            // debug("Audio 0 DMA switched on\n");
            paula->audioUnit.enableDMA(0);
            
        } else {
            
            // debug("Audio 0 DMA switched off\n");
            paula->audioUnit.disableDMA(0);
        }
    }
    
    if (oldAU1EN ^ newAU1EN) {
        
        if (newAU1EN) {
            
            // debug("Audio 1 DMA switched on\n");
            paula->audioUnit.enableDMA(1);
            
        } else {
            
            // debug("Audio 1 DMA switched off\n");
            paula->audioUnit.disableDMA(1);
        }
    }
    
    if (oldAU2EN ^ newAU2EN) {
        
        if (newAU2EN) {
            
            // debug("Audio 2 DMA switched on\n");
            paula->audioUnit.enableDMA(2);
            
        } else {
            
            // debug("Audio 2 DMA switched off\n");
            paula->audioUnit.disableDMA(2);
        }
    }
    
    if (oldAU3EN ^ newAU3EN) {
        
        if (newAU3EN) {
            
            // debug("Audio 3 DMA switched on\n");
            paula->audioUnit.enableDMA(3);
            
        } else {
            
            // debug("Audio 3 DMA switched off\n");
            paula->audioUnit.disableDMA(3);
        }
    }
}

void
Agnus::pokeDSKPTH(uint16_t value)
{
    dskpt = REPLACE_HI_WORD(dskpt, value & 0x7);

    debug(DSK_DEBUG, "pokeDSKPTH(%X): dskpt = %X\n", value, dskpt);
}

void
Agnus::pokeDSKPTL(uint16_t value)
{
    assert(IS_EVEN(value));
    dskpt = REPLACE_LO_WORD(dskpt, value & 0xFFFE);

    debug(DSK_DEBUG, "pokeDSKPTL(%X): dskpt = %X\n", value, dskpt);
}

uint16_t
Agnus::peekVHPOSR()
{
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1
    uint16_t result = BEAM(pos.v, pos.h) & 0xFFFF;

    // debug(BPL_DEBUG, "peekVHPOSR() = %X\n", result);
    return result;
}

void
Agnus::pokeVHPOS(uint16_t value)
{
    debug(BPL_DEBUG, "pokeVHPOS(%X)\n", value);
    // Don't know what to do here ...
}

uint16_t
Agnus::peekVPOSR()
{
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // LF -- -- -- -- -- -- -- -- -- -- -- -- -- -- V8
    uint16_t result = (pos.v >> 8) | (isLongFrame() ? 0x8000 : 0);
    assert((result & 0x7FFE) == 0);

    debug(BPL_DEBUG, "peekVPOSR() = %X\n", result);
    return result;

}

void
Agnus::pokeVPOS(uint16_t value)
{
    // Don't know what to do here ...
}

template <PokeSource s> void
Agnus::pokeDIWSTRT(uint16_t value)
{
    debug(DIW_DEBUG, "pokeDIWSTRT<%s>(%X)\n", pokeSourceName(s), value);

    scheduleRegEvent<s>(DMA_CYCLES(2), REG_DIWSTRT, (int64_t)value);
}

template <PokeSource s> void
Agnus::pokeDIWSTOP(uint16_t value)
{
    debug(DIW_DEBUG, "pokeDIWSTOP<%s>(%X)\n", pokeSourceName(s), value);

    scheduleRegEvent<s>(DMA_CYCLES(2), REG_DIWSTOP, (int64_t)value);
}

void
Agnus::setDIWSTRT(uint16_t value)
{
    debug(DIW_DEBUG, "setDIWSTRT(%X)\n", value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0, V8 = 0

    diwstrt = value;

    // Extract the upper left corner of the display window
    int16_t newDiwVstrt = HI_BYTE(value);
    int16_t newDiwHstrt = LO_BYTE(value);

    debug(DIW_DEBUG, "newDiwVstrt = %d newDiwHstrt = %d\n", newDiwVstrt, newDiwHstrt);

    // Invalidate the horizontal coordinate if it is out of range
    if (newDiwHstrt < 2) {
        debug(DIW_DEBUG, "newDiwHstrt is too small\n");
        newDiwHstrt = -1;
    }

    /* Check if the change already takes effect in the current rasterline.
     *
     *     old: Old trigger coordinate (diwHstrt)
     *     new: New trigger coordinate (newDiwHstrt)
     *     cur: Position of the electron beam (derivable from pos.h)
     *
     * The following cases have to be taken into accout:
     *
     *    1) cur < old < new : Change takes effect in this rasterline.
     *    2) cur < new < old : Change takes effect in this rasterline.
     *    3) new < cur < old : Neither the old nor the new trigger hits.
     *    4) new < old < cur : Already triggered. Nothing to do in this line.
     *    5) old < cur < new : Already triggered. Nothing to do in this line.
     *    6) old < new < cur : Already triggered. Nothing to do in this line.
     */

    int16_t cur = 2 * pos.h;

     // (1) and (2)
    if (cur < diwHstrt && cur < newDiwHstrt) {

        debug(DIW_DEBUG, "Updating hFlopOn immediately at %d\n", cur);
        diwHFlopOn = newDiwHstrt;
    }

    // (3)
    if (newDiwHstrt < cur && cur < diwHstrt) {

        debug(DIW_DEBUG, "hFlop not switched on in current line\n");
        diwHFlopOn = -1;
    }

    diwVstrt = newDiwVstrt;
    diwHstrt = newDiwHstrt;
}

void
Agnus::setDIWSTOP(uint16_t value)
{
    debug(DIW_DEBUG, "setDIWSTOP(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1, V8 = !V7

    diwstop = value;

    // Extract the lower right corner of the display window
    int16_t newDiwVstop = HI_BYTE(value) | ((value & 0x8000) ? 0 : 0x100);
    int16_t newDiwHstop = LO_BYTE(value) | 0x100;

    debug(DIW_DEBUG, "newDiwVstop = %d newDiwHstop = %d\n", newDiwVstop, newDiwHstop);

    // Invalidate the coordinate if it is out of range
    if (newDiwHstop > 0x1C7) {
        debug(DIW_DEBUG, "newDiwHstop is too large\n");
        newDiwHstop = -1;
    }

    // Check if the change already takes effect in the current rasterline.
    int16_t cur = 2 * pos.h;

    // (1) and (2) (see setDIWSTRT)
    if (cur < diwHstop && cur < newDiwHstop) {

        debug(DIW_DEBUG, "Updating hFlopOff immediately at %d\n", cur);
        diwHFlopOff = newDiwHstop;
    }

    // (3) (see setDIWSTRT)
    if (newDiwHstop < cur && cur < diwHstop) {

        debug(DIW_DEBUG, "hFlop not switched off in current line\n");
        diwHFlopOff = -1;
    }

    diwVstop = newDiwVstop;
    diwHstop = newDiwHstop;
}

void
Agnus::pokeDDFSTRT(uint16_t value)
{
    debug(DDF_DEBUG, "pokeDDFSTRT(%X)\n", value);

    // 15 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 -- --

    ddfstrt = value & 0xFC;

    // Let the hsync handler recompute the data fetch window
    hsyncActions |= HSYNC_COMPUTE_DDF_WINDOW;

    // Take action if we haven't reached the old DDFSTRT cycle yet
    if (pos.h < ddfstrtReached) {

        // Check if the new position has already been passed
        if (ddfstrt <= pos.h + 2) {

            // DDFSTRT never matches in the current rasterline. Disable DMA
            ddfstrtReached = -1;
            switchBitplaneDmaOff();

        } else {

            // Update the matching position and recalculute the DMA table
            ddfstrtReached = ddfstrt;
            computeDDFStrt();
            updateBitplaneDma();
            scheduleNextBplEvent();
        }
    }

    // int16_t oldStrt = denise->hires() ? dmaStrtHires : dmaStrtLores;
    // if (pos.h <= oldStrt - 2) {
    /*
    if (pos.h < ddfstrt - 2) {

        debug("Recomputing allocation table: %d\n", ddfstrt);
        computeDDFStrt();
        updateBitplaneDma();
        scheduleNextBplEvent();
    }
    */
}

void
Agnus::pokeDDFSTOP(uint16_t value)
{
    debug(DDF_DEBUG, "pokeDDFSTOP(%X)\n", value);

    // 15 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 -- --

    ddfstop = value & 0xFC;

    // Let the hsync handler recompute the data fetch window
    hsyncActions |= HSYNC_COMPUTE_DDF_WINDOW;

    // Check if the modification also affects the current rasterline
    /*
    int16_t oldStop = denise->hires() ? dmaStopHires : dmaStopLores;
    if (pos.h <= oldStop - 2) {

        computeDDFStop();
        updateBitplaneDma();
        scheduleNextBplEvent();
    }
    */
}

void
Agnus::computeDDFStrt()
{
    int16_t strt = ddfstrt;

    // Align ddfstrt to the next possible fetch unit start
    dmaStrtHires = strt;
    dmaStrtLoresShift = strt & 0b100;
    dmaStrtLores = strt + dmaStrtLoresShift;

    debug(DDF_DEBUG, "computeDDFStrt: %d %d\n", dmaStrtLores, dmaStrtHires);
    assert(dmaStrtLoresShift == 0 || dmaStrtLoresShift == 4);
}

void
Agnus::computeDDFStop()
{
    int16_t strt = dmaStrtLores - dmaStrtLoresShift;
    int16_t stop = MIN(ddfstop, 0xD8);

    // Compute the number of fetch units
    int fetchUnits = ((stop - strt) + 15) >> 3;

    // Compute the end of the DMA window
    dmaStopLores = MIN(dmaStrtLores + 8 * fetchUnits, 0xE0);
    dmaStopHires = MIN(dmaStrtHires + 8 * fetchUnits, 0xE0);

    debug(DDF_DEBUG, "computeDDFStop: %d %d\n", dmaStopLores, dmaStopHires);

    /*
    // Compute the number of fetch units
    int numUnitsLores = (((stop - strt) + 15) >> 3);
    int numUnitsHires = (((stop - strt) + 15) >> 3) * 2;
    assert(numUnitsLores == (((stop - strt) +  7) >> 3) + 1);

    // Compute the end of the DMA window
    dmaStopLores = MIN(dmaStrtLores + 8 * numUnitsLores, 0xE0);
    dmaStopHires = MIN(dmaStrtHires + 4 * numUnitsHires, 0xE0);
    */
}

template <int x, PokeSource s> void
Agnus::pokeBPLxPTH(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPL%dPTH(%X)\n", x, value);

    // Check if the written value gets lost
    if (skipBPLxPT(x)) return;

    scheduleRegEvent<s>(DMA_CYCLES(2), REG_BPLxPTH, HI_W_LO_W(x, value));
}

template <int x, PokeSource s> void
Agnus::pokeBPLxPTL(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPL%dPTL(%X)\n", x, value);

    // Check if the written value gets lost
    if (skipBPLxPT(x)) return;

    scheduleRegEvent<s>(DMA_CYCLES(2), REG_BPLxPTL, HI_W_LO_W(x, value));
}

bool
Agnus::skipBPLxPT(int x)
{
    /* If a new value is written into BPLxPTL or BPLxPTH, this usually happens
     * as described in the left scenario:
     *
     * 88888888888888889999999999999999      88888888888888889999999999999999
     * 0123456789ABCDEF0123456789ABCDEF      0123456789ABCDEF0123456789ABCDEF
     * .4.2.351.4.2.351.4.2.351.4.2.351      .4.2.351.4.2.351.4.2.351.4.2.351
     *     ^ ^                                     ^ ^
     *     | |                                     | |
     *     | Change takes effect here              | New value is lost
     *     Write to BPLxPT                         Write to BPL1PT
     *
     * The right scenario shows that the new value can get lost under certain
     * circumstances. The following must hold:
     *
     *     (1) There is a Lx or Hx event once cycle after the BPL1PT write.
     *     (2) There is no DMA going on when the write would happen.
     */

    if (isBplxEvent(dmaEvent[pos.h + 1], x)) { // (1)

        if (dmaEvent[pos.h + 2] == EVENT_NONE) { // (2)

            // debug("skipBPLxPT: Value gets lost\n");
            // dumpDMAEventTable();
            return true;
        }
    }

    return false;
}

void
Agnus::setBPLxPTH(int x, uint16_t value)
{
    debug(BPL_DEBUG, "setBPLxPTH(%d, %X)\n", x, value);
    assert(1 <= x && x <= 6);

    bplpt[x - 1] = REPLACE_HI_WORD(bplpt[x - 1], value & 0x7);
}

void
Agnus::setBPLxPTL(int x, uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPLxPTL(%d, %X)\n", x, value);
    assert(1 <= x && x <= 6);

    bplpt[x - 1] = REPLACE_LO_WORD(bplpt[x - 1], value & 0xFFFE);
}

void
Agnus::pokeBPL1MOD(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPL1MOD(%X)\n", value);
    bpl1mod = int16_t(value & 0xFFFE);
}

void
Agnus::pokeBPL2MOD(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPL2MOD(%X)\n", value);
    bpl2mod = int16_t(value & 0xFFFE);
}

template <int x> void
Agnus::pokeSPRxPTH(uint16_t value)
{
    debug(SPR_DEBUG, "pokeSPR%dPTH(%X)\n", x, value);
    
    sprpt[x] = REPLACE_HI_WORD(sprpt[x], value & 0x7);
}

template <int x> void
Agnus::pokeSPRxPTL(uint16_t value)
{
    debug(SPR_DEBUG, "pokeSPR%dPTL(%X)\n", x, value);
    
    sprpt[x] = REPLACE_LO_WORD(sprpt[x], value & 0xFFFE);
}

void
Agnus::pokeBPLCON0(uint16_t value)
{
    debug(DMA_DEBUG, "pokeBPLCON0(%X)\n", value);

    if (bplcon0 != value) {

        pokeBPLCON0(bplcon0, value);
        bplcon0 = value;
    }
}

void
Agnus::pokeBPLCON0(uint16_t oldValue, uint16_t newValue)
{
    assert(oldValue != newValue);

    // Update the DMA allocation table in the next rasterline
    hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;

    // Check if the hires bit or one of the BPU bits have been modified
    if ((oldValue ^ newValue) & 0xF000) {

        /* Determine the number of enabled bitplanes.
         *
         *     - In hires mode, at most 4 bitplanes are possible.
         *     - In lores mode, at most 6 bitplanes are possible.
         *     - Invalid numbers disable bitplane DMA.
         */
        bool hires = (newValue & 0x8000);
        activeBitplanes = (newValue & 0x7000) >> 12;
        if (activeBitplanes > (hires ? 4 : 6)) activeBitplanes = 0;

        /* TODO:
         * BPLCON0 is usually written in each frame.
         * To speed up, just check the hpos. If it is smaller than the start
         * of the DMA window, a standard update() is enough and the scheduled
         * update in hsyncActions (HSYNC_UPDATE_EVENT_TABLE) can be omitted.
         */

        // Update the DMA allocation table with a 2 cycle delay
        allocateBplSlots(inBplDmaArea() ? activeBitplanes : 0, hires, pos.h + 2);

        // Since the table has changed, we also need to update the event slot
        scheduleBplEventForCycle(pos.h);
    }
}

void
Agnus::executeUntil(Cycle targetClock)
{
    // msg("clock is %lld, Executing until %lld\n", clock, targetClock);
    while (clock <= targetClock - DMA_CYCLES(1)) {

        // Process all pending events
        if (clock >= nextTrigger) executeEventsUntil(clock);
        
        // Advance the internal counters
        pos.h++;
        
        // Note: If this assertion hits, the HSYNC event hasn't been served!
        assert(pos.h <= HPOS_MAX);

        clock += DMA_CYCLES(1);
    }
}

template <int nr> void
Agnus::executeFirstSpriteCycle()
{
    // Activate sprite data DMA if the first sprite line has been reached
    if (pos.v == sprVStrt[nr]) { sprDmaState[nr] = SPR_DMA_DATA; }

    // Deactivate sprite data DMA if the last sprite line has been reached
    if (pos.v == sprVStop[nr]) {

        // Deactivate sprite data DMA
        sprDmaState[nr] = SPR_DMA_IDLE;

        // Read the next control word (POS part)
        uint16_t pos = doSpriteDMA<nr>();

        // Extract vertical trigger coordinate bits from POS
        sprVStrt[nr] = ((pos & 0xFF00) >> 8) | (sprVStrt[nr] & 0x0100);
        denise->pokeSPRxPOS<nr>(pos);
    }

    // Read sprite data if data DMA is activated
    if (sprDmaState[nr] == SPR_DMA_DATA) {

        // Read DATA
        denise->pokeSPRxDATB<nr>(doSpriteDMA(nr));
    }
}

template <int nr> void
Agnus::executeSecondSpriteCycle()
{
    // Deactivate sprite data DMA if the last sprite line has been reached
    if (pos.v == sprVStop[nr]) {
        
        // Sprite DMA should already be inactive in the second DMA cycle
        assert(sprDmaState[nr] == SPR_DMA_IDLE);
        
        // Read the next control word (CTL part)
        uint16_t ctl = doSpriteDMA(nr);
        
        // Extract vertical trigger coordinate bits from CTL
        sprVStrt[nr] = ((ctl & 0b100) << 6) | (sprVStrt[nr] & 0x00FF);
        sprVStop[nr] = ((ctl & 0b010) << 7) | (ctl >> 8);
        denise->pokeSPRxCTL<nr>(ctl);
    }
    
    // Read sprite data if data DMA is activated
    if (sprDmaState[nr] == SPR_DMA_DATA) {
        
        // Read DATB
        denise->pokeSPRxDATA<nr>(doSpriteDMA(nr));
    }
}

void
Agnus::hsyncHandler()
{
    // Make sure we really reached the end of the line
    if (pos.h != HPOS_MAX) { dump(); assert(false); }


    //
    // Let other components finish up the current line
    //

    // Let Denise draw the current line
    denise->endOfLine(pos.v);

    // Let Paula synthesize new sound samples
    paula->audioUnit.executeUntil(clock);
    
    // Let CIA B count the HSYNCs
    amiga->ciaB.incrementTOD();
    
    // Check the keyboard once in a while
    if ((pos.v & 0b1111) == 0) amiga->keyboard.execute();

    //
    // Increment the position counters
    //

    /* Remember: When the end of a line is reached, we reset the horizontal
     * counter. The new value should be 0. To make things work, we have to set
     * it to -1, because there is an upcoming hpos++ instruction at the end
     * of executeUntil(). This means that we can not rely on the correct
     * hpos value in the hsync and vsync handlers(). The value will be
     * -1 and not 0 as expected. Take care of that and feel free to come up
     * with a nicer solution! */

    pos.v++;
    pos.h = -1;

    // Check if the current frame is finished
    if (pos.v >= frameInfo.numLines) {
        vsyncHandler();
    }

    //
    // Prepare for the next line
    //

    // Switch sprite DMA off if the last rasterline has been reached
    if (pos.v == frameInfo.numLines - 1) {
        for (unsigned i = 0; i < 8; i++) {
            sprDmaState[i] = SPR_DMA_IDLE;
        }
    }

    // Check if this line is a bitplane DMA line
    bool bplDmaArea = inBplDmaArea();

    // Update the event table if the value has changed
    if (bplDmaArea ^ oldBplDmaArea) {
        hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;
        oldBplDmaArea = bplDmaArea;
    }

    // Check if we have reached line 25 (sprite DMA starts here)
    if (pos.v == 25) {
        if ((dmacon & DMAEN) && (dmacon & SPREN)) {
            
            // Reset vertical sprite trigger coordinates which forces the sprite
            // logic to read in the control words for all sprites in this line.
            for (unsigned i = 0; i < 8; i++) { sprVStop[i] = 25; }
        }
    }


    //
    // Determine the disk, audio and sprite DMA status for the next line
    //

    if (dmacon & DMAEN) {

        // Copy DMA enable bits from dmacon
        dmaDAS = dmacon & 0b111111;

        // Disable sprites outside the sprite DMA area
        if (pos.v < 25 || pos.v >= frameInfo.numLines - 1) dmaDAS &= 0b011111;

    } else {

        dmaDAS = 0;
    }


    //
    // Update the DIW flipflops
    //

    // Vertical flipflop
    if (pos.v == diwVstrt && !diwVFlop) {
        diwVFlop = true;
        updateBitplaneDma();
    }
    if (pos.v == diwVstop && diwVFlop) {
        diwVFlop = false;
        updateBitplaneDma();
    }

    // Horizontal DIW flipflop
    diwHFlop = (diwHFlopOff != -1) ? false : (diwHFlopOn != -1) ? true : diwHFlop;
    diwHFlopOn = diwHstrt;
    diwHFlopOff = diwHstop;

    //
    // Update the DDF related variables
    //

    ddfstrtReached = ddfstrt;
    ddfstopReached = ddfstop;

    //
    // Process pending work items
    //

    if (hsyncActions) {

        if (hsyncActions & HSYNC_COMPUTE_DDF_WINDOW) {

            // Update the display data fetch window
            computeDDFStrt();
            computeDDFStop();
            hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;
        }

        if (hsyncActions & HSYNC_UPDATE_EVENT_TABLE) {

            // Update the bitplane DMA allocation table
            updateBitplaneDma();
        }


        hsyncActions = 0;
    }

    // Clear the bus usage table
    for (int i = 0; i < HPOS_CNT; i++) busOwner[i] = BUS_NONE;

    
    //
    // Schedule events
    //

    // Schedule the first biplane event (if any)
    if (nextDmaEvent[0]) {
        EventID eventID = dmaEvent[nextDmaEvent[0]];
        schedulePos<BPL_SLOT>(pos.v, nextDmaEvent[0], eventID);
    }

    // Schedule the next SYNC event
    scheduleInc<SYNC_SLOT>(DMA_CYCLES(HPOS_CNT), SYNC_EOL, pos.v);


    //
    // Let other components prepare for the next line
    //

    denise->beginOfLine(pos.v);
}

void
Agnus::vsyncHandler()
{
    // debug("diwVstrt = %d diwVstop = %d diwHstrt = %d diwHstop = %d\n", diwVstrt, diwVstop, diwHstrt, hstop);

    // Advance to the next frame
    frameInfo.nr++;

    // Check if we the next frame is drawn in interlace mode
    frameInfo.interlaced = denise->lace();

    // If yes, toggle the the long frame flipflop
    lof = (frameInfo.interlaced) ? !lof : true;

    // Determine if the next frame is a long or a short frame
    frameInfo.numLines = lof ? 313 : 312;

    // Increment frame and reset vpos
    frame++; // DEPRECATED
    assert(frame == frameInfo.nr);

    // Reset vertical position counter
    pos.v = 0;

    // Initialize the DIW flipflops
    diwVFlop = false;
    diwHFlop = true; 
    
    // CIA A counts VSYNCs
    amiga->ciaA.incrementTOD();
    
    // Trigger VSYNC interrupt
    paula->pokeINTREQ(0x8020);
    
    // Let the subcomponents do their own VSYNC stuff
    copper.vsyncAction();
    denise->beginOfFrame(frameInfo.interlaced);
    amiga->joystick1.execute();
    amiga->joystick2.execute();

    // Prepare to take a snapshot once in a while
    if (amiga->snapshotIsDue()) amiga->signalSnapshot();
        
    // Count some sheep (zzzzzz) ...
    if (!amiga->getWarp()) {
        amiga->synchronizeTiming();
    }
}


//
// Instantiate template functions
//

template void Agnus::executeFirstSpriteCycle<0>();
template void Agnus::executeFirstSpriteCycle<1>();
template void Agnus::executeFirstSpriteCycle<2>();
template void Agnus::executeFirstSpriteCycle<3>();
template void Agnus::executeFirstSpriteCycle<4>();
template void Agnus::executeFirstSpriteCycle<5>();
template void Agnus::executeFirstSpriteCycle<6>();
template void Agnus::executeFirstSpriteCycle<7>();

template void Agnus::executeSecondSpriteCycle<0>();
template void Agnus::executeSecondSpriteCycle<1>();
template void Agnus::executeSecondSpriteCycle<2>();
template void Agnus::executeSecondSpriteCycle<3>();
template void Agnus::executeSecondSpriteCycle<4>();
template void Agnus::executeSecondSpriteCycle<5>();
template void Agnus::executeSecondSpriteCycle<6>();
template void Agnus::executeSecondSpriteCycle<7>();

template void Agnus::pokeBPLxPTH<1, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTH<1, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTH<2, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTH<2, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTH<3, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTH<3, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTH<4, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTH<4, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTH<5, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTH<5, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTH<6, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTH<6, POKE_COPPER>(uint16_t value);

template void Agnus::pokeBPLxPTL<1, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTL<1, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTL<2, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTL<2, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTL<3, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTL<3, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTL<4, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTL<4, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTL<5, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTL<5, POKE_COPPER>(uint16_t value);
template void Agnus::pokeBPLxPTL<6, POKE_CPU>(uint16_t value);
template void Agnus::pokeBPLxPTL<6, POKE_COPPER>(uint16_t value);

template void Agnus::pokeSPRxPTH<0>(uint16_t value);
template void Agnus::pokeSPRxPTH<1>(uint16_t value);
template void Agnus::pokeSPRxPTH<2>(uint16_t value);
template void Agnus::pokeSPRxPTH<3>(uint16_t value);
template void Agnus::pokeSPRxPTH<4>(uint16_t value);
template void Agnus::pokeSPRxPTH<5>(uint16_t value);
template void Agnus::pokeSPRxPTH<6>(uint16_t value);
template void Agnus::pokeSPRxPTH<7>(uint16_t value);

template void Agnus::pokeSPRxPTL<0>(uint16_t value);
template void Agnus::pokeSPRxPTL<1>(uint16_t value);
template void Agnus::pokeSPRxPTL<2>(uint16_t value);
template void Agnus::pokeSPRxPTL<3>(uint16_t value);
template void Agnus::pokeSPRxPTL<4>(uint16_t value);
template void Agnus::pokeSPRxPTL<5>(uint16_t value);
template void Agnus::pokeSPRxPTL<6>(uint16_t value);
template void Agnus::pokeSPRxPTL<7>(uint16_t value);

template uint16_t Agnus::doBitplaneDMA<0>();
template uint16_t Agnus::doBitplaneDMA<1>();
template uint16_t Agnus::doBitplaneDMA<2>();
template uint16_t Agnus::doBitplaneDMA<3>();
template uint16_t Agnus::doBitplaneDMA<4>();
template uint16_t Agnus::doBitplaneDMA<5>();

template void Agnus::pokeDIWSTRT<POKE_CPU>(uint16_t value);
template void Agnus::pokeDIWSTRT<POKE_COPPER>(uint16_t value);
template void Agnus::pokeDIWSTOP<POKE_CPU>(uint16_t value);
template void Agnus::pokeDIWSTOP<POKE_COPPER>(uint16_t value);

