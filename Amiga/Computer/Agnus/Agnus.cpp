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
    
    registerSubcomponents(vector<HardwareComponent *> {
        
        &copper,
        &blitter,
        &dmaDebugger
    });
    
    registerSnapshotItems(vector<SnapshotItem> {

        { &slot,            sizeof(slot),            BYTE_ARRAY },
        { &nextTrigger,     sizeof(nextTrigger),     0 },

        { &clock,                sizeof(clock),                0 },
        { &frame,                sizeof(frame),                0 },
        { &vpos,                 sizeof(vpos),                 0 },
        { &hpos,                 sizeof(hpos),                 0 },
        { &frameInfo.nr,         sizeof(frameInfo.nr),         0 },
        { &frameInfo.interlaced, sizeof(frameInfo.interlaced), 0 },
        { &frameInfo.numLines,   sizeof(frameInfo.numLines),   0 },
        { &lof,                  sizeof(lof),                  0 },
        { &dmaStrtLores,         sizeof(dmaStrtLores),         0 },
        { &dmaStrtHires,         sizeof(dmaStrtHires),         0 },
        { &dmaStopLores,         sizeof(dmaStopLores),         0 },
        { &dmaStopHires,         sizeof(dmaStopHires),         0 },
        { &dmaStrt,              sizeof(dmaStrt),              0 },
        { &dmaStop,              sizeof(dmaStop),              0 },
        { &sprVStrt,             sizeof(sprVStrt),             WORD_ARRAY },
        { &sprVStop,             sizeof(sprVStop),             WORD_ARRAY },
        { &sprDmaState,          sizeof(sprDmaState),          DWORD_ARRAY },
        { &diwHstrt,             sizeof(diwHstrt),             0 },
        { &diwHstop,             sizeof(diwHstop),             0 },
        { &diwVstrt,             sizeof(diwVstrt),             0 },
        { &diwVstop,             sizeof(diwVstop),             0 },
        { &vFlop,                sizeof(vFlop),                0 },
        { &hFlop,                sizeof(hFlop),                0 },
        { &hFlopOn,              sizeof(hFlopOn),              0 },
        { &hFlopOff,             sizeof(hFlopOff),             0 },
        { &dmacon,               sizeof(dmacon),               0 },
        { &dskpt,                sizeof(dskpt),                0 },
        { &diwstrt,              sizeof(diwstrt),              0 },
        { &diwstop,              sizeof(diwstop),              0 },
        { &ddfstrt,              sizeof(ddfstrt),              0 },
        { &ddfstop,              sizeof(ddfstop),              0 },
        { &ddfstopAligned,              sizeof(ddfstopAligned),              0 },
        { &ddfstrtAligned,              sizeof(ddfstrtAligned),              0 },
        { &ddfstopAligned,              sizeof(ddfstopAligned),              0 },
        { &audlc,                sizeof(audlc),                DWORD_ARRAY },
        { &audlcold,             sizeof(audlcold),             DWORD_ARRAY },
        { &bplpt,                sizeof(bplpt),                DWORD_ARRAY },
        { &bpl1mod,              sizeof(bpl1mod),              0 },
        { &bpl2mod,              sizeof(bpl2mod),              0 },
        { &sprpt,                sizeof(sprpt),                DWORD_ARRAY },
        { &activeBitplanes,      sizeof(activeBitplanes),      0 },
        { &dmaEvent,             sizeof(dmaEvent),             0 },
        { &nextDmaEvent,         sizeof(nextDmaEvent),         0 },
        { &dmaFirstBpl1Event,    sizeof(dmaFirstBpl1Event),    0 },
        { &dmaLastBpl1Event,     sizeof(dmaLastBpl1Event),     0 },
        { &hsyncActions,         sizeof(hsyncActions),         0 }
    });

    initLookupTables();
}

void
Agnus::initLookupTables()
{
    initLoresBplEventTable();
    initHiresBplEventTable();
}

void
Agnus::initLoresBplEventTable()
{
    memset(bitplaneDMA[0], 0, sizeof(bitplaneDMA[0]));

    for (int bpu = 0; bpu < 7; bpu++) {

        // Goto the first bitplane DMA slot
        EventID *p = &bitplaneDMA[0][bpu][0x30];

        // Iterate through all 22 fetch units
        for (int unit = 0; unit < 22; unit++, p += 8) {

            switch(bpu) {
                case 6: p[2] = DMA_L6;
                case 5: p[6] = DMA_L5;
                case 4: p[1] = DMA_L4;
                case 3: p[5] = DMA_L3;
                case 2: p[3] = DMA_L2;
                case 1: p[7] = DMA_L1;
            }
        }
    }
}

void
Agnus::initHiresBplEventTable()
{
    memset(bitplaneDMA[1], 0, sizeof(bitplaneDMA[1]));

    for (int bpu = 0; bpu < 7; bpu++) {

        // Goto the first bitplane DMA slot
        EventID *p = &bitplaneDMA[1][bpu][0x30];

        // Iterate through all 22 fetch units
        for (int unit = 0; unit < 22; unit++, p += 8) {

            switch(bpu) {
                case 6:
                case 5:
                case 4: p[0] = p[4] = DMA_H4;
                case 3: p[2] = p[6] = DMA_H3;
                case 2: p[1] = p[5] = DMA_H2;
                case 1: p[3] = p[7] = DMA_H1;
            }
        }
    }
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

    // Schedule the first SYNC event
    scheduleAbs<SYNC_SLOT>(DMA_CYCLES(HPOS_MAX), SYNC_H);

    // Schedule the first CIA A and CIA B events
    scheduleAbs<CIAA_SLOT>(CIA_CYCLES(1), CIA_EXECUTE);
    scheduleAbs<CIAB_SLOT>(CIA_CYCLES(1), CIA_EXECUTE);
}

void
Agnus::_powerOff()
{
    
}
void
Agnus::_reset()
{
    
}
void
Agnus::_ping()
{
    
}

void
Agnus::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.dmacon  = dmacon;
    info.diwstrt = diwstrt;
    info.diwstop = diwstop;
    info.ddfstrt = ddfstrtAligned;
    info.ddfstop = ddfstopAligned;
    
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
    return clock - DMA_CYCLES(vpos * DMACyclesPerLine() + hpos);
}

Cycle
Agnus::startOfNextFrame()
{
    return startOfCurrentFrame() + cyclesInCurrentFrame();
}

bool
Agnus::inBplDmaArea() {
    return
    vFlop                             // Vertical DIW flipflop
    && vpos >= 26                     // Outside VBLANK area
    && vpos < frameInfo.numLines - 1  // Not in last line of frame
    && denise->bplconBPU()            // At least one bitplane enabled
    && bplDMA();                      // Bitplane DMA enabled
}

Beam
Agnus::beamPosition()
{
    Beam result;

    result.y = vpos;
    result.x = hpos;

    return result;
}

Cycle
Agnus::beamToCycle(Beam beam)
{
    return startOfCurrentFrame() + DMA_CYCLES(beam.y * DMACyclesPerLine() + beam.x);
}

Beam
Agnus::cycleToBeam(Cycle cycle)
{
    Beam result;

    Cycle diff = AS_DMA_CYCLES(cycle - startOfCurrentFrame());
    assert(diff >= 0);

    result.y = diff / DMACyclesPerLine();
    result.x = diff % DMACyclesPerLine();
    return result;
}

Beam
Agnus::addToBeam(Beam beam, Cycle cycles)
{
    Beam result;

    Cycle cycle = beam.y * DMACyclesPerLine() + beam.x + cycles;
    result.y = cycle / DMACyclesPerLine();
    result.x = cycle % DMACyclesPerLine();

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
    
    // In PAL mode, all lines have the same length (227 color clocks)
    return DMA_CYCLES(vDiff * 227 + hDiff);
}

bool
Agnus::copperCanHaveBus()
{
    // Deny access if Copper DMA is disabled
    if ((dmacon & (DMAEN|COPEN)) != (DMAEN|COPEN)) return false;

    // Deny access if the current slot is used for bitplane DMA
    if (isBplEvent(dmaEvent[hpos])) {
        debug(COP_DEBUG, "Copper blocked by bitplane DMA\n");
        return false;
    }

    // Grant access
    return true;
}

uint16_t
Agnus::doDiskDMA()
{
    uint16_t result = mem->peekChip16(dskpt);
    INC_DMAPTR(dskpt);

    busOwner[hpos] = BUS_DISK;
    busValue[hpos] = result;

    return result;
}

void
Agnus::doDiskDMA(uint16_t value)
{
    mem->pokeChip16(dskpt, value);
    INC_DMAPTR(dskpt);

    busOwner[hpos] = BUS_DISK;
    busValue[hpos] = value;
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

    busOwner[hpos] = BUS_SPRITE;
    busValue[hpos] = result;

    return result;
}

uint16_t
Agnus::doSpriteDMA(int channel)
{
    uint16_t result = mem->peekChip16(sprpt[channel]);
    INC_DMAPTR(sprpt[channel]);

    busOwner[hpos] = BUS_SPRITE;
    busValue[hpos] = result;

    return result; 
}

template <int channel> uint16_t
Agnus::doBitplaneDMA()
{
    uint16_t result = mem->peekChip16(bplpt[channel]);
    INC_DMAPTR(bplpt[channel]);

    busOwner[hpos] = BUS_BITPLANE;
    busValue[hpos] = result;

    return result;
}

uint16_t
Agnus::copperRead(uint32_t addr)
{
    uint16_t result = mem->peek16(addr);

    busOwner[hpos] = BUS_COPPER;
    busValue[hpos] = result;

    return result;
}

void
Agnus::copperWrite(uint32_t addr, uint16_t value)
{
    mem->pokeCustom16(addr, value);

    busOwner[hpos] = BUS_COPPER;
    busValue[hpos] = value;
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

    if (first < 0x18 || first > 0xDF) return;
    if (last < 0x18 || last > 0xDF) return;

    // Update events
    for (unsigned i = first; i <= last; i++) {
        dmaEvent[i] = bitplaneDMA[hires][bpu][i];
    }

    // Update jump table
    updateJumpTable();
}

void
Agnus::allocateBplSlots(int bpu, bool hires, int first)
{
    // allocateBplSlots(bpu, hires, MAX(first, dmaStrt), dmaStop);
    if (hires) {
        allocateBplSlots(bpu, hires, MAX(first, dmaStrtHires), dmaStopHires - 1);
    } else {
        allocateBplSlots(bpu, hires, MAX(first, dmaStrtLores), dmaStopLores - 1);
    }
}

void
Agnus::switchDiskDmaOn()
{
    dmaEvent[0x07] = DMA_DISK;
    dmaEvent[0x09] = DMA_DISK;
    dmaEvent[0x0B] = DMA_DISK;
    updateJumpTable(0x0B);
}

void
Agnus::switchDiskDmaOff()
{
    dmaEvent[0x07] = EVENT_NONE;
    dmaEvent[0x09] = EVENT_NONE;
    dmaEvent[0x0B] = EVENT_NONE;
    updateJumpTable(0x0B);
}

void
Agnus::switchAudioDmaOn(int channel)
{
    switch (channel) {
        
        case 0: dmaEvent[0x0D] = DMA_A0; break;
        case 1: dmaEvent[0x0F] = DMA_A1; break;
        case 2: dmaEvent[0x11] = DMA_A2; break;
        case 3: dmaEvent[0x13] = DMA_A3; break;
        
        default: assert(false);
    }
    
    updateJumpTable(0x13);
}

void
Agnus::switchAudioDmaOff(int channel)
{
    switch (channel) {
        
        case 0: dmaEvent[0x0D] = EVENT_NONE; break;
        case 1: dmaEvent[0x0F] = EVENT_NONE; break;
        case 2: dmaEvent[0x11] = EVENT_NONE; break;
        case 3: dmaEvent[0x13] = EVENT_NONE; break;
        
        default: assert(false);
    }
    
    updateJumpTable(0x13);
}

void
Agnus::switchSpriteDmaOn()
{
    dmaEvent[0x15] = DMA_S0_1;
    dmaEvent[0x17] = DMA_S0_2;

    /* Note: Except for sprite 0, sprite DMA and bitplane DMA may overlap.
     * Bitplane DMA has priority over sprite DMA.
     */
    if (dmaEvent[0x19] == EVENT_NONE) dmaEvent[0x19] = DMA_S1_1;
    if (dmaEvent[0x1B] == EVENT_NONE) dmaEvent[0x1B] = DMA_S1_2;
    if (dmaEvent[0x1D] == EVENT_NONE) dmaEvent[0x1D] = DMA_S2_1;
    if (dmaEvent[0x1F] == EVENT_NONE) dmaEvent[0x1F] = DMA_S2_2;
    if (dmaEvent[0x21] == EVENT_NONE) dmaEvent[0x21] = DMA_S3_1;
    if (dmaEvent[0x23] == EVENT_NONE) dmaEvent[0x23] = DMA_S3_2;
    if (dmaEvent[0x25] == EVENT_NONE) dmaEvent[0x25] = DMA_S4_1;
    if (dmaEvent[0x27] == EVENT_NONE) dmaEvent[0x27] = DMA_S4_2;
    if (dmaEvent[0x29] == EVENT_NONE) dmaEvent[0x29] = DMA_S5_1;
    if (dmaEvent[0x2B] == EVENT_NONE) dmaEvent[0x2B] = DMA_S5_2;
    if (dmaEvent[0x2D] == EVENT_NONE) dmaEvent[0x2D] = DMA_S6_1;
    if (dmaEvent[0x2F] == EVENT_NONE) dmaEvent[0x2F] = DMA_S6_2;
    if (dmaEvent[0x31] == EVENT_NONE) dmaEvent[0x31] = DMA_S7_1;
    if (dmaEvent[0x33] == EVENT_NONE) dmaEvent[0x33] = DMA_S7_2;
    
    updateJumpTable(0x33);
}

void
Agnus::switchSpriteDmaOff()
{
    dmaEvent[0x15] = EVENT_NONE;
    dmaEvent[0x17] = EVENT_NONE;
    
    /* Note: Except for sprite 0, sprite DMA and bitplane DMA may overlap.
     * Bitplane DMA has priority over sprite DMA.
     */
    if (dmaEvent[0x19] == DMA_S1_1) dmaEvent[0x19] = EVENT_NONE;
    if (dmaEvent[0x1B] == DMA_S1_2) dmaEvent[0x1B] = EVENT_NONE;
    if (dmaEvent[0x1D] == DMA_S2_1) dmaEvent[0x1D] = EVENT_NONE;
    if (dmaEvent[0x1F] == DMA_S2_2) dmaEvent[0x1F] = EVENT_NONE;
    if (dmaEvent[0x21] == DMA_S3_1) dmaEvent[0x21] = EVENT_NONE;
    if (dmaEvent[0x23] == DMA_S3_2) dmaEvent[0x23] = EVENT_NONE;
    if (dmaEvent[0x25] == DMA_S4_1) dmaEvent[0x25] = EVENT_NONE;
    if (dmaEvent[0x27] == DMA_S4_2) dmaEvent[0x27] = EVENT_NONE;
    if (dmaEvent[0x29] == DMA_S5_1) dmaEvent[0x29] = EVENT_NONE;
    if (dmaEvent[0x2B] == DMA_S5_2) dmaEvent[0x2B] = EVENT_NONE;
    if (dmaEvent[0x2D] == DMA_S6_1) dmaEvent[0x2D] = EVENT_NONE;
    if (dmaEvent[0x2F] == DMA_S6_2) dmaEvent[0x2F] = EVENT_NONE;
    if (dmaEvent[0x31] == DMA_S7_1) dmaEvent[0x31] = EVENT_NONE;
    if (dmaEvent[0x33] == DMA_S7_2) dmaEvent[0x33] = EVENT_NONE;
    
    updateJumpTable(0x33);
}

void
Agnus::switchBitplaneDmaOn()
{
    // Clear slots first (TODO: THIS IS UGLY AND SLOW)
    switchBitplaneDmaOff();

    debug(BPL_DEBUG, "switchBitplaneDmaOn: bpu = %d\n", denise->bplconBPU());

    if (denise->hires()) {

        /*
        // Determine start and stop cycle
        uint16_t start = ddfstrtAligned;
        uint16_t stop  = ddfstopAligned;
        
        // Align stop such that (stop - start) is dividable by 8
        stop += (stop - start) & 0b100;
        */
        int16_t start = dmaStrtHires;
        int16_t stop = dmaStopHires;

        // Determine event IDs
        EventID h4 = (activeBitplanes >= 4) ? DMA_H4 : EVENT_NONE;
        EventID h3 = (activeBitplanes >= 3) ? DMA_H3 : EVENT_NONE;
        EventID h2 = (activeBitplanes >= 2) ? DMA_H2 : EVENT_NONE;
        EventID h1 = (activeBitplanes >= 1) ? DMA_H1 : EVENT_NONE;
        
        // Schedule events
        for (unsigned i = start; i < stop; i += 8) {
            dmaEvent[i]   = dmaEvent[i+4] = h4;
            dmaEvent[i+1] = dmaEvent[i+5] = h2;
            dmaEvent[i+2] = dmaEvent[i+6] = h3;
            dmaEvent[i+3] = dmaEvent[i+7] = h1;
        }

        // Remember start / stop positions
        if (dmaEvent[start+3] != EVENT_NONE) {

            dmaFirstBpl1Event = start + 3;
            dmaLastBpl1Event = stop - 1;

            assert(dmaEvent[dmaFirstBpl1Event] == DMA_H1);
            assert(dmaEvent[dmaLastBpl1Event] == DMA_H1);

            denise->firstCanvasPixel = (dmaFirstBpl1Event * 4) + 6;
            denise->lastCanvasPixel = (dmaLastBpl1Event * 4) + 6 + 15;

        } else {

            dmaFirstBpl1Event = 0;
            dmaLastBpl1Event = 0;
            denise->firstCanvasPixel = 0;
            denise->lastCanvasPixel = 0;
        }

        /*
        debug("HIRES ddfstrt = %X ddfstop = %X dmaStrt = %X dmaStop = %X\n",
              ddfstrt, ddfstop, dmaStrtHires, dmaStopHires);
        dumpDMAEventTable();
        */

    } else {

        /*
        // Determine start and stop cycle
        uint16_t start = ddfstrtAligned;
        uint16_t stop  = ddfstopAligned;

        // Align stop such that (stop - start) is dividable by 8
        stop += (stop - start) & 0b100;
        */

        // Align fetch start and stop
        /*
        uint16_t start = (ddfstrt + 4) & ~0b111;
        uint16_t stop = (ddfstop + 4) & ~0b111; // WRONG

        // Compute real stop position
        stop = start + (((ddfstop & ~3) - (ddfstrt & ~3) + 7) & ~7);
        */

        int16_t start = dmaStrtLores;
        int16_t stop = dmaStopLores;

        // Determine event IDs
        EventID l6 = (activeBitplanes >= 6) ? DMA_L6 : EVENT_NONE;
        EventID l5 = (activeBitplanes >= 5) ? DMA_L5 : EVENT_NONE;
        EventID l4 = (activeBitplanes >= 4) ? DMA_L4 : EVENT_NONE;
        EventID l3 = (activeBitplanes >= 3) ? DMA_L3 : EVENT_NONE;
        EventID l2 = (activeBitplanes >= 2) ? DMA_L2 : EVENT_NONE;
        EventID l1 = (activeBitplanes >= 1) ? DMA_L1 : EVENT_NONE;
        
        // Schedule events
        for (unsigned i = start; i < stop; i += 8) {
            dmaEvent[i+0] = EVENT_NONE;
            dmaEvent[i+1] = l4;
            dmaEvent[i+2] = l6;
            dmaEvent[i+3] = l2;
            dmaEvent[i+4] = EVENT_NONE;
            dmaEvent[i+5] = l3;
            dmaEvent[i+6] = l5;
            dmaEvent[i+7] = l1;
        }

        // Remember start / stop positions
        if (dmaEvent[start+7] != EVENT_NONE) {

            dmaFirstBpl1Event = start + 7;
            dmaLastBpl1Event = stop - 1;

            assert(dmaEvent[dmaFirstBpl1Event] == DMA_L1);
            assert(dmaEvent[dmaLastBpl1Event] == DMA_L1);

            denise->firstCanvasPixel = (dmaFirstBpl1Event * 4) + 6;
            denise->lastCanvasPixel = (dmaLastBpl1Event * 4) + 6 + 31;

        } else {

            dmaFirstBpl1Event = 0;
            dmaLastBpl1Event = 0;
            denise->firstCanvasPixel = 0;
            denise->lastCanvasPixel = 0;
        }
    }

    // Because bitplane DMA and sprite DMA overlap, some sprite events might
    // have been overwritten with EVENT_NONE by the code above. These events
    // need to be restored.
    if (dmaEvent[0x15] != EVENT_NONE) {
        assert(dmaEvent[0x15] == DMA_S0_1);
        switchSpriteDmaOn();
    }

    updateJumpTable();

    // Do some consistency checks with the new lookup tables
    for (int i = 0; i < HPOS_CNT; i++) {
        if ((dmaEvent[i] >= DMA_L1 && dmaEvent[i] <= DMA_L6) &&
            (dmaEvent[i] >= DMA_H1 && dmaEvent[i] <= DMA_H4)) {
            if (dmaEvent[i] != bitplaneDMA[denise->hires()][activeBitplanes][i]) {
                warn("EVENT INCONSISTENCY DETECTED\n");
                dumpDMAEventTable();
                assert(false);
            }
        }
    }

    // debug("EventTable:\n");
    // dumpDMAEventTable();
}

void
Agnus::switchBitplaneDmaOff()
{
    // Quick-exit if the event table is free of any bitplane DMA events
    if (dmaFirstBpl1Event == 0) return;

    debug(BPL_DEBUG, "switchBitplaneDmaOff: \n");

    // Clear the event table
    for (int i = 0x18; i < HPOS_CNT; dmaEvent[i++] = (EventID)0);

    // Because bitplane DMA and sprite DMA overlap, the previous operation
    // might have wiped out sprite events. These events need to be restored.
    if (dmaEvent[0x15] != EVENT_NONE) {
        assert(dmaEvent[0x15] == DMA_S0_1);
        switchSpriteDmaOn();
    }

    updateJumpTable();

    dmaFirstBpl1Event = 0;
    dmaLastBpl1Event = 0;
    denise->firstCanvasPixel = 0;
    denise->lastCanvasPixel = 0;
}

void
Agnus::updateBitplaneDma()
{
    // Determine if bitplane DMA has to be on or off
    bool bplDma = inBplDmaArea();
    /*
    vFlop &&                                        // Vertical DIW flop set
    denise->bplconBPU() &&                          // at least one bitplane
    vpos >= bplVstrt && vpos < bplVstop &&       // DEPRECATED
    (dmacon & (DMAEN | BPLEN)) == (DMAEN | BPLEN);  // DMA is enabled
    */

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
    return (hpos >= dmaStopLores - 8);
    // return dmaEvent[hpos] != dmaEvent[hpos + 8];
}

bool
Agnus::isLastHx(int16_t dmaCycle)
{
    return (hpos >= dmaStopHires - 4);
    // return dmaEvent[hpos] != dmaEvent[hpos + 4];
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
            case DMA_DISK:     r3[i] = 'D'; r4[i] = 'I'; break;
            case DMA_A0:       r3[i] = 'A'; r4[i] = '0'; break;
            case DMA_A1:       r3[i] = 'A'; r4[i] = '1'; break;
            case DMA_A2:       r3[i] = 'A'; r4[i] = '2'; break;
            case DMA_A3:       r3[i] = 'A'; r4[i] = '3'; break;
            case DMA_S0_1:     r3[i] = 's'; r4[i] = '0'; break;
            case DMA_S1_1:     r3[i] = 's'; r4[i] = '1'; break;
            case DMA_S2_1:     r3[i] = 's'; r4[i] = '2'; break;
            case DMA_S3_1:     r3[i] = 's'; r4[i] = '3'; break;
            case DMA_S4_1:     r3[i] = 's'; r4[i] = '4'; break;
            case DMA_S5_1:     r3[i] = 's'; r4[i] = '5'; break;
            case DMA_S6_1:     r3[i] = 's'; r4[i] = '6'; break;
            case DMA_S7_1:     r3[i] = 's'; r4[i] = '7'; break;
            case DMA_S0_2:     r3[i] = 'S'; r4[i] = '0'; break;
            case DMA_S1_2:     r3[i] = 'S'; r4[i] = '1'; break;
            case DMA_S2_2:     r3[i] = 'S'; r4[i] = '2'; break;
            case DMA_S3_2:     r3[i] = 'S'; r4[i] = '3'; break;
            case DMA_S4_2:     r3[i] = 'S'; r4[i] = '4'; break;
            case DMA_S5_2:     r3[i] = 'S'; r4[i] = '5'; break;
            case DMA_S6_2:     r3[i] = 'S'; r4[i] = '6'; break;
            case DMA_S7_2:     r3[i] = 'S'; r4[i] = '7'; break;
            case DMA_L1:       r3[i] = 'L'; r4[i] = '1'; break;
            case DMA_L2:       r3[i] = 'L'; r4[i] = '2'; break;
            case DMA_L3:       r3[i] = 'L'; r4[i] = '3'; break;
            case DMA_L4:       r3[i] = 'L'; r4[i] = '4'; break;
            case DMA_L5:       r3[i] = 'L'; r4[i] = '5'; break;
            case DMA_L6:       r3[i] = 'L'; r4[i] = '6'; break;
            case DMA_H1:       r3[i] = 'H'; r4[i] = '1'; break;
            case DMA_H2:       r3[i] = 'H'; r4[i] = '2'; break;
            case DMA_H3:       r3[i] = 'H'; r4[i] = '3'; break;
            case DMA_H4:       r3[i] = 'H'; r4[i] = '4'; break;
            default:           r3[i] = '.'; r4[i] = '.'; break;
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
    
    bool oldDMAEN = (dmacon & DMAEN);
    bool oldBPLEN = (dmacon & BPLEN) && oldDMAEN;
    bool oldCOPEN = (dmacon & COPEN) && oldDMAEN;
    bool oldBLTEN = (dmacon & BLTEN) && oldDMAEN;
    bool oldSPREN = (dmacon & SPREN) && oldDMAEN;
    // bool oldDSKEN = (dmacon & DSKEN) && oldDMAEN;
    bool oldAU0EN = (dmacon & AU0EN) && oldDMAEN;
    bool oldAU1EN = (dmacon & AU1EN) && oldDMAEN;
    bool oldAU2EN = (dmacon & AU2EN) && oldDMAEN;
    bool oldAU3EN = (dmacon & AU3EN) && oldDMAEN;

    
    if (value & 0x8000) dmacon |= value; else dmacon &= ~value;
    dmacon &= 0x07FF;
    
    bool newDMAEN = (dmacon & DMAEN);
    bool newBPLEN = (dmacon & BPLEN) && newDMAEN;
    bool newCOPEN = (dmacon & COPEN) && newDMAEN;
    bool newBLTEN = (dmacon & BLTEN) && newDMAEN;
    bool newSPREN = (dmacon & SPREN) && newDMAEN;
    // bool newDSKEN = (dmacon & DSKEN) && newDMAEN;
    bool newAU0EN = (dmacon & AU0EN) && newDMAEN;
    bool newAU1EN = (dmacon & AU1EN) && newDMAEN;
    bool newAU2EN = (dmacon & AU2EN) && newDMAEN;
    bool newAU3EN = (dmacon & AU3EN) && newDMAEN;

    // Bitplane DMA
    if (oldBPLEN ^ newBPLEN) {

        if (newBPLEN) {
            
            // Bitplane DMA on
            debug(DMA_DEBUG, "Bitplane DMA switched on\n");
            // switchBitplaneDmaOn();

        } else {
            
            // Bitplane DMA off
            debug(DMA_DEBUG, "Bitplane DMA switched off\n");
            // switchBitplaneDmaOff();
        }

        hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;
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
        
        if (newSPREN) {
            // Sprite DMA on
            debug(DMA_DEBUG, "Sprite DMA switched on\n");
            switchSpriteDmaOn();
            
        } else {
            
            // Sprite DMA off
            debug(DMA_DEBUG, "Sprite DMA switched off\n");
            switchSpriteDmaOff();
        }
    }
    
    // Disk DMA (only the master bit is checked)
    // if (oldDSKEN ^ newDSKEN) {
    if (oldDMAEN ^ newDMAEN) {

        if (newDMAEN) {
            
            // Disk DMA on
            debug(DMA_DEBUG, "Disk DMA switched on\n");
            switchDiskDmaOn();
            
        } else {
            
            // Disk DMA off
            debug(DMA_DEBUG, "Disk DMA switched off\n");
            switchDiskDmaOff();
        }
    }
    
    // Audio DMA
    if (oldAU0EN ^ newAU0EN) {
        
        if (newAU0EN) {
            
            // debug("Audio 0 DMA switched on\n");
            switchAudioDmaOn(0);
            paula->audioUnit.enableDMA(0);
            
        } else {
            
            // debug("Audio 0 DMA switched off\n");
            switchAudioDmaOff(0);
            paula->audioUnit.disableDMA(0);
        }
    }
    
    if (oldAU1EN ^ newAU1EN) {
        
        if (newAU1EN) {
            
            // debug("Audio 1 DMA switched on\n");
            switchAudioDmaOn(1);
            paula->audioUnit.enableDMA(1);
            
        } else {
            
            // debug("Audio 1 DMA switched off\n");
            switchAudioDmaOff(1);
            paula->audioUnit.disableDMA(1);
        }
    }
    
    if (oldAU2EN ^ newAU2EN) {
        
        if (newAU2EN) {
            
            // debug("Audio 2 DMA switched on\n");
            switchAudioDmaOn(2);
            paula->audioUnit.enableDMA(2);
            
        } else {
            
            // debug("Audio 2 DMA switched off\n");
            switchAudioDmaOff(2);
            paula->audioUnit.disableDMA(2);
        }
    }
    
    if (oldAU3EN ^ newAU3EN) {
        
        if (newAU3EN) {
            
            // debug("Audio 3 DMA switched on\n");
            switchAudioDmaOn(3);
            paula->audioUnit.enableDMA(3);
            
        } else {
            
            // debug("Audio 3 DMA switched off\n");
            switchAudioDmaOff(3);
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
    uint16_t result = BEAM(vpos, hpos) & 0xFFFF;

    debug(BPL_DEBUG, "peekVHPOSR() = %X\n", result);
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
    uint16_t result = (vpos >> 8) | (isLongFrame() ? 0x8000 : 0);
    assert((result & 0x7FFE) == 0);

    debug(BPL_DEBUG, "peekVPOSR() = %X\n", result);
    return result;

}

void
Agnus::pokeVPOS(uint16_t value)
{
    // Don't know what to do here ...
}

void
Agnus::pokeDIWSTRT(uint16_t value)
{
    debug(BPL_DEBUG, "pokeDIWSTRT(%X)\n", value);

    // Simulate a 4 cycle delay
    // TODO: Pass a boolean flag `cpu` that shows who writes to the register.
    // Depending on this variable, schedule in REG_CPU_SLOT or REG_COP_SLOT.
    scheduleRegEvent(REG_COP_SLOT, DMA_CYCLES(4), REG_DIWSTRT, (int64_t)value);
}

void
Agnus::pokeDIWSTOP(uint16_t value)
{
    debug(BPL_DEBUG, "pokeDIWSTOP(%X)\n", value);

    // Simulate a 4 cycle delay
    // TODO: Pass a boolean flag `cpu` that shows who writes to the register.
    // Depending on this variable, schedule in REG_CPU_SLOT or REG_COP_SLOT.
    scheduleRegEvent(REG_COP_SLOT, DMA_CYCLES(4), REG_DIWSTOP, (int64_t)value);
}

void
Agnus::setDIWSTRT(uint16_t value)
{
    debug(BPL_DEBUG, "setDIWSTRT(%X)\n", value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0, V8 = 0

    diwstrt = value;

    // Extract the upper left corner of the display window
    diwVstrt = HI_BYTE(value);
    diwHstrt = LO_BYTE(value);

    // Invalidate the coordinate if it is out of range
    if (diwHstrt < 2) diwHstrt = -1;

    //
    // Recalculate diwFlopOn based on the new values
    //

    // Take into account that a value change needs 4 DMA cycles to show up
    // int16_t realhpos = hpos + 4;
    // int16_t pixelpos = realhpos * 2;
    int16_t pixelpos = hpos * 2;

    // Update diwFlopOn if hpos hasn't matched the old trigger position yet.
    if (pixelpos < hFlopOn) hFlopOn = diwHstrt;

    debug(BPL_DEBUG, "diwstrt = %X diwHstrt = %d diwVstrt = %d\n",
          diwstrt, diwHstrt, diwVstrt);
}

void
Agnus::setDIWSTOP(uint16_t value)
{
    debug(BPL_DEBUG, "setDIWSTOP(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1, V8 = !V7

    diwstop = value;

    // Extract the lower right corner of the display window
    diwVstop = HI_BYTE(value) | ((value & 0x8000) ? 0 : 0x100);
    diwHstop = LO_BYTE(value) | 0x100;

    // debug("diwstop = $%X diwVstrt = %d diwVstop = %d\n", diwstop, diwVstrt, diwVstop);

    // Invalidate the coordinate if it is out of range
    if (diwHstop > 0x1C7) diwHstop = -1;

    //
    // Recalculate diwFlopOff based on the new values
    //

    // Take into account that a value change needs 4 DMA cycles to take effect
    // int16_t realhpos = hpos + 4;
    // int16_t pixelpos = realhpos * 2;
    int16_t pixelpos = hpos * 2;

    // Update diwFlopOff if hpos hasn't matched the old trigger position yet.
    if (pixelpos < hFlopOff) hFlopOff = diwHstop;

    debug(BPL_DEBUG, "diwstop = %X diwHstop = %d diwVstop = %d\n",
          diwstop, diwHstop, diwVstop);
}

void
Agnus::pokeDDFSTRT(uint16_t value)
{
    debug(BPL_DEBUG, "pokeDDFSTRT(%X)\n", value);

    ddfstrt = value;

    // Fit to raster and cap minimum value at 0x18
    uint16_t oldValue = ddfstrtAligned;
    uint16_t newValue = MAX(value & 0xFC, 0x18);

    if (newValue != value) {
        debug(BPL_DEBUG, "Strange value detected in pokeDDFSTRT(%X)\n", value);
    }

    if (newValue != oldValue) {
        debug(BPL_DEBUG, "DDFSTRT changed from %d to %d\n", oldValue, newValue);
    }

    // DEPRECATED
    ddfstrtAligned = newValue;
    dmaStrt = ddfstrtAligned;

    // Compute the DMA window
    computeDDFWindow();

    // hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;
    updateBitplaneDma();
}

void
Agnus::pokeDDFSTOP(uint16_t value)
{
    debug(BPL_DEBUG, "pokeDDFSTOP(%X)\n", value);

    ddfstop = value;

    // Fit to raster and cap maximum value at 0xD8
    uint16_t oldValue = ddfstopAligned;
    uint16_t newValue = MIN(value & 0xFC, 0xD8);

    if (newValue != value) {
        debug(BPL_DEBUG, "Strange value detected in pokeDDFSTOP(%X)\n", value);
    }

    if (newValue != oldValue) {
        debug(BPL_DEBUG, "DDFSTOP changed from %d to %d\n", oldValue, newValue);
    }

    // DEPRECATED
    ddfstopAligned = newValue;
    dmaStop = ddfstopAligned;
    // Align dmaStop such that (dmaStop - dmaStart) is dividable by 8
    dmaStop += (dmaStop - dmaStrt) & 0b100;
    // Goto last event in fetch unit
    dmaStop += 7;

    // Compute the DMA window
    computeDDFWindow();

    // hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;
    updateBitplaneDma();
}

void
Agnus::computeDDFWindow()
{
    // Clip values
    int16_t strt = MAX(ddfstrt, 0x18);
    int16_t stop = MIN(ddfstop, 0xDF);

    // Compute the beginning of the DMA window
    dmaStrtLores = (strt + 4) & ~7;
    dmaStrtHires = (strt + 2) & ~3;

    // Compute the number of fetch units
    int numUnitsLores = ((stop & ~3) - (strt & ~3) + 7) / 8;
    int numUnitsHires = ((stop & ~3) - (strt & ~3) + 7) / 4;

    numUnitsHires += 2; // THIS SHOULDN'T BE 
    numUnitsLores += 1;

    // Compute the end of the DMA window
    dmaStopLores = dmaStrtLores + 8 * numUnitsLores;
    dmaStopHires = dmaStrtHires + 4 * numUnitsHires;
}

template <int x> void
Agnus::pokeBPLxPTH(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPL%dPTH(%X)\n", x, value);
    
    bplpt[x] = REPLACE_HI_WORD(bplpt[x], value & 0x7);
}

template <int x> void
Agnus::pokeBPLxPTL(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPL%dPTL(%X)\n", x, value);
    
    bplpt[x] = REPLACE_LO_WORD(bplpt[x], value & 0xFFFE);
}

void
Agnus::pokeBPL1MOD(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPL1MOD(%X)\n", value);

    // Simulate delay
    // TODO: Pass a boolean flag `cpu` that shows who writes to the register.
    // Depending on this variable, schedule in REG_CPU_SLOT or REG_COP_SLOT.
    scheduleRegEvent(REG_COP_SLOT, DMA_CYCLES(2), REG_BPL1MOD, (int64_t)value);
}

void
Agnus::pokeBPL2MOD(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPL2MOD(%X)\n", value);

    // Simulate delay
    // TODO: Pass a boolean flag `cpu` that shows who writes to the register.
    // Depending on this variable, schedule in REG_CPU_SLOT or REG_COP_SLOT.
    scheduleRegEvent(REG_COP_SLOT, DMA_CYCLES(2), REG_BPL2MOD, (int64_t)value);
}

void
Agnus::setBPL1MOD(uint16_t value)
{
    debug(BPL_DEBUG, "setBPL1MOD(%X)\n", value);
    bpl1mod = int16_t(value & 0xFFFE);
}

void
Agnus::setBPL2MOD(uint16_t value)
{
    debug(BPL_DEBUG, "setBPL2MOD(%X)\n", value);
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
Agnus::addBPLxMOD()
{
    // Add bpl2mod is added to all active even bitplane pointers
    // Add blp1mod is added to all active odd bitplane pointers
    
    switch (activeBitplanes) {
        case 6: INC_OCS_PTR(bplpt[5], bpl2mod); // fallthrough
        case 5: INC_OCS_PTR(bplpt[4], bpl1mod); // fallthrough
        case 4: INC_OCS_PTR(bplpt[3], bpl2mod); // fallthrough
        case 3: INC_OCS_PTR(bplpt[2], bpl1mod); // fallthrough
        case 2: INC_OCS_PTR(bplpt[1], bpl2mod); // fallthrough
        case 1: INC_OCS_PTR(bplpt[0], bpl1mod);
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
        hpos++;
        
        // Note: If this assertion hits, the HSYNC event hasn't been served!
        assert(hpos <= HPOS_MAX);

        clock += DMA_CYCLES(1);
    }
}

template <int nr> void
Agnus::executeFirstSpriteCycle()
{
    // Activate sprite data DMA if the first sprite line has been reached
    if (vpos == sprVStrt[nr]) { sprDmaState[nr] = SPR_DMA_DATA; }

    // Deactivate sprite data DMA if the last sprite line has been reached
    if (vpos == sprVStop[nr]) {

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
    if (vpos == sprVStop[nr] + 1) {
        
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
    if (hpos != HPOS_MAX) { dump(); assert(false); }

    //
    // Let other components finish up the current line
    //

    // Let Denise draw the current line
    denise->endOfLine(vpos);

    // Let Paula synthesize new sound samples
    paula->audioUnit.executeUntil(clock);
    
    // Let CIA B count the HSYNCs
    amiga->ciaB.incrementTOD();
    
    // Check the keyboard once in a while
    if ((vpos & 0b1111) == 0) amiga->keyboard.execute();

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

    vpos++;
    hpos = -1;

    // Check if the current frame is finished
    if (vpos >= frameInfo.numLines) {
        vsyncHandler();
    }

    //
    // Prepare for the next line
    //

    // Switch sprite DMA off if the last rasterline has been reached
    if (vpos == frameInfo.numLines - 1) {
        switchSpriteDmaOff();
        for (unsigned i = 0; i < 8; i++) {
            sprDmaState[i] = SPR_DMA_IDLE;
        }
    }

    // Check if this line is a bitplane DMA line
    bool bplDmaArea = inBplDmaArea();

    // Update the event table if the values has changed
    if (bplDmaArea ^ oldBplDmaArea) {
        hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;
        oldBplDmaArea = bplDmaArea;
    }

    // Check if we have reached line 25 (sprite DMA starts here)
    if (vpos == 25) {
        if ((dmacon & DMAEN) && (dmacon & SPREN)) {
            
            // Reset vertical sprite trigger coordinates which forces the sprite
            // logic to read in the control words for all sprites in this line.
            for (unsigned i = 0; i < 8; i++) { sprVStop[i] = 25; }
            
            switchSpriteDmaOn();
        }
    }

    // Update the vertical DIW flipflop
    if (vpos == diwVstrt && !vFlop) {
        vFlop = true;
        updateBitplaneDma();
        // debug("vFlop = %d diwVstrt = %d\n", vFlop, diwVstrt);
    }
    if (vpos == diwVstop && vFlop) {
        vFlop = false;
        updateBitplaneDma();
        // debug("vFlop = %d diwVstop = %d\n", vFlop, diwVstop);
    }

    // Update the horizontal DIW flipflop
    hFlop = (hFlopOff != -1) ? false : (hFlopOn != -1) ? true : hFlop;
    hFlopOn = diwHstrt;
    hFlopOff = diwHstop;

    //
    // Process pending work items
    //

    if (hsyncActions) {

        if (hsyncActions & HSYNC_UPDATE_EVENT_TABLE) {

            // Force the DMA time slot allocation table to update.
            updateBitplaneDma();
        }
        hsyncActions = 0;
    }

    //
    // Schedule events
    //

    // Schedule the first hi-prio DMA event (if any)
    if (nextDmaEvent[0]) {
        EventID eventID = dmaEvent[nextDmaEvent[0]];
        schedulePos<DMA_SLOT>(vpos, nextDmaEvent[0], eventID);
    }

    // Schedule first SYNC event
    schedulePos<SYNC_SLOT>(vpos, HPOS_MAX, SYNC_H);


    //
    // Let other components prepare for the next line
    //

    denise->beginOfLine(vpos);
}

void
Agnus::vsyncHandler()
{
    // debug("diwVstrt = %d diwVstop = %d diwHstrt = %d diwHstop = %d\n", diwVstrt, diwVstop, diwHstrt, hstop);

    // Advance to the next frame
    frameInfo.nr++;

    // Check if we the next frame is drawn in interlace mode
    frameInfo.interlaced = denise->bplconLACE();

    // If yes, toggle the the long frame flipflop
    lof = (frameInfo.interlaced) ? !lof : true;

    // Determine if the next frame is a long or a short frame
    frameInfo.numLines = lof ? 313 : 312;

    // Increment frame and reset vpos
    frame++; // DEPRECATED
    assert(frame == frameInfo.nr);

    // Reset vertical position counter
    vpos = 0;

    // Reset the vertical DIW flipflop
    vFlop = false;
    
    // CIA A counts VSYNCs
    amiga->ciaA.incrementTOD();
    
    // Trigger VSYNC interrupt
    paula->pokeINTREQ(0x8020);
    
    // Let the subcomponents do their own VSYNC stuff
    copper.vsyncAction();
    denise->prepareForNextFrame(isLongFrame(), frameInfo.interlaced);
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

template void Agnus::pokeBPLxPTH<0>(uint16_t value);
template void Agnus::pokeBPLxPTH<1>(uint16_t value);
template void Agnus::pokeBPLxPTH<2>(uint16_t value);
template void Agnus::pokeBPLxPTH<3>(uint16_t value);
template void Agnus::pokeBPLxPTH<4>(uint16_t value);
template void Agnus::pokeBPLxPTH<5>(uint16_t value);

template void Agnus::pokeBPLxPTL<0>(uint16_t value);
template void Agnus::pokeBPLxPTL<1>(uint16_t value);
template void Agnus::pokeBPLxPTL<2>(uint16_t value);
template void Agnus::pokeBPLxPTL<3>(uint16_t value);
template void Agnus::pokeBPLxPTL<4>(uint16_t value);
template void Agnus::pokeBPLxPTL<5>(uint16_t value);

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
