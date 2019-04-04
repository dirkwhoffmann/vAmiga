// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

/* Emulates a Direct Memory Access.
 * ptr is a DMA pointer register and dest the destination
 */
#define DO_DMA(ptr,dest) \
dest = amiga->mem.peekChip16(ptr); \
ptr = (ptr + 2) & 0x7FFFE;

Agnus::Agnus()
{
    setDescription("Agnus");
    
    registerSubcomponents(vector<HardwareComponent *> {
        
        &copper,
        &blitter,
        &eventHandler,
    });
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,           sizeof(clock),           0 },
        { &frame,           sizeof(frame),           0 },
        { &latchedClock,    sizeof(latchedClock),    0 },
        { &vpos,            sizeof(vpos),            0 },
        { &hpos,            sizeof(hpos),            0 },
        { &hstrt,           sizeof(hstrt),           0 },
        { &hstop,           sizeof(hstop),           0 },
        { &vstrt,           sizeof(vstrt),           0 },
        { &vstop,           sizeof(vstop),           0 },
        { &dmacon,          sizeof(dmacon),          0 },
        { &dskpt,           sizeof(dskpt),           0 },
        { &diwstrt,         sizeof(diwstrt),         0 },
        { &diwstop,         sizeof(diwstop),         0 },
        { &ddfstrt,         sizeof(ddfstrt),         0 },
        { &ddfstop,         sizeof(ddfstop),         0 },
        { &audlc,           sizeof(audlc),           DWORD_ARRAY },
        { &bplpt,           sizeof(bplpt),           DWORD_ARRAY },
        { &bpl1mod,         sizeof(bpl1mod),         0 },
        { &bpl2mod,         sizeof(bpl2mod),         0 },
        { &sprpt,           sizeof(sprpt),           DWORD_ARRAY },
        { &busOwner,        sizeof(busOwner),        0 },
        { &dmaEvent,        sizeof(dmaEvent),        0 },
        { &nextDmaEvent,    sizeof(nextDmaEvent),    0 },
        { &activeBitplanes, sizeof(activeBitplanes), 0 }
    });
}

void
Agnus::_powerOn()
{
    clock = 0;
    
    // Initialize lookup tables
    // buildDMAEventTable();
    clearDMAEventTable();
    
    // Schedule the first RAS event
    eventHandler.scheduleAbs(RAS_SLOT, DMA_CYCLES(HPOS_MAX), RAS_HSYNC);
    
    // Schedule the first CIA A and CIA B events
    eventHandler.scheduleAbs(CIAA_SLOT, CIA_CYCLES(1), CIA_EXECUTE);
    eventHandler.scheduleAbs(CIAB_SLOT, CIA_CYCLES(1), CIA_EXECUTE);

    // Initialize the SEC_SLOT with a (never triggering) SEC_TRIGGER event.
    // Doing so let's us use reschedule() on this slot all the time.
    eventHandler.scheduleAbs(SEC_SLOT, NEVER, SEC_TRIGGER);
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
    info.dmacon  = dmacon;
    info.diwstrt = diwstrt;
    info.diwstop = diwstop;
    info.ddfstrt = ddfstrt;
    info.ddfstop = ddfstop;
    
    info.bpl1mod = bpl1mod;
    info.bpl2mod = bpl2mod;
    info.numBpls = activeBitplanes;
    
    info.dskpt   = dskpt;
    for (unsigned i = 0; i < 4; i++)  info.audlc[i] = audlc[i];
    for (unsigned i = 0; i < 6; i++)  info.bplpt[i] = bplpt[i];
    for (unsigned i = 0; i < 8; i++) info.sprptr[i] = sprpt[i];
}

void
Agnus::_dump()
{        
    plainmsg("  hstrt : %d\n", hstrt);
    plainmsg("  hstop : %d\n", hstop);
    plainmsg("  vstrt : %d\n", vstrt);
    plainmsg("  vstop : %d\n", vstop);

    plainmsg("\nDMA time slot allocation:\n\n");

    dumpDMAEventTable(0x00, 0x4F);
    dumpDMAEventTable(0x50, 0x9F);
    dumpDMAEventTable(0xA0, 0xE2);
    
    eventHandler.dump();
}

DMACycle
Agnus::cyclesInCurrentFrame()
{
    // TODO: Distinguish between short frames and long frames
    /*
    if (shortFrame) {
        return 312 * cyclesPerLine();
    } else {
        return 313 * cyclesPerLine();
    }
    */
    return 313 * cyclesPerLine();
}

void
Agnus::cycleToBeamAbs(Cycle cycle, int64_t &frame, int16_t &vpos, int16_t &hpos)
{
    DMACycle dmaCycle    = AS_DMA_CYCLES(cycle);
    DMACycle frameCycles = cyclesInCurrentFrame();
    DMACycle lineCycles  = cyclesPerLine();
    
    frame = dmaCycle / frameCycles;
    dmaCycle = dmaCycle % frameCycles;
    vpos = dmaCycle / lineCycles;
    dmaCycle = dmaCycle % lineCycles;
    hpos = dmaCycle;
}

void
Agnus::cycleToBeamRel(Cycle cycle, int64_t &frame, int16_t &vpos, int16_t &hpos)
{
    return cycleToBeamAbs(cycle + latchedClock, frame, vpos, hpos);
}

Cycle
Agnus::beamToCyclesAbs(int16_t vpos, int16_t hpos)
{
    return latchedClock + beamToCyclesRel(vpos, hpos);
}

Cycle
Agnus::beamToCyclesRel(int16_t vpos, int16_t hpos)
{
    return DMA_CYCLES(vpos * cyclesPerLine() + hpos); 
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

void
Agnus::buildDMAEventTable()
{
    // debug("buildDMAEventTable vpos = %d hpos = %d\n", vpos, hpos);
    
    // Start with a clean table
    memset(dmaEvent, 0, sizeof(dmaEvent));

    // Check DMA enable bit
    if (dmacon & DMAEN) {
        
        // Disk DMA
        if (dmacon & DSKEN) {
            dmaEvent[0x07] = DMA_DISK;
            dmaEvent[0x09] = DMA_DISK;
            dmaEvent[0x0B] = DMA_DISK;
        }
        
        // Audio DMA
        if (dmacon & AU0EN) dmaEvent[0x0D] = DMA_A0;
        if (dmacon & AU1EN) dmaEvent[0x0F] = DMA_A1;
        if (dmacon & AU2EN) dmaEvent[0x11] = DMA_A2;
        if (dmacon & AU3EN) dmaEvent[0x13] = DMA_A3;
        
        // Sprite DMA (some slots may be overwritten by bitplane DMA cycles)
        // TODO: Individually switch on / off channels
        if (dmacon & SPREN) { // && sprite 0 enabled
            dmaEvent[0x15] = DMA_S0;
            dmaEvent[0x17] = DMA_S0;
        }
        if (dmacon & SPREN) { // && sprite 1 enabled
            dmaEvent[0x19] = DMA_S1;
            dmaEvent[0x1B] = DMA_S1;
        }
        if (dmacon & SPREN) { // && sprite 2 enabled
            dmaEvent[0x1D] = DMA_S2;
            dmaEvent[0x1F] = DMA_S2;
        }
        if (dmacon & SPREN) { // && sprite 3 enabled
            dmaEvent[0x21] = DMA_S3;
            dmaEvent[0x23] = DMA_S3;
        }
        if (dmacon & SPREN) { // && sprite 4 enabled
            dmaEvent[0x25] = DMA_S4;
            dmaEvent[0x27] = DMA_S4;
        }
        if (dmacon & SPREN) { // && sprite 5 enabled
            dmaEvent[0x29] = DMA_S5;
            dmaEvent[0x2B] = DMA_S5;
        }
        if (dmacon & SPREN) { // && sprite 6 enabled
            dmaEvent[0x2D] = DMA_S6;
            dmaEvent[0x2F] = DMA_S6;
        }
        if (dmacon & SPREN) { // && sprite 7 enabled
            dmaEvent[0x31] = DMA_S7;
            dmaEvent[0x33] = DMA_S7;
        }
        
        // Bitplane DMA
        if (dmacon & BPLEN) {
            
            // Determine start and stop cycle (TODO: CHECK IN WINFELLOW)
            uint8_t start = ddfstrt;
            uint8_t stop  = ddfstop + 16 /* ??? LORES ONLY(?!)*/;

            if (start < 0x18) start = 0x18;
            // if (stop > (0xD7 + 8)) stop = (0xD7 + 8);
            if (stop > (0xD7)) stop = (0xD7);

            if (amiga->denise.hires()) {
                
                switch (activeBitplanes) {
                    case 6:
                    case 5:
                    case 4:
                        for (int i = start & 0xF8; i < stop; i += 4)
                            dmaEvent[i] = DMA_H4;
                        // fallthrough
                    case 3:
                        for (int i = start & 0xF8; i < stop; i += 4)
                            dmaEvent[i+1] = DMA_H3;
                        // fallthrough
                    case 2:
                        for (int i = start & 0xF8; i < stop; i += 4)
                            dmaEvent[i+2] = DMA_H2;
                        // fallthrough
                    case 1:
                        for (int i = start & 0xF8; i < stop; i += 4)
                            dmaEvent[i+3] = DMA_H1;
                }
                
            } else {
                
                switch (activeBitplanes) {
                    case 6:
                        for (int i = start & 0xF8; i < stop; i += 8)
                            dmaEvent[i+2] = DMA_L6;
                        // fallthrough
                    case 5:
                        for (int i = start & 0xF8; i < stop; i += 8)
                            dmaEvent[i+6] = DMA_L5;
                        // fallthrough
                    case 4:
                        for (int i = start & 0xF8; i < stop; i += 8)
                            dmaEvent[i+1] = DMA_L4;
                        // fallthrough
                    case 3:
                        for (int i = start & 0xF8; i < stop; i += 8)
                            dmaEvent[i+5] = DMA_L3;
                        // fallthrough
                    case 2:
                        for (int i = start & 0xF8; i < stop; i += 8)
                            dmaEvent[i+3] = DMA_L2;
                        // fallthrough
                    case 1:
                        for (int i = start & 0xF8; i < stop; i += 8)
                            dmaEvent[i+7] = DMA_L1;
                }
            }
        }
    }
    
    // Build jump table
    uint8_t next = 0;
    for (int i = HPOS_MAX; i >= 0; i--) {
        nextDmaEvent[i] = next;
        if (dmaEvent[i]) next = i;
    }
}

void
Agnus::clearDMAEventTable()
{
    // debug("CLEARING DMA event table\n");
    
    // Clear the allocation table
    memset(dmaEvent, 0, sizeof(dmaEvent));
    memset(nextDmaEvent, 0, sizeof(nextDmaEvent));
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
            case DMA_DISK:  r3[i] = 'D'; r4[i] = 'I'; break;
            case DMA_A0:    r3[i] = 'A'; r4[i] = '0'; break;
            case DMA_A1:    r3[i] = 'A'; r4[i] = '1'; break;
            case DMA_A2:    r3[i] = 'A'; r4[i] = '2'; break;
            case DMA_A3:    r3[i] = 'A'; r4[i] = '3'; break;
            case DMA_S0:    r3[i] = 'S'; r4[i] = '0'; break;
            case DMA_S1:    r3[i] = 'S'; r4[i] = '1'; break;
            case DMA_S2:    r3[i] = 'S'; r4[i] = '2'; break;
            case DMA_S3:    r3[i] = 'S'; r4[i] = '3'; break;
            case DMA_S4:    r3[i] = 'S'; r4[i] = '4'; break;
            case DMA_S5:    r3[i] = 'S'; r4[i] = '5'; break;
            case DMA_S6:    r3[i] = 'S'; r4[i] = '6'; break;
            case DMA_S7:    r3[i] = 'S'; r4[i] = '7'; break;
            case DMA_L1:    r3[i] = 'L'; r4[i] = '1'; break;
            case DMA_L2:    r3[i] = 'L'; r4[i] = '2'; break;
            case DMA_L3:    r3[i] = 'L'; r4[i] = '3'; break;
            case DMA_L4:    r3[i] = 'L'; r4[i] = '4'; break;
            case DMA_L5:    r3[i] = 'L'; r4[i] = '5'; break;
            case DMA_L6:    r3[i] = 'L'; r4[i] = '6'; break;
            case DMA_H1:    r3[i] = 'H'; r4[i] = '1'; break;
            case DMA_H2:    r3[i] = 'H'; r4[i] = '2'; break;
            case DMA_H3:    r3[i] = 'H'; r4[i] = '3'; break;
            case DMA_H4:    r3[i] = 'H'; r4[i] = '4'; break;
            default:        r3[i] = '.'; r4[i] = '.'; break;
        }
    }
    r1[i] = r2[i] = r3[i] = r4[i] = 0;
    plainmsg("%s\n", r1);
    plainmsg("%s\n", r2);
    plainmsg("%s\n", r3);
    plainmsg("%s\n", r4);
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
    debug(2, "pokeDMACON(%X)\n", value);
    
    bool oldDMAEN = (dmacon & DMAEN);
    bool oldBPLEN = (dmacon & BPLEN) && oldDMAEN;
    bool oldCOPEN = (dmacon & COPEN) && oldDMAEN;
    bool oldBLTEN = (dmacon & BLTEN) && oldDMAEN;
    bool oldSPREN = (dmacon & SPREN) && oldDMAEN;
    bool oldDSKEN = (dmacon & DSKEN) && oldDMAEN;
    
    if (value & 0x8000) dmacon |= value; else dmacon &= ~value;
    dmacon &= 0x07FF;
    
    bool newDMAEN = (dmacon & DMAEN);
    bool newBPLEN = (dmacon & BPLEN) && newDMAEN;
    bool newCOPEN = (dmacon & COPEN) && newDMAEN;
    bool newBLTEN = (dmacon & BLTEN) && newDMAEN;
    bool newSPREN = (dmacon & SPREN) && newDMAEN;
    bool newDSKEN = (dmacon & DSKEN) && newDMAEN;
 
    // Bitplane DMA
    if (oldBPLEN ^ newBPLEN) {

        if (newBPLEN) {
            
            // Bitplane DMA on
            debug("Bitplane DMA switched on\n");

        } else {
            
            // Bitplane DMA off
            debug("Bitplane DMA switched off\n");
        }
    }
    
    // Copper DMA
    if (oldCOPEN ^ newCOPEN) {
        
        if (newCOPEN) {
            
            // Copper DMA on
            debug("Copper DMA switched on\n");
            
            // Determine trigger cycle for the first Copper event
            // (the next even DMA cycle)
            Cycle trigger = (clock + 15) & ~15;
            
            eventHandler.scheduleAbs(COP_SLOT, trigger, COP_FETCH);
            
        } else {
            
            // Copper DMA off
            debug("Copper DMA switched off\n");
            eventHandler.cancel(COP_SLOT);
        }
    }
    
    // Blitter DMA
    if (oldBLTEN ^ newBLTEN) {
        
        if (newBLTEN) {
            // Blitter DMA on
            debug("Blitter DMA switched on\n");
            // amiga->agnus.eventHandler.scheduleRel(BLT_SLOT, DMA_CYCLES(1), BLT_EXECUTE);
            amiga->agnus.eventHandler.scheduleRel(BLT_SLOT, DMA_CYCLES(1), BLT_FAST_BLIT);
    
        } else {
            
            // Blitter DMA off
            debug("Blitter DMA switched off\n");
            amiga->agnus.eventHandler.disable(BLT_SLOT);
        }
    }
    
    // Sprite DMA
    if (oldSPREN ^ newSPREN) {
        
        if (newSPREN) {
            // Sprite DMA on
            debug("Sprite DMA switched on\n");
            
        } else {
            
            // Sprite DMA off
            debug("Sprite DMA switched off\n");
        }
    }
    
    // Disk DMA
    if (oldDSKEN ^ newDSKEN) {
        
        if (newDSKEN) {
            
            // Disk DMA on
            debug("Disk DMA switched on\n");
            
        } else {
            
            // Disk DMA off
            debug("Disk DMA switched off\n");
        }
    }
    
    buildDMAEventTable();
}

void
Agnus::pokeDSKPTH(uint16_t value)
{
    debug(2, "pokeDSKPTH(%X)\n", value);
    dskpt = REPLACE_HI_WORD(dskpt, value & 0x7);
}

void
Agnus::pokeDSKPTL(uint16_t value)
{
    debug(2, "pokeDSKPTL(%X)\n", value);
    dskpt = REPLACE_LO_WORD(dskpt, value);
}

uint16_t
Agnus::peekVHPOSR()
{
    // V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1
    
    debug(2, "peekVHPOSR: %X\n", BEAM(vpos, hpos) & 0xFFFF);

    return BEAM(vpos, hpos) & 0xFFFF;
}

void
Agnus::pokeVHPOS(uint16_t value)
{
    // Don't know what to do here ...
    
    // Caution: If we change the beam position, the value of variable
    // latchedClock (clock at the beginning of the current frame is
    // invalidated).
}

uint16_t
Agnus::peekVPOSR()
{
    // LF -- -- -- -- -- -- -- -- -- -- -- -- -- -- V8
    // TODO: LF (Long Frame)
    assert((vpos >> 8) <= 1);
    
    debug(2, "peekVPOSR: %X\n", (vpos >> 8) | ((frame % 2) ? 0x8000 : 0));

    return (vpos >> 8) | ((frame % 2) ? 0x8000 : 0);

}

void
Agnus::pokeVPOS(uint16_t value)
{
    // Don't know what to do here ...
    
    // Caution: If we change the beam position, the value of variable
    // latchedClock (clock at the beginning of the current frame is
    // invalidated).
}

void
Agnus::pokeDIWSTRT(uint16_t value)
{
    debug(2, "pokeDIWSTRT(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0, H8 = 0
    
    diwstrt = value;
    hstrt = LO_BYTE(value);
    vstrt = HI_BYTE(value);
}

void
Agnus::pokeDIWSTOP(uint16_t value)
{
    debug(2, "pokeDIWSTOP(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1, V8 = !V7
    
    diwstop = value;
    hstop = LO_BYTE(value) | 0x100;
    vstop = HI_BYTE(value) | ((value & 0x80) << 1);
}

void
Agnus::pokeDDFSTRT(uint16_t value)
{
    debug(2, "pokeDDFSTRT(%X)\n", value);

    ddfstrt = value;
}

void
Agnus::pokeDDFSTOP(uint16_t value)
{
    debug(2, "pokeDDFSTOP(%X)\n", value);
    
    ddfstop = value;
}

void
Agnus::pokeAUDxLCH(int x, uint16_t value)
{
    assert(x < 4);
    
    debug(2, "pokeAUD%dLCH(%X)\n", x, value);
    audlc[x] = REPLACE_HI_WORD(audlc[x], value & 0x7);
}

void
Agnus::pokeAUDxLCL(int x, uint16_t value)
{
    assert(x < 4);
    
    debug(2, "pokeAUD%dLCL(%X)\n", x, value);
    audlc[x] = REPLACE_LO_WORD(audlc[x], value);
}

void
Agnus::pokeBPLxPTH(int x, uint16_t value)
{
    assert(x < 6);
    
    debug(2, "pokeBPL%dPTH(%X)\n", x, value);
    bplpt[x] = REPLACE_HI_WORD(bplpt[x], value & 0x7);
}

void
Agnus::pokeBPLxPTL(int x, uint16_t value)
{
    assert(x < 6);
    
    debug(2, "pokeBPL%dPTL(%X)\n", x, value);
    bplpt[x] = REPLACE_LO_WORD(bplpt[x], value);
}

void
Agnus::pokeBPL1MOD(uint16_t value)
{
    debug(2, "pokeBPL1MOD(%X)\n", value);
    
    bpl1mod = value;
}

void
Agnus::pokeBPL2MOD(uint16_t value)
{
    debug(2, "pokeBPL2MOD(%X)\n", value);
    
    bpl2mod = value;
}

void
Agnus::pokeSPRxPTH(int x, uint16_t value)
{
    assert(x < 8);
    
    debug(2, "pokeSPR%dPTH(%X)\n", x, value);
    sprpt[x] = REPLACE_HI_WORD(sprpt[x], value & 0x7);
}

void
Agnus::pokeSPRxPTL(int x, uint16_t value)
{
    assert(x < 8);
    
    debug(2, "pokeSPR%dPTL(%X)\n", x, value);
    sprpt[x] = REPLACE_LO_WORD(sprpt[x], value);
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

bool
Agnus::copperCanHaveBus()
{
    // For now, we only check the DMACON register.
    // Later, we need to check if the bus is really free and if the current
    // cylce is even.
    
    if (dmacon & 0b1010000000) {
        // debug("COPPER DMA IS ENABLED !!!!\n"); 
    }
        
    return (dmacon & 0b1010000000) != 0;
}

void
Agnus::executeUntil(Cycle targetClock)
{
    // msg("clock is %lld, Executing until %lld\n", clock, targetClock);
    while (clock <= targetClock - DMA_CYCLES(1)) {
        
        busOwner = 0;
        
        // Process all pending events
        eventHandler.executeUntil(clock);
        
        // Advance the internal counters
        hpos++;
        
        // Note: If this assertion hits, the HSYNC event hasn't been served!
        assert(hpos <= HPOS_MAX);
        
        clock += DMA_CYCLES(1);
    }
}

void
Agnus::serviceDMAEvent(EventID id)
{
    busOwner = BPLEN;
    
    switch (id) {
           
        case DMA_DISK:
            amiga->paula.doDiskDMA();
            break;
            
        case DMA_A0:
            break;
            
        case DMA_A1:
            break;
            
        case DMA_A2:
            break;
            
        case DMA_A3:
            break;
            
        case DMA_S0:
            break;
            
        case DMA_S1:
            break;
            
        case DMA_S2:
            break;
            
        case DMA_S3:
            break;
            
        case DMA_S4:
            break;
            
        case DMA_S5:
            break;
            
        case DMA_S6:
            break;
            
        case DMA_S7:
            break;
            
        case DMA_H1:
            if (amiga->debugDMA) debug("H1\n");
        case DMA_L1:
            
            if (amiga->debugDMA) debug("DO_DMA H1/L1 (%d,%d): bpldat[%d] = peekChip16(%X) = %X\n", vpos, hpos, PLANE1, bplpt[PLANE1], amiga->mem.peekChip16(bplpt[PLANE1]));
                  
            DO_DMA(bplpt[PLANE1], amiga->denise.bpldat[PLANE1]);
            
            // The bitplane 1 fetch is an important one. Once it is performed,
            // Denise fills it's shift registers.
            amiga->denise.fillShiftRegisters();
            break;
            
        case DMA_H2:
            if (amiga->debugDMA) debug("H2\n");
        case DMA_L2:
            
            if (amiga->debugDMA) debug("DO_DMA H2/L2 (%d,%d): bpldat[%d] = peekChip16(%X) = %X\n", vpos, hpos, PLANE2, bplpt[PLANE2], amiga->mem.peekChip16(bplpt[PLANE2]));
            DO_DMA(bplpt[PLANE2], amiga->denise.bpldat[PLANE2]);
            break;
            
        case DMA_H3:
            if (amiga->debugDMA) debug("H3\n");
        case DMA_L3:
            
            if (amiga->debugDMA) debug("DO_DMA H3/L3: bpldat[%d] = peekChip16(%X) = %X\n", PLANE3, bplpt[PLANE3], amiga->mem.peekChip16(bplpt[PLANE3]));
            DO_DMA(bplpt[PLANE3], amiga->denise.bpldat[PLANE3]);
            break;
            
        case DMA_H4:
            if (amiga->debugDMA) debug("H4\n");
        case DMA_L4:
            
            if (amiga->debugDMA) debug("DO_DMA H4/L4: bpldat[%d] = peekChip16(%X) = %X\n", PLANE4, bplpt[PLANE4], amiga->mem.peekChip16(bplpt[PLANE4]));
            DO_DMA(bplpt[PLANE4], amiga->denise.bpldat[PLANE4]);
            break;
            
        case DMA_L5:
            
            if (amiga->debugDMA) debug("DO_DMA L5: bpldat[%d] = peekChip16(%X) = %X\n", PLANE5, bplpt[PLANE5], amiga->mem.peekChip16(bplpt[PLANE5]));
            DO_DMA(bplpt[PLANE5], amiga->denise.bpldat[PLANE5]);
            break;
            
        case DMA_L6:
            
            if (amiga->debugDMA) debug("DO_DMA L6: bpldat[%d] = peekChip16(%X) = %X\n", PLANE6, bplpt[PLANE6], amiga->mem.peekChip16(bplpt[PLANE6]));
            DO_DMA(bplpt[PLANE6], amiga->denise.bpldat[PLANE6]);
            break;
            
        default:
            debug("id = %d\n", id);
            assert(false);
    }
    
    // Schedule next event
    uint8_t next = nextDmaEvent[hpos];
    // debug("id = %d hpos = %d, next = %d\n", id, hpos, next);
    if (next) {
        eventHandler.schedulePos(DMA_SLOT, vpos, next, dmaEvent[next]);
    } else {
        eventHandler.cancel(DMA_SLOT);
    }
}

void
Agnus::serviceRASEvent(EventID id)
{
    switch (id) {
            
        case RAS_HSYNC:
            
            hsyncHandler();
            break;
            
        case RAS_DIWSTRT:
            
            if (amiga->debugDMA) debug("RAS_DIWSTRT: (%d,%d)\n", vpos, hpos);
            amiga->denise.draw16();
            break;
            
        case RAS_DIWDRAW:

            if (amiga->debugDMA) debug("RAS_DIWDRAW: (%d,%d)\n", vpos, hpos);
            amiga->denise.draw16();
            break;
            
        default:
            assert(false);
            break;
    }
    
    // Schedule next RAS event
    scheduleNextRASEvent(vpos, hpos);
}

void
Agnus::scheduleNextRASEvent(int16_t vpos, int16_t hpos)
{
    // Map hstrt, hstop to DMA cycle values
    uint16_t hstrtdma = hstrt / 2;
    uint16_t hstopdma = hstop / 2;

    // Check if the vertical position is inside the drawing area
    if (vpos > 25 && vpos >= vstrt && vpos <= vstop) {
        
        // Check if the next event is the first DIW event in this line
        if (hpos < hstrtdma) {
            // debug("Next RAS event is %d at (%d,%d)\n", RAS_DIWSTRT, vpos, hstrt);
            eventHandler.schedulePos(RAS_SLOT, vpos, hstrtdma, RAS_DIWSTRT);
            return;
        }
        
        // Check if there is another DIW event to come in this line
        if (hpos < hstopdma) {
            // debug("Next RAS event is %d at (%d,%d)\n", RAS_DIWDRAW, vpos, hstrt);
            eventHandler.schedulePos(RAS_SLOT, vpos, hpos + 8, RAS_DIWDRAW);
            return;
        }
        
        // If we come here, all DIW events have been processed
    }
    
    // Schedule a HSYNC event to finish up the current line
    eventHandler.schedulePos(RAS_SLOT, vpos, HPOS_MAX, RAS_HSYNC);
}

void
Agnus::hsyncHandler()
{
    // Make sure that we are really at the end of the line
    if (hpos != 226) {
        dump();
    }
    assert(hpos == 226 /* 0xE2 */);
    
    // CIA B counts HSYNCs
    amiga->ciaB.incrementTOD();
    
    // Check the keyboard about each millisecond
    if ((vpos & 0b1111) == 0) amiga->keyboard.execute();
    
    // Add bit plane pointer modulo values
    bplpt[0] += bpl1mod;
    bplpt[1] += bpl2mod;
    bplpt[2] += bpl1mod;
    bplpt[3] += bpl2mod;
    bplpt[4] += bpl1mod;
    bplpt[5] += bpl2mod;
    
    // Increment vpos and reset hpos
    
    /* Important: When the end of a line is reached, we reset the horizontal
     * counter. The new value should be 0. To make things work, we have to set
     * it to -1, because there is an upcoming hpos++ instruction at the end
     * of executeUntil(). This means that we can not rely on the correct
     * hpos value in the hsync and vsync handlers(). The value will be
     * -1 and not 0 as expected. Take care of that and fell free to come up
     * with a nicer solution!
     */
    vpos++;
    hpos = -1;
    
    // Check if the current frame is finished
    if (vpos > VPOS_MAX) {
        vsyncHandler();
    }
    
    // Check if have reached line 26 (where bitplane DMA starts)
    if (vpos == 26) {
        buildDMAEventTable();
    }
    
    // Schedule the first hi-prio DMA event (if any)
    if (nextDmaEvent[0]) {
        EventID eventID = dmaEvent[nextDmaEvent[0]];
        eventHandler.schedulePos(DMA_SLOT, 26, nextDmaEvent[0], eventID);
    }
    
    // Schedule first RAS event
    scheduleNextRASEvent(vpos, hpos);
}

void
Agnus::vsyncHandler()
{
    // Increment frame and reset vpos
    frame++;
    vpos = 0;
    
    // debug("[%d]\n", frame);
    
    // Remember the clock count at SOF (Start Of Frame)
    // Add one because the DMA clock hasn't been advanced yet
    latchedClock = clock + DMA_CYCLES(1);
    
    // CIA A counts VSYNCs
    amiga->ciaA.incrementTOD();
    
    // Trigger VSYNC interrupt
    amiga->paula.pokeINTREQ(0x8020);
    
    // Let the sub components do their own VSYNC stuff
    copper.vsyncAction();
    amiga->denise.endOfFrame();
    
    // Prepare to take a snapshot once in a while
    if (amiga->snapshotIsDue()) amiga->signalSnapshot();
        
    // Count some sheep (zzzzzz) ...
    if (!amiga->getWarp()) {
        amiga->synchronizeTiming();
    }
}
