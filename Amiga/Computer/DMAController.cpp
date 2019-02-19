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
    
    // Register sub components
    registerSubcomponents(vector<HardwareComponent *> {
        
        &eventHandler,
    });
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,    sizeof(clock),    0 },
        { &vhpos,    sizeof(vhpos),    0 },
        { &vpos,     sizeof(vpos),     0 },
        { &dmacon,   sizeof(dmacon),   0 },
        { &diwstrt,  sizeof(diwstrt),  0 },
        { &diwstop,  sizeof(diwstop),  0 },
        { &ddfstrt,  sizeof(ddfstrt),  0 },
        { &ddfstop,  sizeof(ddfstop),  0 },
        { &bplpt,    sizeof(bplpt),    DWORD_ARRAY },
        { &bpl1mod,  sizeof(bpl1mod),  0 },
        { &bpl2mod,  sizeof(bpl2mod),  0 },
        { &sprptr,   sizeof(sprptr),   DWORD_ARRAY }
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
    
    // Schedule the first two CIA events
    eventHandler.scheduleEvent(EVENT_CIAA, CIA_CYCLES(1), 0);
    eventHandler.scheduleEvent(EVENT_CIAB, CIA_CYCLES(1), 0);
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
    plainmsg("   clock: %lld\n", clock);
    plainmsg("   vhpos: %lld $%llX\n", vhpos, vhpos);
    plainmsg("    vpos: %lld $%llX\n", vpos, vpos);
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
    
    for (unsigned i = 0; i < 6; i++)
        info.bplpt[i] = bplpt[i];
    
    info.bpl1mod = bpl1mod;
    info.bpl2mod = bpl2mod;
    
    return info;
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
    
    if (value & 0x8000) dmacon |= value; else dmacon &= ~value;
    dmacon &= 0x07FF;
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
DMAController::pokeBPLxPTL(int x, uint16_t value)
{
    assert(x < 6);
    debug("pokeBPL%dPTL(%X)\n", x, value);
    
    bplpt[x] = (bplpt[x] & 0x70000) | value;
}

void
DMAController::pokeBPLxPTH(int x, uint16_t value)
{
    assert(x < 6);
    debug("pokeBPL%dPTH(%X)\n", x, value);

    bplpt[x] = (bplpt[x] & 0x0FFFF) | ((value << 16) & 0x70000);
}

void
DMAController::pokeSPRxPTL(int x, uint16_t value)
{
    assert(x < 8);
    sprptr[x] = (sprptr[x] & 0xFFFF0000) | value;
}

void
DMAController::pokeSPRxPTH(int x, uint16_t value)
{
    assert(x < 8);
    sprptr[x] = (value << 16) | (sprptr[x] & 0xFFFF);
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
DMAController::executeUntil(Cycle targetClock)
{
    // msg("clock is %lld, Executing until %lld\n", clock, targetClock);
    while (clock <= targetClock - DMA_CYCLES(1)) {
        
        clock += DMA_CYCLES(1);
        
        // Determine number of master clock cycles to execute
        // Cycle missingCycles = targetClock - clock;
        
        // Convert to DMA cycles
        // DMACycle missingDMACycles = AS_DMA_CYCLES(missingCycles);
        
        // Execute until target clock is reached ...
        // for (DMACycle i = 0; i < missingDMACycles; i++) {
        
        // Process all pending events
        eventHandler.executeUntil(clock);
        
        // Perform DMA
        switch (vhpos) {
            case 0x07:
            case 0x09:
            case 0x0B:
                // Do disk DMA
                break;
                
            case 0x0D:
            case 0x0F:
            case 0x11:
            case 0x13:
                // Do Audio DMA
                break;
                
            // AND SO ON ...
                
            default:
                break;
        }
        
        // Check if the current rasterline has been completed
        if (vhpos < 227) vhpos++; else hsyncAction();
    }
}

void
DMAController::hsyncAction()
{
    vhpos = 0;
    
    // CIA B counts HSYNCs
    amiga->ciaB.incrementTOD();
    
    // Check if the current frame has been completed
    if (vpos < 312) vpos++; else vsyncAction();
}

void
DMAController::vsyncAction()
{
    vpos = 0;
    
    // CIA A counts VSYNCs
    amiga->ciaA.incrementTOD();
}
