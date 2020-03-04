// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Agnus::Agnus(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("Agnus");
    
    subComponents = vector<HardwareComponent *> {
        
        &copper,
        &blitter,
        &dmaDebugger
    };

    config.revision = AGNUS_8372;

    initLookupTables();
}

void
Agnus::initLookupTables()
{
    initBplEventTableLores();
    initBplEventTableHires();
    initDasEventTable();
}

void
Agnus::initBplEventTableLores()
{
    memset(bplDMA[0], 0, sizeof(bplDMA[0]));
    memset(fetchUnitNr[0], 0, sizeof(fetchUnitNr[0]));

    for (int bpu = 0; bpu < 7; bpu++) {

        EventID *p = &bplDMA[0][bpu][0];

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

        assert(bplDMA[0][bpu][HPOS_MAX] == EVENT_NONE);
        bplDMA[0][bpu][HPOS_MAX] = BPL_EOL;
    }

    for (int i = 0; i <= 0xD8; i++) {
        fetchUnitNr[0][i] = i % 8;
    }
}

void
Agnus::initBplEventTableHires()
{
    memset(bplDMA[1], 0, sizeof(bplDMA[1]));
    memset(fetchUnitNr[1], 0, sizeof(fetchUnitNr[1]));

    for (int bpu = 0; bpu < 7; bpu++) {

        EventID *p = &bplDMA[1][bpu][0];

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

        assert(bplDMA[1][bpu][HPOS_MAX] == EVENT_NONE);
        bplDMA[1][bpu][HPOS_MAX] = BPL_EOL;
    }

    for (int i = 0; i <= 0xD8; i++) {
        fetchUnitNr[0][i] = i % 4;
    }
}

void
Agnus::initDasEventTable()
{
    memset(dasDMA, 0, sizeof(dasDMA));

    for (int dmacon = 0; dmacon < 64; dmacon++) {

        EventID *p = dasDMA[dmacon];

        p[0x01] = DAS_REFRESH;

        if (dmacon & DSKEN) {
            p[0x07] = DAS_D0;
            p[0x09] = DAS_D1;
            p[0x0B] = DAS_D2;
        }

        if (dmacon & AUD0EN) p[0x0D] = DAS_A0;
        if (dmacon & AUD1EN) p[0x0F] = DAS_A1;
        if (dmacon & AUD2EN) p[0x11] = DAS_A2;
        if (dmacon & AUD3EN) p[0x13] = DAS_A3;

        if (dmacon & SPREN) {
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
        // p[0xE2] = DAS_REFRESH;
    }
}

void
Agnus::setRevision(AgnusRevision revision)
{
    debug("setRevision(%d)\n", revision);

    assert(isAgnusRevision(revision));
    config.revision = revision;
}

long
Agnus::chipRamLimit()
{
    switch (config.revision) {

        case AGNUS_8375: return 2048;
        case AGNUS_8372: return 1024;
        default:         return 512;
    }
}

u32
Agnus::chipRamMask()
{
    switch (config.revision) {

        case AGNUS_8375: return 0x1FFFFF;
        case AGNUS_8372: return 0x0FFFFF;
        default:         return 0x07FFFF;
    }
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

    // Initialize statistical counters
    clearStats();

    // Initialize event tables
    clearBplEventTable();
    clearDasEventTable();

    // Initialize the event slots
    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        slot[i].triggerCycle = NEVER;
        slot[i].id = (EventID)0;
        slot[i].data = 0;
    }

    // Schedule initial events
    scheduleAbs<RAS_SLOT>(DMA_CYCLES(HPOS_CNT), RAS_HSYNC);
    scheduleAbs<CIAA_SLOT>(CIA_CYCLES(1), CIA_EXECUTE);
    scheduleAbs<CIAB_SLOT>(CIA_CYCLES(1), CIA_EXECUTE);
    scheduleAbs<SEC_SLOT>(NEVER, SEC_TRIGGER);
    scheduleAbs<KBD_SLOT>(DMA_CYCLES(1), KBD_SELFTEST);
    scheduleAbs<VBL_SLOT>(DMA_CYCLES(HPOS_CNT * vStrobeLine() + 1), VBL_STROBE);
    scheduleAbs<IRQ_SLOT>(NEVER, IRQ_CHECK);
    scheduleNextBplEvent();
    scheduleNextDasEvent();
}

void
Agnus::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);

    u32 mask      = chipRamMask();

    info.vpos     = pos.v;
    info.hpos     = pos.h;

    info.dmacon   = dmacon;
    info.bplcon0  = bplcon0;
    info.bpu      = bpu();
    info.ddfstrt  = ddfstrt;
    info.ddfstop  = ddfstop;
    info.diwstrt  = diwstrt;
    info.diwstop  = diwstop;

    info.bpl1mod  = bpl1mod;
    info.bpl2mod  = bpl2mod;
    info.bltamod  = blitter.bltamod;
    info.bltbmod  = blitter.bltbmod;
    info.bltcmod  = blitter.bltcmod;
    info.bltdmod  = blitter.bltdmod;
    info.bls      = bls;

    info.coppc    = copper.coppc & mask;
    info.dskpt    = dskpt & mask;
    info.bltpt[0] = blitter.bltapt & mask;
    info.bltpt[1] = blitter.bltbpt & mask;
    info.bltpt[2] = blitter.bltcpt & mask;
    info.bltpt[3] = blitter.bltdpt & mask;
    for (unsigned i = 0; i < 6; i++) info.bplpt[i] = bplpt[i] & mask;
    for (unsigned i = 0; i < 4; i++) info.audpt[i] = audpt[i] & mask;
    for (unsigned i = 0; i < 8; i++) info.sprpt[i] = sprpt[i] & mask;

    pthread_mutex_unlock(&lock);
}

void
Agnus::_dump()
{
    msg(" actions : %X\n", hsyncActions);

    msg("   dskpt : %X\n", dskpt);
    for (unsigned i = 0; i < 4; i++) msg("audpt[%d] : %X\n", i, audpt[i]);
    for (unsigned i = 0; i < 6; i++) msg("bplpt[%d] : %X\n", i, bplpt[i]);
    for (unsigned i = 0; i < 8; i++) msg("bplpt[%d] : %X\n", i, sprpt[i]);
    
    msg("   hstrt : %d\n", diwHstrt);
    msg("   hstop : %d\n", diwHstop);
    msg("   vstrt : %d\n", diwVstrt);
    msg("   vstop : %d\n", diwVstop);

    msg("\nEvents:\n\n");
    dumpEvents();

    msg("\nBPL DMA table:\n\n");
    dumpBplEventTable();

    msg("\nDAS DMA table:\n\n");
    dumpDasEventTable();
}

AgnusInfo
Agnus::getInfo()
{
    AgnusInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

Cycle
Agnus::cyclesInFrame()
{
    return DMA_CYCLES(frameInfo.numLines * HPOS_CNT);
}

Cycle
Agnus::startOfFrame()
{
    return clock - DMA_CYCLES(pos.v * HPOS_CNT + pos.h);
}

Cycle
Agnus::startOfNextFrame()
{
    return startOfFrame() + cyclesInFrame();
}

bool
Agnus::belongsToPreviousFrame(Cycle cycle)
{
    return cycle < startOfFrame();
}

bool
Agnus::belongsToCurrentFrame(Cycle cycle)
{
    return !belongsToPreviousFrame(cycle) && !belongsToNextFrame(cycle);
}

bool
Agnus::belongsToNextFrame(Cycle cycle)
{
    return cycle >= startOfNextFrame();
}

bool
Agnus::inBplDmaLine(u16 dmacon, u16 bplcon0) {

    return
    ddfVFlop            // Outside VBLANK, inside DIW
    && bpu(bplcon0)     // At least one bitplane enabled
    && bpldma(dmacon);  // Bitplane DMA enabled
}

Cycle
Agnus::beamToCycle(Beam beam)
{
    return startOfFrame() + DMA_CYCLES(beam.v * HPOS_CNT + beam.h);
}

Beam
Agnus::cycleToBeam(Cycle cycle)
{
    Beam result;

    Cycle diff = AS_DMA_CYCLES(cycle - startOfFrame());
    assert(diff >= 0);

    result.v = diff / HPOS_CNT;
    result.h = diff % HPOS_CNT;
    return result;
}

Beam
Agnus::addToBeam(Beam beam, Cycle cycles)
{
    Beam result;

    Cycle cycle = beam.v * HPOS_CNT + beam.h + cycles;
    result.v = cycle / HPOS_CNT;
    result.h = cycle % HPOS_CNT;

    return result;
}

bool
Agnus::copperCanRun()
{
    // Deny access if Copper DMA is disabled
    if (!copdma()) return false;

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

template <BusOwner owner> bool
Agnus::busIsFree()
{
    // Deny if the bus has been allocated already
    if (busOwner[pos.h] != BUS_NONE) return false;

    switch (owner) {

        case BUS_BLITTER:
        {
            // Deny if Blitter DMA is off
            if (!bltdma()) return false;
        }
        default: return true;
    }
}

template <BusOwner owner> bool
Agnus::allocateBus()
{
    // Deny if the bus has been allocated already
    if (busOwner[pos.h] != BUS_NONE) return false;

    switch (owner) {

        case BUS_COPPER:
        {
            // Assign bus to the Copper
            busOwner[pos.h] = BUS_COPPER;
            return true;
        }
        case BUS_BLITTER:
        {
            // Deny if Blitter DMA is off
            if (!bltdma()) return false;

            // Deny if the CPU has precedence
            if (bls && !bltpri()) return false;

            // Assign the bus to the Blitter
            busOwner[pos.h] = BUS_BLITTER;
            return true;
        }
    }

    assert(false);
    return false;
}

u16
Agnus::doDiskDMA()
{
    u16 result = mem.peekChip16(dskpt);
    INC_CHIP_PTR(dskpt);

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = result;
    stats.count[BUS_DISK]++;

    return result;
}

void
Agnus::doDiskDMA(u16 value)
{
    mem.pokeChip16(dskpt, value);
    INC_CHIP_PTR(dskpt);

    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = value;
    stats.count[BUS_DISK]++;
}

template <int channel> u16
Agnus::doAudioDMA()
{
    u16 result;

    result = mem.peekChip16(audpt[channel]);
    INC_CHIP_PTR(audpt[channel]);

    busOwner[pos.h] = BUS_AUDIO;
    busValue[pos.h] = result;
    stats.count[BUS_AUDIO]++;

    return result;
}

template <int channel> u16
Agnus::doSpriteDMA()
{
    u16 result = mem.peekChip16(sprpt[channel]);
    INC_CHIP_PTR(sprpt[channel]);

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_SPRITE;
    busValue[pos.h] = result;
    stats.count[BUS_SPRITE]++;

    return result;
}

u16
Agnus::doSpriteDMA(int channel)
{
    u16 result = mem.peekChip16(sprpt[channel]);
    INC_CHIP_PTR(sprpt[channel]);

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_SPRITE;
    busValue[pos.h] = result;
    stats.count[BUS_SPRITE]++;

    return result;
}

template <int bitplane> u16
Agnus::doBitplaneDMA()
{
    u16 result = mem.peekChip16(bplpt[bitplane]);
    INC_CHIP_PTR(bplpt[bitplane]);

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_BITPLANE;
    busValue[pos.h] = result;
    stats.count[BUS_BITPLANE]++;

    return result;
}

u16
Agnus::copperRead(u32 addr)
{
    u16 result = mem.peek16<BUS_COPPER>(addr);

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = result;
    stats.count[BUS_COPPER]++;

    return result;
}

void
Agnus::copperWrite(u32 addr, u16 value)
{
    mem.pokeCustom16<POKE_COPPER>(addr, value);

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = value;
    stats.count[BUS_COPPER]++;
}

u16
Agnus::blitterRead(u32 addr)
{
    // Assure that the Blitter owns the bus when this function is called
    assert(pos.h < HPOS_CNT);
    assert(busOwner[pos.h] == BUS_BLITTER);

    u16 result = mem.peek16<BUS_BLITTER>(addr);

    busOwner[pos.h] = BUS_BLITTER;
    busValue[pos.h] = result;
    stats.count[BUS_BLITTER]++;

    return result;
}

void
Agnus::blitterWrite(u32 addr, u16 value)
{
    // Assure that the Blitter owns the bus when this function is called
    assert(pos.h < HPOS_CNT);
    assert(busOwner[pos.h] == BUS_BLITTER);

    mem.poke16<BUS_BLITTER>(addr, value);

    busOwner[pos.h] = BUS_BLITTER;
    busValue[pos.h] = value;
    stats.count[BUS_BLITTER]++;
}

void
Agnus::clearBplEventTable()
{
    memset(bplEvent, 0, sizeof(bplEvent));
    bplEvent[HPOS_MAX] = BPL_EOL;
    updateBplJumpTable();
}

void
Agnus::clearDasEventTable()
{
    memset(dasEvent, 0, sizeof(dasEvent));
    updateDasDma(0);
    updateDasJumpTable();
}

void
Agnus::allocateBplSlots(u16 dmacon, u16 bplcon0, int first, int last)
{
    assert(first >= 0 && last < HPOS_MAX);

    int channels = bpu(bplcon0);
    bool hires = Denise::hires(bplcon0);

    // Set number of bitplanes to 0 if we are not in a bitplane DMA line
    if (!inBplDmaLine(dmacon, bplcon0)) channels = 0;
    assert(channels <= 6);

    // Allocate slots
    if (hires) {
        for (int i = first; i <= last; i++) {
            bplEvent[i] = inHiresDmaArea(i) ? bplDMA[1][channels][i] : EVENT_NONE;
        }
    } else {
        for (int i = first; i <= last; i++) {
            bplEvent[i] = inLoresDmaArea(i) ? bplDMA[0][channels][i] : EVENT_NONE;
        }
    }

    updateBplJumpTable();
}

void
Agnus::allocateBplSlots(int first, int last)
{
    allocateBplSlots(dmacon, bplcon0, first, last);
}

void
Agnus::switchBplDmaOn()
{
    i16 start;
    i16 stop;

    bool hires = denise.hires();
    int activeBitplanes = bpu();

    // Determine the range that is covered by fetch units
    if (hires) {

        start = ddfStrtHires;
        stop = ddfStopHires;
        assert((stop - start) % 4 == 0);

    } else {

        start = ddfStrtLores;
        stop = ddfStopLores;
        assert((stop - start) % 8 == 0);
    }

    debug(BPL_DEBUG, "switchBitplaneDmaOn()\n");
    debug(BPL_DEBUG, "hires = %d start = %d stop = %d\n", hires, start, stop);

    assert(start >= 0 && start <= HPOS_MAX);
    assert(stop >= 0 && stop <= HPOS_MAX);

    // Wipe out all events outside the fetch unit window
    for (int i = 0; i < start; i++) bplEvent[i] = EVENT_NONE;
    for (int i = stop; i < HPOS_MAX; i++) bplEvent[i] = EVENT_NONE;

    // Copy events from the proper lookup table
    for (int i = start; i < stop; i++) {
        bplEvent[i] = bplDMA[hires][activeBitplanes][i];
    }

    // Setup the jump table
    updateBplJumpTable();
}


void
Agnus::switchBplDmaOff()
{
    debug(BPL_DEBUG, "switchBitplaneDmaOff: \n");

    // Quick-exit if nothing happens at regular DMA cycle positions
    if (nextBplEvent[0] == HPOS_MAX) {
        assert(bplEvent[nextBplEvent[0]] == BPL_EOL);
        return;
    }

    clearBplEventTable();
    scheduleNextBplEvent();
}

void
Agnus::updateBplDma()
{
    debug(BPL_DEBUG, "updateBitplaneDma()\n");

    // Determine if bitplane DMA has to be on or off
    bool bplDma = inBplDmaLine();

    // Update the event table accordingly
    bplDma ? switchBplDmaOn() : switchBplDmaOff();
}

void
Agnus::updateDasDma(u16 dmacon)
{
    assert(dmacon < 64);

    // Copy events from the proper lookup table
    for (int i = 0; i < 0x38; i++) {
        dasEvent[i] = dasDMA[dmacon][i];
    }
    dasEvent[0xE2] = dasDMA[dmacon][0xE2];

    // Setup the jump table
    updateDasJumpTable();
}

void
Agnus::updateJumpTable(EventID *eventTable, u8 *jumpTable, int end)
{
    assert(end <= HPOS_MAX);

     u8 next = jumpTable[end];
     for (int i = end; i >= 0; i--) {
         jumpTable[i] = next;
         if (eventTable[i]) next = i;
     }
}

void
Agnus::updateBplJumpTable(i16 end)
{
    // Build the jump table
    updateJumpTable(bplEvent, nextBplEvent, end);

    // Make sure the table ends with an BPL_EOL event
    assert(bplEvent[HPOS_MAX] == BPL_EOL);
    assert(nextBplEvent[HPOS_MAX - 1] == HPOS_MAX);
}

void
Agnus::updateDasJumpTable(i16 end)
{
    // Build the jump table
    updateJumpTable(dasEvent, nextDasEvent, end);
}

bool
Agnus::isLastLx(i16 dmaCycle)
{
    return (pos.h >= ddfStopLores - 8);
}

bool
Agnus::isLastHx(i16 dmaCycle)
{
    return (pos.h >= ddfStopHires - 4);
}

bool
Agnus::inLastFetchUnit(i16 dmaCycle)
{
    return denise.hires() ? isLastHx(dmaCycle) : isLastLx(dmaCycle);
}

void
Agnus::dumpEventTable(EventID *table, char str[256][2], int from, int to)
{
    char r1[256], r2[256], r3[256], r4[256];
    int i;

    for (i = 0; i <= to - from; i++) {

        int digit1 = (from + i) / 16;
        int digit2 = (from + i) % 16;

        r1[i] = (digit1 < 10) ? digit1 + '0' : (digit1 - 10) + 'A';
        r2[i] = (digit2 < 10) ? digit2 + '0' : (digit2 - 10) + 'A';
        r3[i] = str[table[from + i]][0];
        r4[i] = str[table[from + i]][1];
    }
    r1[i] = r2[i] = r3[i] = r4[i] = 0;

    msg("%s\n", r1);
    msg("%s\n", r2);
    msg("%s\n", r3);
    msg("%s\n", r4);
}


void
Agnus::dumpBplEventTable(int from, int to)
{
    char str[256][2];

    memset(str, '?', sizeof(str));
    str[(int)EVENT_NONE][0] = '.'; str[(int)EVENT_NONE][1] = '.';
    str[(int)BPL_L1][0]     = 'L'; str[(int)BPL_L1][1]     = '1';
    str[(int)BPL_L2][0]     = 'L'; str[(int)BPL_L2][1]     = '2';
    str[(int)BPL_L3][0]     = 'L'; str[(int)BPL_L3][1]     = '3';
    str[(int)BPL_L4][0]     = 'L'; str[(int)BPL_L4][1]     = '4';
    str[(int)BPL_L5][0]     = 'L'; str[(int)BPL_L5][1]     = '5';
    str[(int)BPL_L6][0]     = 'L'; str[(int)BPL_L6][1]     = '6';
    str[(int)BPL_H1][0]     = 'H'; str[(int)BPL_H1][1]     = '1';
    str[(int)BPL_H2][0]     = 'H'; str[(int)BPL_H2][1]     = '2';
    str[(int)BPL_H3][0]     = 'H'; str[(int)BPL_H3][1]     = '3';
    str[(int)BPL_H4][0]     = 'H'; str[(int)BPL_H4][1]     = '4';
    str[(int)BPL_EOL][0]    = 'E'; str[(int)BPL_EOL][1]    = 'O';

    dumpEventTable(bplEvent, str, from, to);

    /*
    char r1[256], r2[256], r3[256], r4[256];
    int i;

    for (i = 0; i <= (to - from); i++) {
        
        int digit1 = (from + i) / 16;
        int digit2 = (from + i) % 16;
        
        r1[i] = (digit1 < 10) ? digit1 + '0' : (digit1 - 10) + 'A';
        r2[i] = (digit2 < 10) ? digit2 + '0' : (digit2 - 10) + 'A';
        
        switch(bplEvent[i + from]) {
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
            case BPL_EOL:      r3[i] = 'E'; r4[i] = 'O'; break;
            default:           r3[i] = '?'; r4[i] = '?'; break;
        }
    }
    r1[i] = r2[i] = r3[i] = r4[i] = 0;
    msg("%s\n", r1);
    msg("%s\n", r2);
    msg("%s\n", r3);
    msg("%s\n", r4);
    */
}

void
Agnus::dumpBplEventTable()
{
    // Dump the event table
    msg("Event table:\n\n");
    msg("ddfstrt = %X dffstop = %X\n",
             ddfstrt, ddfstop);
    msg("ddfStrtLores = %X ddfStrtHires = %X\n",
             ddfStrtLores, ddfStrtHires);
    msg("ddfStopLores = %X ddfStopHires = %X\n",
             ddfStopLores, ddfStopHires);

    dumpBplEventTable(0x00, 0x4F);
    dumpBplEventTable(0x50, 0x9F);
    dumpBplEventTable(0xA0, 0xE2);

    // Dump the jump table
    msg("\nJump table:\n\n");
    int i = nextBplEvent[0];
    msg("0 -> %X", i);
    while (i) {
        assert(i < HPOS_CNT);
        assert(nextBplEvent[i] == 0 || nextBplEvent[i] > i);
        i = nextBplEvent[i];
        msg(" -> %X", i);
    }
    msg("\n");
}

void
Agnus::dumpDasEventTable(int from, int to)
{
    char str[256][2];

    memset(str, '?', sizeof(str));
    str[(int)EVENT_NONE][0]  = '.'; str[(int)EVENT_NONE][1]  = '.';
    str[(int)DAS_REFRESH][0] = 'R'; str[(int)DAS_REFRESH][1] = 'E';
    str[(int)DAS_D0][0]      = 'D'; str[(int)DAS_D0][1]      = '0';
    str[(int)DAS_D1][0]      = 'D'; str[(int)DAS_D1][1]      = '1';
    str[(int)DAS_D2][0]      = 'D'; str[(int)DAS_D2][1]      = '2';
    str[(int)DAS_A0][0]      = 'A'; str[(int)DAS_A0][1]      = '0';
    str[(int)DAS_A1][0]      = 'A'; str[(int)DAS_A1][1]      = '1';
    str[(int)DAS_A2][0]      = 'A'; str[(int)DAS_A2][1]      = '2';
    str[(int)DAS_A3][0]      = 'A'; str[(int)DAS_A3][1]      = '3';
    str[(int)DAS_S0_1][0]    = '0'; str[(int)DAS_S0_1][1]    = '1';
    str[(int)DAS_S0_2][0]    = '0'; str[(int)DAS_S0_2][1]    = '2';
    str[(int)DAS_S1_1][0]    = '1'; str[(int)DAS_S1_1][1]    = '1';
    str[(int)DAS_S1_2][0]    = '1'; str[(int)DAS_S1_2][1]    = '2';
    str[(int)DAS_S2_1][0]    = '2'; str[(int)DAS_S2_1][1]    = '1';
    str[(int)DAS_S2_2][0]    = '2'; str[(int)DAS_S2_2][1]    = '2';
    str[(int)DAS_S3_1][0]    = '3'; str[(int)DAS_S3_1][1]    = '1';
    str[(int)DAS_S3_2][0]    = '3'; str[(int)DAS_S3_2][1]    = '2';
    str[(int)DAS_S4_1][0]    = '4'; str[(int)DAS_S4_1][1]    = '1';
    str[(int)DAS_S4_2][0]    = '4'; str[(int)DAS_S4_2][1]    = '2';
    str[(int)DAS_S5_1][0]    = '5'; str[(int)DAS_S5_1][1]    = '1';
    str[(int)DAS_S5_2][0]    = '5'; str[(int)DAS_S5_2][1]    = '2';
    str[(int)DAS_S6_1][0]    = '6'; str[(int)DAS_S6_1][1]    = '1';
    str[(int)DAS_S6_2][0]    = '6'; str[(int)DAS_S6_2][1]    = '2';
    str[(int)DAS_S7_1][0]    = '7'; str[(int)DAS_S7_1][1]    = '1';
    str[(int)DAS_S7_2][0]    = '7'; str[(int)DAS_S7_2][1]    = '2';
    str[(int)DAS_SDMA][0]    = 'S'; str[(int)DAS_SDMA][1]    = 'D';

    dumpEventTable(dasEvent, str, from, to);
}

void
Agnus::dumpDasEventTable()
{
    // Dump the event table
    dumpDasEventTable(0x00, 0x4F);
    dumpDasEventTable(0x50, 0x9F);
    dumpDasEventTable(0xA0, 0xE2);
}

u16
Agnus::peekDMACONR()
{
    u16 result = dmacon;

    assert((result & ((1 << 14) | (1 << 13))) == 0);
    
    if (blitter.isBusy()) result |= (1 << 14);
    if (blitter.isZero()) result |= (1 << 13);

    debug(2, "peekDMACONR: %X\n", result);
    return result;
}

void
Agnus::pokeDMACON(u16 value)
{
    debug(DMA_DEBUG, "pokeDMACON(%X)\n", value);

    // Record the change
    recordRegisterChange(DMA_CYCLES(2), REG_DMACON, value);
}

void
Agnus::setDMACON(u16 oldValue, u16 value)
{
    debug(DMA_DEBUG, "setDMACON(%x, %x)\n", oldValue, value);

    // Compute new value
    u16 newValue;
    if (value & 0x8000) {
        newValue = (dmacon | value) & 0x07FF;
    } else {
        newValue = (dmacon & ~value) & 0x07FF;
    }

    if (oldValue == newValue) return;

    dmacon = newValue;

    // Update variable dmaconAtDDFStrt if DDFSTRT has not been reached yet
    if (pos.h + 2 < ddfstrtReached) dmaconAtDDFStrt = newValue;

    // Check the lowest 5 bits
    bool oldDMAEN = (oldValue & DMAEN);
    bool oldBPLEN = (oldValue & BPLEN) && oldDMAEN;
    bool oldCOPEN = (oldValue & COPEN) && oldDMAEN;
    bool oldBLTEN = (oldValue & BLTEN) && oldDMAEN;
    bool oldSPREN = (oldValue & SPREN) && oldDMAEN;
    bool oldDSKEN = (oldValue & DSKEN) && oldDMAEN;
    bool oldAUD0EN = (oldValue & AUD0EN) && oldDMAEN;
    bool oldAUD1EN = (oldValue & AUD1EN) && oldDMAEN;
    bool oldAUD2EN = (oldValue & AUD2EN) && oldDMAEN;
    bool oldAUD3EN = (oldValue & AUD3EN) && oldDMAEN;

    bool newDMAEN = (newValue & DMAEN);
    bool newBPLEN = (newValue & BPLEN) && newDMAEN;
    bool newCOPEN = (newValue & COPEN) && newDMAEN;
    bool newBLTEN = (newValue & BLTEN) && newDMAEN;
    bool newSPREN = (newValue & SPREN) && newDMAEN;
    bool newDSKEN = (newValue & DSKEN) && newDMAEN;
    bool newAUD0EN = (newValue & AUD0EN) && newDMAEN;
    bool newAUD1EN = (newValue & AUD1EN) && newDMAEN;
    bool newAUD2EN = (newValue & AUD2EN) && newDMAEN;
    bool newAUD3EN = (newValue & AUD3EN) && newDMAEN;

    // Inform the delegates
    blitter.pokeDMACON(oldValue, newValue);

    // Bitplane DMA
    if (oldBPLEN ^ newBPLEN) {

        // Update the bpl event table in the next rasterline
        hsyncActions |= HSYNC_UPDATE_BPL_TABLE;

        if (newBPLEN) {

            // Bitplane DMA is switched on

            // Check if the current line is affected by the change
            if (pos.h + 2 < ddfstrtReached || bpldma(dmaconAtDDFStrt)) {

                allocateBplSlots(newValue, bplcon0, pos.h + 2);
                updateBplEvent();
            }

        } else {

            // Bitplane DMA is switched off
            allocateBplSlots(newValue, bplcon0, pos.h + 2);
            updateBplEvent();
        }

        // Let Denise know about the change
        denise.pokeDMACON(oldValue, newValue);
    }

    // Check DAS DMA (Disk, Audio, Sprites)
    u16 oldDAS = oldDMAEN ? (oldValue & 0x3F) : 0;
    u16 newDAS = newDMAEN ? (newValue & 0x3F) : 0;

    if (oldDAS != newDAS) {

        // Schedule the DAS DMA tabel to rebuild
        hsyncActions |= HSYNC_UPDATE_DAS_TABLE;

        // Make the effect visible in the current rasterline as well
        for (int i = pos.h; i < HPOS_CNT; i++) {
            dasEvent[i] = dasDMA[newDAS][i];
        }
        updateDasJumpTable();

        // Rectify the currently scheduled DAS event
        scheduleDasEventForCycle(pos.h);
    }

    // Copper DMA
    if (oldCOPEN ^ newCOPEN) {
        debug(DMA_DEBUG, "Copper DMA switched %s\n", newCOPEN ? "on" : "off");
    }
    
    // Blitter DMA
    if (oldBLTEN ^ newBLTEN) {
        debug(DMA_DEBUG, "Blitter DMA switched %s\n", newBLTEN ? "on" : "off");
    }
    
    // Sprite DMA
    if (oldSPREN ^ newSPREN) {
        debug(DMA_DEBUG, "Sprite DMA switched %s\n", newSPREN ? "on" : "off");
    }
    
    // Disk DMA
    if (oldDSKEN ^ newDSKEN) {
        debug(DMA_DEBUG, "Disk DMA switched %s\n", newDSKEN ? "on" : "off");
    }
    
    // Audio DMA
    if (oldAUD0EN ^ newAUD0EN) {
        newAUD0EN ? audioUnit.channel0.enableDMA() : audioUnit.channel0.disableDMA();
    }
    if (oldAUD1EN ^ newAUD1EN) {
        newAUD1EN ? audioUnit.channel1.enableDMA() : audioUnit.channel1.disableDMA();
    }
    if (oldAUD2EN ^ newAUD2EN) {
        newAUD2EN ? audioUnit.channel2.enableDMA() : audioUnit.channel2.disableDMA();
    }
    if (oldAUD3EN ^ newAUD3EN) {
        newAUD3EN ? audioUnit.channel3.enableDMA() : audioUnit.channel3.disableDMA();
    }
}

void
Agnus::pokeDSKPTH(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKPTH(%X)\n", value);
    dskpt = REPLACE_HI_WORD(dskpt, value);
}

void
Agnus::pokeDSKPTL(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKPTL(%X)\n", value);
    dskpt = REPLACE_LO_WORD(dskpt, value & 0xFFFE);
}

u16
Agnus::peekVHPOSR()
{
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1

    i16 posh = pos.h + 4;
    i16 posv = pos.v;

    // posh might have wrapped over, because we added 4
    if (posh > HPOS_MAX) {
        posh -= HPOS_CNT;
        if (++posv >= frameInfo.numLines) posv = 0;
    }

    // posv wraps over in cycle 2
    if (posh > 1)
        return HI_LO(posv & 0xFF, posh);

    if (posv == 0)
       return HI_LO(isLongFrame() ? (312 & 0xFF) : (313 & 0xFF), posh);

    return HI_LO((posv - 1) & 0xFF, posh);
}

void
Agnus::pokeVHPOS(u16 value)
{
    debug(2, "pokeVHPOS(%X)\n", value);
    // Don't know what to do here ...
}

u16
Agnus::peekVPOSR()
{
    u16 id;

    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // LF I6 I5 I4 I3 I2 I1 I0 -- -- -- -- -- -- -- V8
    u16 result = (pos.v >> 8) | (isLongFrame() ? 0x8000 : 0);
    assert((result & 0x7FFE) == 0);

    // Add indentification bits
    switch (config.revision) {

        case AGNUS_8367: id = 0x00; break;
        case AGNUS_8372: id = 0x20; break;
        case AGNUS_8375: id = 0x20; break; // ??? CHECK ON REAL MACHINE
        default: assert(false);
    }
    result |= (id << 8);

    debug(2, "peekVPOSR() = %X\n", result);
    return result;
}

void
Agnus::pokeVPOS(u16 value)
{
    // Don't know what to do here ...
}

template <PokeSource s> void
Agnus::pokeDIWSTRT(u16 value)
{
    debug(DIW_DEBUG, "pokeDIWSTRT<%s>(%X)\n", pokeSourceName(s), value);
    recordRegisterChange(DMA_CYCLES(2), REG_DIWSTRT, value);
}

template <PokeSource s> void
Agnus::pokeDIWSTOP(u16 value)
{
    debug(DIW_DEBUG, "pokeDIWSTOP<%s>(%X)\n", pokeSourceName(s), value);
    recordRegisterChange(DMA_CYCLES(2), REG_DIWSTOP, value);
}

void
Agnus::setDIWSTRT(u16 value)
{
    debug(DIW_DEBUG, "setDIWSTRT(%X)\n", value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0, V8 = 0

    diwstrt = value;

    // Extract the upper left corner of the display window
    i16 newDiwVstrt = HI_BYTE(value);
    i16 newDiwHstrt = LO_BYTE(value);

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

    i16 cur = 2 * pos.h;

     // (1) and (2)
    if (cur < diwHstrt && cur < newDiwHstrt) {

        debug(DIW_DEBUG, "Updating DIW hflop immediately at %d\n", cur);
        diwHFlopOn = newDiwHstrt;
    }

    // (3)
    if (newDiwHstrt < cur && cur < diwHstrt) {

        debug(DIW_DEBUG, "DIW hflop not switched on in current line\n");
        diwHFlopOn = -1;
    }

    diwVstrt = newDiwVstrt;
    diwHstrt = newDiwHstrt;

    /* Update the vertical DIW flipflop
     * This is not 100% accurate. If the vertical DIW flipflop changes in the
     * middle of a rasterline, the effect is immediately visible on a real
     * Amiga. The current emulation code only evaluates the flipflop at the end
     * of the rasterline in the drawing routine of Denise. Hence, the whole
     * line will be blacked out, not just the rest of it.
     */
    if (pos.v == diwVstrt) diwVFlop = true;
    if (pos.v == diwVstop) diwVFlop = false;
}

void
Agnus::setDIWSTOP(u16 value)
{
    debug(DIW_DEBUG, "setDIWSTOP(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1, V8 = !V7

    diwstop = value;

    // Extract the lower right corner of the display window
    i16 newDiwVstop = HI_BYTE(value) | ((value & 0x8000) ? 0 : 0x100);
    i16 newDiwHstop = LO_BYTE(value) | 0x100;

    debug(DIW_DEBUG, "newDiwVstop = %d newDiwHstop = %d\n", newDiwVstop, newDiwHstop);

    // Invalidate the coordinate if it is out of range
    if (newDiwHstop > 0x1C7) {
        debug(DIW_DEBUG, "newDiwHstop is too large\n");
        newDiwHstop = -1;
    }

    // Check if the change already takes effect in the current rasterline.
    i16 cur = 2 * pos.h;

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

    /* Update the vertical DIW flipflop
     * This is not 100% accurate. See comment in setDIWSTRT().
     */
    if (pos.v == diwVstrt) diwVFlop = true;
    if (pos.v == diwVstop) diwVFlop = false;
}

void
Agnus::pokeDDFSTRT(u16 value)
{
    debug(DDF_DEBUG, "pokeDDFSTRT(%X)\n", value);

    //      15 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // OCS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 -- --
    // ECS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 H2 --

    value &= ddfMask();
    recordRegisterChange(DMA_CYCLES(2), REG_DDFSTRT, value);
}

void
Agnus::pokeDDFSTOP(u16 value)
{
    debug(DDF_DEBUG, "pokeDDFSTOP(%X)\n", value);

    //      15 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // OCS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 -- --
    // ECS: -- -- -- -- -- -- -- H8 H7 H6 H5 H4 H3 H2 --

    value &= ddfMask();
    recordRegisterChange(DMA_CYCLES(2), REG_DDFSTOP, value);
}

void
Agnus::setDDFSTRT(u16 old, u16 value)
{
    debug(DDF_DEBUG, "setDDFSTRT(%X, %X)\n", old, value);

    ddfstrt = value;

    // Tell the hsync handler to recompute the DDF window
    hsyncActions |= HSYNC_PREDICT_DDF;

    // Take immediate action if we haven't reached the old DDFSTRT cycle yet
    if (pos.h < ddfstrtReached) {

        // Check if the new position has already been passed
        if (ddfstrt <= pos.h + 2) {

            // DDFSTRT never matches in the current rasterline. Disable DMA
            ddfstrtReached = -1;
            switchBplDmaOff();

        } else {

            // Update the matching position and recalculate the DMA table
            ddfstrtReached = ddfstrt;
            computeDDFWindow();
            updateBplDma();
            scheduleNextBplEvent();
        }
    }
}

void
Agnus::setDDFSTOP(u16 old, u16 value)
{
    debug(DDF_DEBUG, "setDDFSTOP(%X, %X)\n", old, value);

    ddfstop = value;

    // Tell the hsync handler to recompute the DDF window
    hsyncActions |= HSYNC_PREDICT_DDF;

    // Take action if we haven't reached the old DDFSTOP cycle yet
     if (pos.h + 2 < ddfstopReached || ddfstopReached == -1) {

         // Check if the new position has already been passed
         if (ddfstop <= pos.h + 2) {

             // DDFSTOP won't match in the current rasterline
             ddfstopReached = -1;

         } else {

             // Update the matching position and recalculate the DMA table
             ddfstopReached = (ddfstop > HPOS_MAX) ? -1 : ddfstop;
             if (ddfstrtReached >= 0) {
                 computeDDFWindow();
                 updateBplDma();
                 scheduleNextBplEvent();
             }
         }
     }
}

void
Agnus::computeDDFWindow()
{
    isOCS() ? computeDDFWindowOCS() : computeDDFWindowECS();
}

void
Agnus::computeDDFWindowOCS()
{
    i16 strt = ddfstrtReached < 0 ? 0x18 : MAX(ddfstrtReached, 0x18);
    i16 stop = ddfstopReached < 0 ? 0xD8 : MIN(ddfstopReached, 0xD8);

    // Align ddfstrt at the start of the next fetch unit
    int loresShift = (8 - (strt & 0b111)) & 0b111;
    int hiresShift = (4 - (strt & 0b11)) & 0b11;

    // Compute the beginning of the DDF window
    ddfStrtLores = MAX(strt + loresShift, 0x18);
    ddfStrtHires = MAX(strt + hiresShift, 0x18);

    // Compute the number of fetch units
    int fetchUnits = ((stop - strt) + 15) >> 3;

    // Compute the end of the DDF window
    ddfStopLores = MIN(ddfStrtLores + 8 * fetchUnits, 0xE0);
    ddfStopHires = MIN(ddfStrtHires + 8 * fetchUnits, 0xE0);

    // Determine the final DDF state
    if (ddfstrtReached >= 0 && ddfstrtReached < 0x18) {
        ddfState = (ddfState == DDF_OFF) ? DDF_READY : DDF_OFF;
        debug("Toggle state ddfstrtReached = %d \n", ddfstrtReached);
    } else {
        ddfState = DDF_OFF;
    }

    debug(DDF_DEBUG, "DDF Window (OCS) Strt: %d %d\n", ddfStrtLores, ddfStrtHires);
    debug(DDF_DEBUG, "DDF Window (OCS) Stop: %d %d\n", ddfStrtLores, ddfStrtHires);
}

void
Agnus::computeDDFWindowECS()
{
    i16 strt = ddfstrtReached < 0 ? 0x18 : MAX(ddfstrtReached, 0x18);
    i16 stop = ddfstopReached < 0 ? 0xD8 : MIN(ddfstopReached, 0xD8);

    // Align ddfstrt at the start of the next fetch unit
    int loresShift = (8 - (strt & 0b111)) & 0b111;
    int hiresShift = (4 - (strt & 0b11)) & 0b11;

    // Compute the beginning of the DDF window
    ddfStrtLores = MAX(strt + loresShift, 0x18);
    ddfStrtHires = MAX(strt + hiresShift, 0x18);

    // Compute the number of fetch units
    int fetchUnits = ((stop - strt) + 15) >> 3;

    // Compute the end of the DDF window
    ddfStopLores = MIN(ddfStrtLores + 8 * fetchUnits, 0xE0);
    ddfStopHires = MIN(ddfStrtHires + 8 * fetchUnits, 0xE0);

    // Determine the final DDF state
    ddfState = DDF_OFF;

    debug(DDF_DEBUG, "DDF Window (ECS) Strt: %d %d\n", ddfStrtLores, ddfStrtHires);
    debug(DDF_DEBUG, "DDF Window (ECS) Stop: %d %d\n", ddfStrtLores, ddfStrtHires);
}

void
Agnus::predictDDF()
{
    i16 ddfStrtLoresOld = ddfStrtLores;
    i16 ddfStopLoresOld = ddfStopLores;
    i16 ddfStrtHiresOld = ddfStrtHires;
    i16 ddfStopHiresOld = ddfStopHires;
    DDFState ddfStateOld = ddfState;

    ddfstrtReached = ddfstrt;
    ddfstopReached = ddfstop;

    computeDDFWindow();

    if (ddfStrtLores != ddfStrtLoresOld || ddfStopLores != ddfStopLoresOld ||
        ddfStrtHires != ddfStrtHiresOld || ddfStopHires != ddfStopHiresOld ||
        ddfState != ddfStateOld) {

        hsyncActions |= HSYNC_UPDATE_BPL_TABLE; // Update the DMA slot
        hsyncActions |= HSYNC_PREDICT_DDF; // Call this function again
    }

    debug(DDF_DEBUG, "predictDDF LORES: %d %d\n", ddfStrtLores, ddfStopLores);
    debug(DDF_DEBUG, "predictDDF HIRES: %d %d\n", ddfStrtHires, ddfStopHires);
}

template <int x> void
Agnus::pokeBPLxPTH(u16 value)
{
    // debug(BPLREG_DEBUG, "pokeBPL%dPTH($%d) (%X)\n", x, value, value);
    // if (x == 1) plaindebug("pokeBPL%dPTH(%x)\n", x, value);

    // Check if the written value gets lost
    if (skipBPLxPT(x)) {
        // debug("BPLxPTH gets lost\n");
        return;
    }

    // Schedule the register updated
    switch (x) {
        case 1: recordRegisterChange(DMA_CYCLES(2), REG_BPL1PTH, value); break;
        case 2: recordRegisterChange(DMA_CYCLES(2), REG_BPL2PTH, value); break;
        case 3: recordRegisterChange(DMA_CYCLES(2), REG_BPL3PTH, value); break;
        case 4: recordRegisterChange(DMA_CYCLES(2), REG_BPL4PTH, value); break;
        case 5: recordRegisterChange(DMA_CYCLES(2), REG_BPL5PTH, value); break;
        case 6: recordRegisterChange(DMA_CYCLES(2), REG_BPL6PTH, value); break;
    }
}

template <int x> void
Agnus::pokeBPLxPTL(u16 value)
{
    // debug(BPLREG_DEBUG, "pokeBPL%dPTL(%d) ($%X)\n", x, value, value);
    // if (x == 1) plaindebug("pokeBPL%dPTL(%x)\n", x, value);

    // Check if the written value gets lost
    if (skipBPLxPT(x)) {
        debug(BPLREG_DEBUG, "BPLxPTL gets lost\n");
        return;
    }

    // Schedule the register updated
    switch (x) {
        case 1: recordRegisterChange(DMA_CYCLES(2), REG_BPL1PTL, value); break;
        case 2: recordRegisterChange(DMA_CYCLES(2), REG_BPL2PTL, value); break;
        case 3: recordRegisterChange(DMA_CYCLES(2), REG_BPL3PTL, value); break;
        case 4: recordRegisterChange(DMA_CYCLES(2), REG_BPL4PTL, value); break;
        case 5: recordRegisterChange(DMA_CYCLES(2), REG_BPL5PTL, value); break;
        case 6: recordRegisterChange(DMA_CYCLES(2), REG_BPL6PTL, value); break;
    }
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

    if (isBplxEvent(bplEvent[pos.h + 1], x)) { // (1)

        if (bplEvent[pos.h + 2] == EVENT_NONE) { // (2)

            // debug("skipBPLxPT: Value gets lost\n");
            // dumpBplEventTable();
            return true;
        }
    }

    return false;
}

template <int x> void
Agnus::setBPLxPTH(u16 value)
{
    debug(BPLREG_DEBUG, "setBPLxPTH(%d, %X)\n", x, value);
    bplpt[x - 1] = REPLACE_HI_WORD(bplpt[x - 1], value);
}

template <int x> void
Agnus::setBPLxPTL(u16 value)
{
    debug(BPLREG_DEBUG, "pokeBPLxPTL(%d, %X)\n", x, value);
    bplpt[x - 1] = REPLACE_LO_WORD(bplpt[x - 1], value & 0xFFFE);
}

void
Agnus::pokeBPL1MOD(u16 value)
{
    debug(BPLREG_DEBUG, "pokeBPL1MOD(%X)\n", value);
    recordRegisterChange(DMA_CYCLES(2), REG_BPL1MOD, value);
}

void
Agnus::setBPL1MOD(u16 value)
{
    debug(BPLREG_DEBUG, "setBPL1MOD(%X)\n", value);
    bpl1mod = (i16)(value & 0xFFFE);
}

void
Agnus::pokeBPL2MOD(u16 value)
{
    debug(BPLREG_DEBUG, "pokeBPL2MOD(%X)\n", value);
    recordRegisterChange(DMA_CYCLES(2), REG_BPL2MOD, value);
}

void
Agnus::setBPL2MOD(u16 value)
{
    debug(BPLREG_DEBUG, "setBPL2MOD(%X)\n", value);
    bpl2mod = (i16)(value & 0xFFFE);
}

template <int x> void
Agnus::pokeSPRxPTH(u16 value)
{
    debug(SPRREG_DEBUG, "pokeSPR%dPTH(%X)\n", x, value);
    sprpt[x] = REPLACE_HI_WORD(sprpt[x], value);
}

template <int x> void
Agnus::pokeSPRxPTL(u16 value)
{
    debug(SPRREG_DEBUG, "pokeSPR%dPTL(%X)\n", x, value);
    sprpt[x] = REPLACE_LO_WORD(sprpt[x], value & 0xFFFE);
}

template <int x> void
Agnus::pokeSPRxPOS(u16 value)
{
    debug(SPRREG_DEBUG, "pokeSPR%dPOS(%X)\n", x, value);

    // Compute the value of the vertical counter that is seen here
    i16 v = (pos.h < 0xDF) ? pos.v : (pos.v + 1);

    // Compute the new vertical start position
    sprVStrt[x] = ((value & 0xFF00) >> 8) | (sprVStrt[x] & 0x0100);

    // Update sprite DMA status
    if (sprVStrt[x] == v) sprDmaState[x] = SPR_DMA_ACTIVE;
    if (sprVStop[x] == v) sprDmaState[x] = SPR_DMA_IDLE;
}

template <int x> void
Agnus::pokeSPRxCTL(u16 value)
{
    debug(SPRREG_DEBUG, "pokeSPR%dCTL(%X)\n", x, value);

    // Compute the value of the vertical counter that is seen here
    i16 v = (pos.h < 0xDF) ? pos.v : (pos.v + 1);

    // bool match = (sprVStop[x] == v);
    // debug("v = %d match = %d\n", v, match);

    // Compute the new vertical start and stop position
    sprVStrt[x] = ((value & 0b100) << 6) | (sprVStrt[x] & 0x00FF);
    sprVStop[x] = ((value & 0b010) << 7) | (value >> 8);

    // Check if sprite DMA should be enabled
    // Check if sprite DMA should be disabled
    /*
    if (match || sprVStop[x] == v) {
        sprDmaState[x] = SPR_DMA_IDLE;
        // debug("Going IDLE\n");
    }
    */

    // Update sprite DMA status
    if (sprVStrt[x] == v) sprDmaState[x] = SPR_DMA_ACTIVE;
    if (sprVStop[x] == v) sprDmaState[x] = SPR_DMA_IDLE;
}

void
Agnus::pokeBPLCON0(u16 value)
{
    debug(DMA_DEBUG, "pokeBPLCON0(%X)\n", value);

    if (bplcon0 != value) {
        recordRegisterChange(DMA_CYCLES(4), REG_BPLCON0_AGNUS, value);
    }
}

void
Agnus::setBPLCON0(u16 oldValue, u16 newValue)
{
    assert(oldValue != newValue);

    debug(DMA_DEBUG, "pokeBPLCON0(%X,%X)\n", oldValue, newValue);

    // Update variable bplcon0AtDDFStrt if DDFSTRT has not been reached yet
    if (pos.h < ddfstrtReached) bplcon0AtDDFStrt = newValue;

    // Update the bpl event table in the next rasterline
    hsyncActions |= HSYNC_UPDATE_BPL_TABLE;

    // Check if the hires bit or one of the BPU bits have been modified
    if ((oldValue ^ newValue) & 0xF000) {

        /*
        debug("oldBplcon0 = %X newBplcon0 = %X\n", oldBplcon0, newBplcon0);
        dumpBplEventTable();
         */

        /* TODO:
         * BPLCON0 is usually written in each frame.
         * To speed up, just check the hpos. If it is smaller than the start
         * of the DMA window, a standard update() is enough and the scheduled
         * update in hsyncActions (HSYNC_UPDATE_BPL_TABLE) can be omitted.
         */

        // Update the DMA allocation table
        allocateBplSlots(dmacon, newValue, pos.h);

        // Since the table has changed, we also need to update the event slot
        scheduleBplEventForCycle(pos.h);
    }

    bplcon0 = newValue;
}

int
Agnus::bpu(u16 v)
{
    // Extract the three BPU bits and check for hires mode
    int bpu = (v >> 12) & 0b111;
    bool hires = GET_BIT(v, 15);

    if (hires) {
        return bpu < 5 ? bpu : 0; // Disable all channels if value is invalid
    } else {
        return bpu < 7 ? bpu : 4; // Enable four channels if value is invalid
    }
}

void
Agnus::execute()
{
    // Process pending events
    if (nextTrigger <= clock) {
        executeEventsUntil(clock);
    } else {
        assert(pos.h < 0xE2);
    }

    // Advance the internal clock and the horizontal counter
    clock += DMA_CYCLES(1);

    // pos.h++;
    assert(pos.h <= HPOS_MAX);
    pos.h = pos.h < HPOS_MAX ? pos.h + 1 : 0; // (pos.h + 1) % HPOS_CNT;

    // If this assertion hits, the HSYNC event hasn't been served
    /*
    if (pos.h > HPOS_CNT) {
        dump();
        dumpBplEventTable();
    }
    */
    assert(pos.h <= HPOS_CNT);
}

#ifdef AGNUS_EXEC_DEBUG

void
Agnus::executeUntil(Cycle targetClock)
{
    // Align to DMA cycle raster
    targetClock &= ~0b111;

    // Compute the number of DMA cycles to execute
    DMACycle dmaCycles = (targetClock - clock) / DMA_CYCLES(1);

    // Execute DMA cycles one after another
    for (DMACycle i = 0; i < dmaCycles; i++) execute();
}

#else

void
Agnus::executeUntil(Cycle targetClock)
{
    // Align to DMA cycle raster
    targetClock &= ~0b111;

    // Compute the number of DMA cycles to execute
    DMACycle dmaCycles = (targetClock - clock) / DMA_CYCLES(1);

    if (targetClock < nextTrigger && dmaCycles > 0) {

        // Advance directly to the target clock
        clock = targetClock;
        pos.h += dmaCycles;

        // If this assertion hits, the HSYNC event hasn't been served
        assert(pos.h <= HPOS_CNT);

    } else {

        // Execute DMA cycles one after another
        for (DMACycle i = 0; i < dmaCycles; i++) execute();
    }
}
#endif

void
Agnus::executeUntilBusIsFree()
{
    i16 posh = pos.h == 0 ? HPOS_MAX : pos.h - 1;

    // Check if the bus is blocked
    if (busOwner[posh] != BUS_NONE) {

        // This variable counts the number of DMA cycles the CPU will be suspended
        DMACycle delay = 0;

        // Execute Agnus until the bus is free
        do {

            posh = pos.h;
            execute();
            if (++delay == 2) bls = true;

        } while (busOwner[posh] != BUS_NONE);

        // Clear the BLS line (Blitter slow down)
        bls = false;

        // Add wait states to the CPU
        cpu.addWaitStates(AS_CPU_CYCLES(DMA_CYCLES(delay)));
    }

    // Assign bus to the CPU
    busOwner[posh] = BUS_CPU;
}

void
Agnus::recordRegisterChange(Cycle delay, u32 addr, u16 value)
{
    // Record the new register value
    changeRecorder.add(clock + delay, addr, value);
    chngRecorder.insert(clock + delay, RegChange { addr, value} );
    
    // Schedule the register change
    scheduleNextREGEvent();
}

void
Agnus::updateRegisters()
{
}

template <int nr> void
Agnus::executeFirstSpriteCycle()
{
    debug(SPR_DEBUG, "executeFirstSpriteCycle<%d>\n", nr);

    if (pos.v == sprVStop[nr]) {

        sprDmaState[nr] = SPR_DMA_IDLE;

        // Read in the next control word (POS part)
        u16 value = doSpriteDMA<nr>();
        agnus.pokeSPRxPOS<nr>(value);
        denise.pokeSPRxPOS<nr>(value);

    } else if (sprDmaState[nr] == SPR_DMA_ACTIVE) {

        // Read in the next data word (part A)
        u16 value = doSpriteDMA<nr>();
        denise.pokeSPRxDATA<nr>(value);
    }
}

template <int nr> void
Agnus::executeSecondSpriteCycle()
{
    debug(SPR_DEBUG, "executeSecondSpriteCycle<%d>\n", nr);

    if (pos.v == sprVStop[nr]) {

        sprDmaState[nr] = SPR_DMA_IDLE;

        // Read in the next control word (CTL part)
        u16 value = doSpriteDMA<nr>();
        agnus.pokeSPRxCTL<nr>(value);
        denise.pokeSPRxCTL<nr>(value);

    } else if (sprDmaState[nr] == SPR_DMA_ACTIVE) {

        // Read in the next data word (part B)
        u16 value = doSpriteDMA<nr>();
        denise.pokeSPRxDATB<nr>(value);
    }
}

void
Agnus::updateSpriteDMA()
{
    // debug("updateSpriteDMA()\n");

    // When this function is called, the sprite logic already sees an inremented
    // vertical position counter.
    i16 v = pos.v + 1;

    // Reset the vertical trigger coordinates in line 25
    if (v == 25 && sprdma()) {
        for (int i = 0; i < 8; i++) sprVStop[i] = 25;
        return;
     }

    // Disable DMA in the last rasterline
    if (v == frameInfo.numLines - 1) {
        for (int i = 0; i < 8; i++) sprDmaState[i] = SPR_DMA_IDLE;
        return;
    }

    // Update the DMA status for all sprites
    for (int i = 0; i < 8; i++) {
        if (v == sprVStrt[i]) sprDmaState[i] = SPR_DMA_ACTIVE;
        if (v == sprVStop[i]) sprDmaState[i] = SPR_DMA_IDLE;
    }
}

void
Agnus::hsyncHandler()
{
    assert(pos.h == 0 || pos.h == HPOS_MAX + 1);

    // Call the hsync handlers of Denise and Paula
    denise.endOfLine(pos.v);

    // Synthesize sound samples
    audioUnit.executeUntil(clock - 50 * DMA_CYCLES(HPOS_CNT));

    // Update pot counters
    if (paula.chargeX0 < 1.0) paula.potCntX0++;
    if (paula.chargeY0 < 1.0) paula.potCntY0++;
    if (paula.chargeX1 < 1.0) paula.potCntX1++;
    if (paula.chargeY1 < 1.0) paula.potCntY1++;

    // Let CIA B count the HSYNCs
    amiga.ciaB.incrementTOD();

    // Reset the horizontal counter
    pos.h = 0;

    // Advance the vertical counter
    if (++pos.v >= frameInfo.numLines) vsyncHandler();

    // Initialize variables which keep values for certain trigger positions
    dmaconAtDDFStrt = dmacon;
    bplcon0AtDDFStrt = bplcon0;


    //
    // DIW
    //

    if (pos.v == diwVstrt && !diwVFlop) {
        diwVFlop = true;
        debug(DIW_DEBUG, "diwVFlop = %d\n", diwVFlop);
        // updateBplDma();
    }
    if (pos.v == diwVstop && diwVFlop) {
        diwVFlop = false;
        debug(DIW_DEBUG, "diwVFlop = %d\n", diwVFlop);
        // updateBplDma();
    }

    // Horizontal DIW flipflop
    diwHFlop = (diwHFlopOff != -1) ? false : (diwHFlopOn != -1) ? true : diwHFlop;
    diwHFlopOn = diwHstrt;
    diwHFlopOff = diwHstop;


    //
    // DDF
    //

    // Update the vertical DDF flipflop
    ddfVFlop = !inLastRasterline() && diwVFlop;


    //
    // Determine the bitplane DMA status for the line to come
    //

    bool bplDmaLine = inBplDmaLine();

    // Update the bpl event table if the value has changed
    if (bplDmaLine ^ oldBplDmaLine) {
        hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
        oldBplDmaLine = bplDmaLine;
    }


    //
    // Determine the disk, audio and sprite DMA status for the line to come
    //

    u16 newDmaDAS;

    if (dmacon & DMAEN) {

        // Copy DMA enable bits from dmacon
        newDmaDAS = dmacon & 0b111111;

        // Disable sprites outside the sprite DMA area
        if (pos.v < 25 || pos.v >= frameInfo.numLines - 1) newDmaDAS &= 0b011111;

    } else {

        newDmaDAS = 0;
    }

    if (dmaDAS != newDmaDAS) hsyncActions |= HSYNC_UPDATE_DAS_TABLE;
    dmaDAS = newDmaDAS;

    //
    // Process pending work items
    //

    if (hsyncActions) {

        if (hsyncActions & HSYNC_PREDICT_DDF) {
            hsyncActions &= ~HSYNC_PREDICT_DDF;
            predictDDF();
        }
        if (hsyncActions & HSYNC_UPDATE_BPL_TABLE) {
            hsyncActions &= ~HSYNC_UPDATE_BPL_TABLE;
            updateBplDma();
        }
        if (hsyncActions & HSYNC_UPDATE_DAS_TABLE) {
            hsyncActions &= ~HSYNC_UPDATE_DAS_TABLE;
            updateDasDma(dmaDAS);
        }
    }

    // Clear the bus usage table
    for (int i = 0; i < HPOS_CNT; i++) busOwner[i] = BUS_NONE;

    // Schedule the first BPL and DAS events
    scheduleNextBplEvent();
    scheduleNextDasEvent();


    //
    // Let other components prepare for the next line
    //

    denise.beginOfLine(pos.v);
}

void
Agnus::vsyncHandler()
{
    // debug("diwVstrt = %d diwVstop = %d diwHstrt = %d diwHstop = %d\n", diwVstrt, diwVstop, diwHstrt, hstop);

    // Advance to the next frame
    frameInfo.nr++;

    // Check if we the next frame is drawn in interlace mode
    frameInfo.interlaced = denise.lace();

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
    amiga.ciaA.incrementTOD();
        
    // Let other subcomponents do their own VSYNC stuff
    blitter.vsyncHandler();
    copper.vsyncHandler();
    denise.beginOfFrame(frameInfo.interlaced);
    diskController.vsyncHandler();
    joystick1.execute();
    joystick2.execute();

    // Update statistics
    amiga.updateStats();

    // Prepare to take a snapshot once in a while
    if (amiga.snapshotIsDue()) amiga.signalSnapshot();

    // Count some sheep (zzzzzz) ...
    if (!amiga.getWarp()) {
        amiga.synchronizeTiming();
    }
}

void
Agnus::serviceVblEvent()
{
    assert(slot[VBL_SLOT].id == VBL_STROBE);
    assert(pos.v == 0 || pos.v == 1);
    assert(pos.h == 1);

    paula.setINTREQ(true, 1 << INT_VERTB);
    rescheduleRel<VBL_SLOT>(cyclesInFrame());
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

template void Agnus::pokeBPLxPTH<1>(u16 value);
template void Agnus::pokeBPLxPTH<2>(u16 value);
template void Agnus::pokeBPLxPTH<3>(u16 value);
template void Agnus::pokeBPLxPTH<4>(u16 value);
template void Agnus::pokeBPLxPTH<5>(u16 value);
template void Agnus::pokeBPLxPTH<6>(u16 value);
template void Agnus::setBPLxPTH<1>(u16 value);
template void Agnus::setBPLxPTH<2>(u16 value);
template void Agnus::setBPLxPTH<3>(u16 value);
template void Agnus::setBPLxPTH<4>(u16 value);
template void Agnus::setBPLxPTH<5>(u16 value);
template void Agnus::setBPLxPTH<6>(u16 value);

template void Agnus::pokeBPLxPTL<1>(u16 value);
template void Agnus::pokeBPLxPTL<2>(u16 value);
template void Agnus::pokeBPLxPTL<3>(u16 value);
template void Agnus::pokeBPLxPTL<4>(u16 value);
template void Agnus::pokeBPLxPTL<5>(u16 value);
template void Agnus::pokeBPLxPTL<6>(u16 value);
template void Agnus::setBPLxPTL<1>(u16 value);
template void Agnus::setBPLxPTL<2>(u16 value);
template void Agnus::setBPLxPTL<3>(u16 value);
template void Agnus::setBPLxPTL<4>(u16 value);
template void Agnus::setBPLxPTL<5>(u16 value);
template void Agnus::setBPLxPTL<6>(u16 value);

template void Agnus::pokeSPRxPTH<0>(u16 value);
template void Agnus::pokeSPRxPTH<1>(u16 value);
template void Agnus::pokeSPRxPTH<2>(u16 value);
template void Agnus::pokeSPRxPTH<3>(u16 value);
template void Agnus::pokeSPRxPTH<4>(u16 value);
template void Agnus::pokeSPRxPTH<5>(u16 value);
template void Agnus::pokeSPRxPTH<6>(u16 value);
template void Agnus::pokeSPRxPTH<7>(u16 value);

template void Agnus::pokeSPRxPTL<0>(u16 value);
template void Agnus::pokeSPRxPTL<1>(u16 value);
template void Agnus::pokeSPRxPTL<2>(u16 value);
template void Agnus::pokeSPRxPTL<3>(u16 value);
template void Agnus::pokeSPRxPTL<4>(u16 value);
template void Agnus::pokeSPRxPTL<5>(u16 value);
template void Agnus::pokeSPRxPTL<6>(u16 value);
template void Agnus::pokeSPRxPTL<7>(u16 value);

template void Agnus::pokeSPRxPOS<0>(u16 value);
template void Agnus::pokeSPRxPOS<1>(u16 value);
template void Agnus::pokeSPRxPOS<2>(u16 value);
template void Agnus::pokeSPRxPOS<3>(u16 value);
template void Agnus::pokeSPRxPOS<4>(u16 value);
template void Agnus::pokeSPRxPOS<5>(u16 value);
template void Agnus::pokeSPRxPOS<6>(u16 value);
template void Agnus::pokeSPRxPOS<7>(u16 value);

template void Agnus::pokeSPRxCTL<0>(u16 value);
template void Agnus::pokeSPRxCTL<1>(u16 value);
template void Agnus::pokeSPRxCTL<2>(u16 value);
template void Agnus::pokeSPRxCTL<3>(u16 value);
template void Agnus::pokeSPRxCTL<4>(u16 value);
template void Agnus::pokeSPRxCTL<5>(u16 value);
template void Agnus::pokeSPRxCTL<6>(u16 value);
template void Agnus::pokeSPRxCTL<7>(u16 value);

template u16 Agnus::doBitplaneDMA<0>();
template u16 Agnus::doBitplaneDMA<1>();
template u16 Agnus::doBitplaneDMA<2>();
template u16 Agnus::doBitplaneDMA<3>();
template u16 Agnus::doBitplaneDMA<4>();
template u16 Agnus::doBitplaneDMA<5>();

template u16 Agnus::doAudioDMA<0>();
template u16 Agnus::doAudioDMA<1>();
template u16 Agnus::doAudioDMA<2>();
template u16 Agnus::doAudioDMA<3>();

template void Agnus::pokeDIWSTRT<POKE_CPU>(u16 value);
template void Agnus::pokeDIWSTRT<POKE_COPPER>(u16 value);
template void Agnus::pokeDIWSTOP<POKE_CPU>(u16 value);
template void Agnus::pokeDIWSTOP<POKE_COPPER>(u16 value);

template bool Agnus::allocateBus<BUS_COPPER>();
template bool Agnus::allocateBus<BUS_BLITTER>();

template bool Agnus::busIsFree<BUS_COPPER>();
template bool Agnus::busIsFree<BUS_BLITTER>();
