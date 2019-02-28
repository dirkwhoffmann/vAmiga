// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DMAController::DMAController()
{
    setDescription("DMAController");
    
    registerSubcomponents(vector<HardwareComponent *> {
        
        &eventHandler,
        &copper,
    });
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,              sizeof(clock),              0 },
        { &beam,               sizeof(beam),               0 },
        { &lores,              sizeof(lores),              0 },
        { &activeBitplanes,    sizeof(activeBitplanes),    0 },
        { &busOwner,           sizeof(busOwner),           0 },
        { &dmacon,             sizeof(dmacon),             0 },
        { &diwstrt,            sizeof(diwstrt),            0 },
        { &diwstop,            sizeof(diwstop),            0 },
        { &ddfstrt,            sizeof(ddfstrt),            0 },
        { &ddfstop,            sizeof(ddfstop),            0 },
        
        // DMA pointer registers
        { &dskpt,    sizeof(dskpt),    0 },
        { &bltpt,    sizeof(bltpt),    DWORD_ARRAY },
        { &audlc,    sizeof(audlc),    DWORD_ARRAY },
        { &bplpt,    sizeof(bplpt),    DWORD_ARRAY },
        { &sprptr,   sizeof(sprptr),   DWORD_ARRAY },

        { &bpl1mod,  sizeof(bpl1mod),  0 },
        { &bpl2mod,  sizeof(bpl2mod),  0 },
    });
}

DMAController::~DMAController()
{
    debug(2, "Destroying DMAController[%p]\n", this);
}

void
DMAController::_powerOn()
{
    clock = 0;
    
    // Schedule the first HSYNC event
    eventHandler.scheduleEvent(RAS_SLOT, DMA_CYCLES(0xE2), RAS_HSYNC);
    
    // Schedule the first two CIA events
    eventHandler.scheduleEvent(CIAA_SLOT, CIA_CYCLES(1), CIA_EXECUTE);
    eventHandler.scheduleEvent(CIAB_SLOT, CIA_CYCLES(1), CIA_EXECUTE);
}

void
DMAController::_powerOff()
{
    
}
void
DMAController::_reset()
{
    
}
void
DMAController::_ping()
{
    
}
void
DMAController::_dump()
{
    uint16_t vp = VPOS(beam);
    uint16_t hp = HPOS(beam);

    plainmsg("        frame: %lld\n", frame);
    plainmsg("        clock: %lld master cycles\n", clock);
    plainmsg("               %lld DMA cycles\n", AS_DMA_CYCLES(clock));
    plainmsg("latched clock: %lld master cycles\n", latchedClock);
    plainmsg("               %lld DMA cycles\n", AS_DMA_CYCLES(latchedClock));
    plainmsg("beam position: %d $%X (%d,%d) ($%X,$%X)\n", beam, beam, vp, hp, vp, hp);
}

DMAInfo
DMAController::getInfo()
{
    DMAInfo info;
    
    info.dmacon = dmacon;
    info.diwstrt = diwstrt;
    info.diwstop = diwstop;
    info.ddfstrt = ddfstrt;
    info.ddfstop = ddfstop;
    
    info.bpl1mod = bpl1mod;
    info.bpl2mod = bpl2mod;
    
    info.dskpt = dskpt;
    for (unsigned i = 0; i < 4; i++) info.bltpt[i] = bltpt[i];
    for (unsigned i = 0; i < 4; i++) info.audlc[i] = audlc[i];
    for (unsigned i = 0; i < 6; i++) info.bplpt[i] = bplpt[i];
    for (unsigned i = 0; i < 8; i++) info.sprptr[i] = sprptr[i];

    return info;
}

Cycle
DMAController::cyclesInCurrentFrame()
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

Cycle
DMAController::beam2cycles(int16_t vpos, int16_t hpos)
{
    return DMA_CYCLES(vpos * cyclesPerLine() + hpos); 
}

int32_t
DMAController::nextBPLDMABeam(uint32_t currentBeam)
{
    // The first DMA cycle happens at (26, ddfstrt) (TODO: CORRECT?)
    if (currentBeam < BEAM(26, ddfstrt))
        return BEAM(26, ddfstrt);
    
    // The last DMA cycle happens at (312, ddfstop) (TODO: CORRECT?)
    if (currentBeam > BEAM(312, ddfstop))
        return -1;
    
    int32_t vpos = currentBeam >> 8;
    int32_t hpos = currentBeam & 0xFF;
    
    // If vpos is greater than ddfstop, we pretend to be in the next line
    if (hpos >= ddfstop) {
        vpos++;
        hpos = ddfstrt;
    }
    
    // We're inside the active DMA area now
    if (amiga->denise.hires())
        return BEAM(vpos, hpos);
    else
        return UP_TO_NEXT_ODD(BEAM(vpos, hpos));
}

Cycle
DMAController::nextBpldmaCycle(uint32_t currentBeam)
{
    Cycle result = latchedClock;
    
    int32_t beam = nextBPLDMABeam(currentBeam);
    
    if (beam == -1) { // Jump to next frame
        result += cyclesInCurrentFrame();
        beam = BEAM(26, ddfstrt);
    }
    
    result += beam2cycles(beam);
    return result;
}

uint16_t
DMAController::peekDMACON()
{
    return dmacon;
}

void
DMAController::pokeDMACON(uint16_t value)
{
    debug("pokeDMACON(%X)\n", value);
    
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
            debug("Bitplane DMA switched on");

        } else {
            
            // Bitplane DMA off
            debug("Bitplane DMA switched off");
        }
    }
    
    // Copper DMA
    if (oldCOPEN ^ newCOPEN) {
        
        if (newCOPEN) {
            
            // Copper DMA on
            debug("Copper DMA switched on");
            eventHandler.scheduleEvent(COP_SLOT, 2, COP_FETCH);
            
        } else {
            
            // Copper DMA off
            debug("Copper DMA switched off");
            eventHandler.cancelEvent(COP_SLOT);
        }
    }
    
    // Blitter DMA
    if (oldBLTEN ^ newBLTEN) {
        
        if (newBLTEN) {
            // Blitter DMA on
            debug("Blitter DMA switched on");
            
        } else {
            
            // Blitter DMA off
            debug("Blitter DMA switched off");
        }
    }
    
    // Sprite DMA
    if (oldSPREN ^ newSPREN) {
        
        if (newSPREN) {
            // Sprite DMA on
            debug("Sprite DMA switched on");
            
        } else {
            
            // Sprite DMA off
            debug("Sprite DMA switched off");
        }
    }
    
    // Disk DMA
    if (oldDSKEN ^ newDSKEN) {
        
        if (newDSKEN) {
            
            // Disk DMA on
            debug("Disk DMA switched on");
            
        } else {
            
            // Disk DMA off
            debug("Disk DMA switched off");
        }
    }
}

uint16_t
DMAController::peekVHPOS()
{
    // V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1
    return beam & 0xFFFF;
}

uint16_t
DMAController::peekVPOS()
{
    // LF -- -- -- -- -- -- -- -- -- -- -- -- -- -- V8
    // TODO: LF (Long Frame)
    return(beam >> 16) & 1;

}

void
DMAController::pokeDIWSTRT(uint16_t value)
{
    debug("pokeDIWSTRT(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0, H8 = 0
    
    diwstrt = value;
    hstrt = LO_BYTE(value);
    vstrt = HI_BYTE(value);
}

void
DMAController::pokeDIWSTOP(uint16_t value)
{
    debug("pokeDIWSTOP(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // V7 V6 V5 V4 V3 V2 V1 V0 H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1, V8 = !V7
    
    diwstop = value;
    hstop = LO_BYTE(value) | 0x100;
    vstop = HI_BYTE(value) | ((value & 0x80) << 1);
}

void
DMAController::pokeDDFSTRT(uint16_t value)
{
    debug("pokeDDFSTRT(%X)\n", value);
    
    ddfstrt = value;
}

void
DMAController::pokeDDFSTOP(uint16_t value)
{
    debug("pokeDDFSTOP(%X)\n", value);
    
    ddfstop = value;
}
        


void
DMAController::pokeBPL1MOD(uint16_t value)
{
    debug("pokeBPL1MOD(%X)\n", value);

    bpl1mod = value;
}

void
DMAController::pokeBPL2MOD(uint16_t value)
{
    debug("pokeBPL2MOD(%X)\n", value);
    
    bpl2mod = value;
}

void
DMAController::pokeDSKPTH(uint16_t value)
{
    debug("pokeDSKPTH(%X)\n", value);
    dskpt = REPLACE_HI_WORD(dskpt, value & 0x7);
}

void
DMAController::pokeDSKPTL(uint16_t value)
{
    debug("pokeDSKPTL(%X)\n", value);
    dskpt = REPLACE_LO_WORD(dskpt, value);
}

void
DMAController::pokeBLTxPTH(int x, uint16_t value)
{
    assert(x < 4);
    
    debug("pokeBLT%dPTH(%X)\n", x, value);
    bltpt[x] = REPLACE_HI_WORD(bltpt[x], value & 0x7);
}

void
DMAController::pokeBLTxPTL(int x, uint16_t value)
{
    assert(x < 4);

    debug("pokeBLT%dPTL(%X)\n", x, value);
    bltpt[x] = REPLACE_LO_WORD(bltpt[x], value);
}

void
DMAController::pokeAUDxLCH(int x, uint16_t value)
{
    assert(x < 4);
    
    debug("pokeAUD%dLCH(%X)\n", x, value);
    audlc[x] = REPLACE_HI_WORD(audlc[x], value & 0x7);
}

void
DMAController::pokeAUDxLCL(int x, uint16_t value)
{
    assert(x < 4);
    
    debug("pokeAUD%dLCL(%X)\n", x, value);
    audlc[x] = REPLACE_LO_WORD(audlc[x], value);
}

void
DMAController::pokeBPLxPTH(int x, uint16_t value)
{
    assert(x < 6);
    
    debug("pokeBPL%dPTH(%X)\n", x, value);
    bplpt[x] = REPLACE_HI_WORD(bplpt[x], value & 0x7);
}

void
DMAController::pokeBPLxPTL(int x, uint16_t value)
{
    assert(x < 6);
    
    debug("pokeBPL%dPTL(%X)\n", x, value);
    bplpt[x] = REPLACE_LO_WORD(bplpt[x], value);
}

void
DMAController::pokeSPRxPTH(int x, uint16_t value)
{
    assert(x < 8);
    
    debug("pokeSPR%dPTH(%X)\n", x, value);
    sprptr[x] = REPLACE_HI_WORD(sprptr[x], value & 0x7);
}

void
DMAController::pokeSPRxPTL(int x, uint16_t value)
{
    assert(x < 8);
    
    debug("pokeSPR%dPTL(%X)\n", x, value);
    sprptr[x] = REPLACE_LO_WORD(sprptr[x], value);
}




void
DMAController::executeUntil(Cycle targetClock)
{
    // msg("clock is %lld, Executing until %lld\n", clock, targetClock);
    while (clock <= targetClock - DMA_CYCLES(1)) {
   
        busOwner = 0;
        
        // Process all pending events
        eventHandler.executeUntil(clock);
        
        // Advance the internal counters
        beam++; 
        clock += DMA_CYCLES(1);
    }
}

DMACycle
DMAController::beamDiff(uint32_t start, uint32_t end)
{
    int32_t vStart = (int32_t)VPOS(start);
    int32_t hStart = (int32_t)HPOS(start);
    int32_t vEnd   = (int32_t)VPOS(end);
    int32_t hEnd   = (int32_t)HPOS(end);
    
    // We assume that the function is called with a valid horizontal position
    assert(hEnd <= 0xE2);

    // Bail out if the end position is unreachable
    if (vEnd > 312) return INT64_MAX;

    // Compute vertical and horizontal difference
    int32_t vDiff  = vEnd - vStart;
    int32_t hDiff  = hEnd - hStart;
    
    // In PAL mode, all lines have the same length (227 color clocks)
    return vDiff * 227 + hDiff;
}

void
DMAController::hsyncHandler()
{
    // Verify that the event has been triggered at the correct beam position
    assert(HPOS(beam) == 226 /* 0xE2 */);
    
    // Reset horizontal position
    // Setting it to -1 ensures that it is 0 at the end of executeUntil()
    sethpos(-1);
    
    // CIA B counts HSYNCs
    amiga->ciaB.incrementTOD();
    
    // Check if the current frame has been completed
    VPOS(beam) < 312 ? incvpos() : vsyncAction();
    
    // Schedule next event
    eventHandler.rescheduleEvent(RAS_SLOT, DMA_CYCLES(0xE3));
}

void
DMAController::vsyncAction()
{
    debug("vsyncAction");
    
    // CIA A counts VSYNCs
    amiga->ciaA.incrementTOD();
    
    // Execute sub components
    copper.vsyncAction(); 

    // Advance counters
    frame++;
    latchedClock = clock + DMA_CYCLES(1);
    setvpos(0);
    
    // Schedule next bitplane DMA event
    if (bplDMA()) {
        if (amiga->denise.lores()) {
            eventHandler.scheduleEvent(BPL_SLOT, 27, ddfstrt, BPL_FETCH_LORES, 4);
        } else {
            eventHandler.scheduleEvent(BPL_SLOT, 26, ddfstrt, BPL_FETCH_HIRES, 4);
        }
    }
}

void
DMAController::addBPLxMOD()
{
    // Add bpl2mod is added to all active even bitplane pointers
    // Add blp1mod is added to all active odd bitplane pointers
    
    switch (activeBitplanes) {
        case 6: bplpt[5] = INC_OCS_PTR(bplpt[5], bpl2mod); // fallthrough
        case 5: bplpt[4] = INC_OCS_PTR(bplpt[4], bpl1mod); // fallthrough
        case 4: bplpt[3] = INC_OCS_PTR(bplpt[3], bpl2mod); // fallthrough
        case 3: bplpt[2] = INC_OCS_PTR(bplpt[2], bpl1mod); // fallthrough
        case 2: bplpt[1] = INC_OCS_PTR(bplpt[1], bpl2mod); // fallthrough
        case 1: bplpt[0] = INC_OCS_PTR(bplpt[0], bpl1mod);
    }
}


bool
DMAController::copperCanHaveBus()
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
DMAController::doBplDMA1()
{
    doBplDMA(0);
    amiga->denise.fillShiftRegisters();
}

void
DMAController::doBplDMA(int plane)
{
    assert(0 <= plane && plane <= 5);
    
    if (plane < activeBitplanes) {
        busOwner = BPLEN;
        amiga->denise.bpldat[plane] = amiga->mem.peekChip16(bplpt[plane]);
        bplpt[plane] = INC_OCS_PTR(bplpt[plane], 2);
    }
}
