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
        { &copcon,   sizeof(copcon),   0 },
        
        // DMA pointer registers
        { &dskpt,    sizeof(dskpt),    0 },
        { &bltpt,    sizeof(bltpt),    DWORD_ARRAY },
        { &coplc,    sizeof(coplc),    DWORD_ARRAY },
        { &audlc,    sizeof(audlc),    DWORD_ARRAY },
        { &bplpt,    sizeof(bplpt),    DWORD_ARRAY },
        { &sprptr,   sizeof(sprptr),   DWORD_ARRAY },

        { &copins,   sizeof(copins),   0 },
        { &coppc,    sizeof(coppc),    0 },
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
    
    info.bpl1mod = bpl1mod;
    info.bpl2mod = bpl2mod;
    
    info.dskpt = dskpt;
    for (unsigned i = 0; i < 4; i++) info.bltpt[i] = bltpt[i];
    for (unsigned i = 0; i < 2; i++) info.coplc[i] = coplc[i];
    for (unsigned i = 0; i < 4; i++) info.audlc[i] = audlc[i];
    for (unsigned i = 0; i < 6; i++) info.bplpt[i] = bplpt[i];
    for (unsigned i = 0; i < 8; i++) info.sprptr[i] = sprptr[i];

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
DMAController::pokeCOPCON(uint16_t value)
{
    debug("pokeCOPCON(%X)\n", value);
    copcon = value;
}

void
DMAController::pokeCOPJMP(int x)
{
    assert(x < 2);
    
    debug("pokeCOPJMP%d\n", x);
    
    /* "When you write to a Copper strobe address, the Copper reloads its
     *  program counter from the corresponding location register." [HRM]
     */
    coppc = coplc[1];
}

void
DMAController::pokeCOPINS(uint16_t value)
{
    copins = value;
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
DMAController::pokeCOPxLCH(int x, uint16_t value)
{
    assert(x < 2);
    
    debug("pokeCOP%dLCH(%X)\n", x, value);
    coplc[x] = REPLACE_HI_WORD(coplc[x], value);
}

void
DMAController::pokeCOPxLCL(int x, uint16_t value)
{
    assert(x < 2);
    
    debug("pokeCOP%dLCL(%X)\n", x, value);
    coplc[x] = REPLACE_LO_WORD(coplc[x], value & 0xFFFE);
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
