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
        
        { &clock,    sizeof(clock),    0 },
        { &beam,     sizeof(beam),     0 },
        { &dmacon,   sizeof(dmacon),   0 },
        { &diwstrt,  sizeof(diwstrt),  0 },
        { &diwstop,  sizeof(diwstop),  0 },
        { &ddfstrt,  sizeof(ddfstrt),  0 },
        { &ddfstop,  sizeof(ddfstop),  0 },
        
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
    plainmsg("   clock: %lld\n", clock);
    plainmsg("    beam: %d $%X\n", beam);
    plainmsg("    hpos: %d $%X\n", hpos());
    plainmsg("    vpos: %d $%X\n", vpos());
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
        
        // Determine number of master clock cycles to execute
        // Cycle missingCycles = targetClock - clock;
        
        // Convert to DMA cycles
        // DMACycle missingDMACycles = AS_DMA_CYCLES(missingCycles);
        
        // Execute until target clock is reached ...
        // for (DMACycle i = 0; i < missingDMACycles; i++) {
        
        // Process all pending events
        eventHandler.executeUntil(clock);
        
        // Perform DMA
        switch (hpos()) {
            case 0x00: case 0x02: case 0x04: case 0x06:
            case 0x08: case 0x0A: case 0x0C: case 0x0E:
            case 0x10: case 0x12: case 0x14: case 0x16:
            case 0x18: case 0x1A: case 0x1C: case 0x1E:
            case 0x20: case 0x22: case 0x24: case 0x26:
            case 0x28: case 0x2C: case 0x30: case 0x34:
            case 0xD8: case 0xDC:
            case 0xE2: // (?!)
                
                // Copper or Blitter only
                break;
                
                
            case 0x01: case 0x03: case 0x05:
                
                // Memory refresh
                break;
    
            case 0x07: case 0x09: case 0x0B:
                
                // Disk DMA
                break;
                
                
            case 0x0D: case 0x0F: case 0x11: case 0x13:
                
                // Audio DMA
                break;
                
            case 0x15: case 0x17:
                
                // Sprite 0 DMA
                break;
                
            case 0x19: case 0x1B:
                
                // Sprite 1 DMA
                break;
 
            case 0x1D: case 0x1F:
            
                // Sprite 2 DMA
                break;
  
            case 0x21: case 0x23:
                
                // Sprite 3 DMA
                break;
                
            case 0x25: case 0x27:
                
                // Sprite 4 DMA
                break;
                
            case 0x29: // S5 or L4
                break;
                
            case 0x2B: // S5 or L2
                break;

            case 0x2D: // S6 or L3
                break;
                
            case 0x2F: // S6 or L1
                break;

            case 0x31: // S7 or L4
                break;

            case 0x33: // S7 or L2
                break;

            case 0x37: case 0xDF: // L1
                break;
  
            case 0xDB: // L2
                break;
                
            case 0x35: case 0xDD: // L3
                break;

            case 0xD9: // L4
                break;

            case 0x2E: case 0x36: case 0xDE: // L5
                break;
           
            case 0x2A: case 0x32: case 0xDA: // L6
                break;

            case 0x38: case 0x3C: case 0x40: case 0x44: // H4
            case 0x48: case 0x4C: case 0x50: case 0x54:
            case 0x58: case 0x5C: case 0x60: case 0x64:
            case 0x68: case 0x6C: case 0x70: case 0x74:
            case 0x78: case 0x7C: case 0x80: case 0x84:
            case 0x88: case 0x8C: case 0x90: case 0x94:
            case 0x98: case 0x9C: case 0xA0: case 0xA4:
            case 0xA8: case 0xAC: case 0xB0: case 0xB4:
            case 0xB8: case 0xBC: case 0xC0: case 0xC4:
            case 0xC8: case 0xCC: case 0xD0: case 0xD4:
                break;
   
            case 0x39: case 0x41: case 0x49: case 0x51: // L4 or H3
            case 0x59: case 0x61: case 0x69: case 0x71:
            case 0x79: case 0x81: case 0x89: case 0x91:
            case 0x99: case 0xA1: case 0xA9: case 0xB1:
            case 0xB9: case 0xC1: case 0xC9: case 0xD1:
                break;
  
            case 0x3A: case 0x42: case 0x4A: case 0x52: // L6 or H2
            case 0x5A: case 0x62: case 0x6A: case 0x72:
            case 0x7A: case 0x82: case 0x8A: case 0x92:
            case 0x9A: case 0xA2: case 0xAA: case 0xB2:
            case 0xBA: case 0xC2: case 0xCA: case 0xD2:
                break;
  
            case 0x3B: case 0x43: case 0x4B: case 0x53: // L2 or H1
            case 0x5B: case 0x63: case 0x6B: case 0x73:
            case 0x7B: case 0x83: case 0x8B: case 0x93:
            case 0x9B: case 0xA3: case 0xAB: case 0xB3:
            case 0xBB: case 0xC3: case 0xCB: case 0xD3:
                break;
  
            case 0x3D: case 0x45: case 0x4D: case 0x55: // L3 or H3
            case 0x5D: case 0x65: case 0x6D: case 0x75:
            case 0x7D: case 0x85: case 0x8D: case 0x95:
            case 0x9D: case 0xA5: case 0xAD: case 0xB5:
            case 0xBD: case 0xC5: case 0xCD: case 0xD5:
                break;
 
            case 0x3E: case 0x46: case 0x4E: case 0x56: // L5 or H2
            case 0x5E: case 0x66: case 0x6E: case 0x76:
            case 0x7E: case 0x86: case 0x8E: case 0x96:
            case 0x9E: case 0xA6: case 0xAE: case 0xB6:
            case 0xBE: case 0xC6: case 0xCE: case 0xD6:
                break;
                
            case 0x3F: case 0x47: case 0x4F: case 0x57: // L1 or H1
            case 0x5F: case 0x67: case 0x6F: case 0x77:
            case 0x7F: case 0x87: case 0x8F: case 0x97:
            case 0x9F: case 0xA7: case 0xAF: case 0xB7:
            case 0xBF: case 0xC7: case 0xCF: case 0xD7:
                break;
            
            case 0xE0: case 0xE1:
                // Unusable (?!)
                break;
                
            case 0xE3:
                
                // This is the last PAL cycle
                hsyncAction();
                break;
                
            default:
                warn("UNIMPLEMENTED CYCLE %X\n", hpos());
                assert(false);
        }
        
        // Advance the internal counters
        inchpos();
        clock += DMA_CYCLES(1);
    }
}

void
DMAController::hsyncAction()
{
    // We set the new horizonzal position to -1. This ensures that it will
    // ne zero when the reach the end of function executeUntil()
    sethpos(-1);
    
    // CIA B counts HSYNCs
    amiga->ciaB.incrementTOD();
    
    // Check if the current frame has been completed
    if (vpos() < 312) incvpos(); else vsyncAction();
}

void
DMAController::vsyncAction()
{
    setvpos(0);
    
    // CIA A counts VSYNCs
    amiga->ciaA.incrementTOD();
    
    copper.vsyncAction(); 
}
