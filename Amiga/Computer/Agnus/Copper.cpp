// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Copper::Copper()
{
    setDescription("Copper");
        
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &state,    sizeof(state),    0 },
        { &coplc,    sizeof(coplc),    DWORD_ARRAY },
        { &cdang,    sizeof(cdang),    0 },
        { &copins,   sizeof(copins),   0 },
        { &coppc,    sizeof(coppc),    0 },
    });
}

CopperInfo
Copper::getInfo()
{
    CopperInfo info;
    
    info.cdang = cdang;
    info.coppc = coppc;
    for (unsigned i = 0; i < 2; i++) info.coplc[i] = coplc[i];
    
    return info;
}

bool
Copper::illegalAddress(uint32_t address)
{
    address &= 0xFF;
    return address >= (cdang ? 0x40 : 0x80);
}

void
Copper::_powerOn()
{
    
}

void
Copper::_powerOff()
{
    
}

void
Copper::_reset()
{
    
}

void
Copper::_ping()
{
    
}

void
Copper::_dump()
{
    plainmsg("   cdang: %lld\n", cdang);
}

void
Copper::pokeCOPCON(uint16_t value)
{
    debug("pokeCOPCON(%X)\n", value);
    
    /* "This is a 1-bit register that when set true, allows the Copper to
     *  access the blitter hardware. This bit is cleared by power-on reset, so
     *  that the Copper cannot access the blitter hardware." [HRM]
     */
    cdang = (value & 0b10) != 0;
}

void
Copper::pokeCOPJMP(int x)
{
    assert(x < 2);
    
    debug("pokeCOPJMP%d\n", x);
    
    /* "When you write to a Copper strobe address, the Copper reloads its
     *  program counter from the corresponding location register." [HRM]
     */
    // coppc = coplc[1];
}

void
Copper::pokeCOPINS(uint16_t value)
{
    copins = value;
}

void
Copper::pokeCOPxLCH(int x, uint16_t value)
{
    assert(x < 2);
    
    debug("pokeCOP%dLCH(%X)\n", x, value);
    coplc[x] = REPLACE_HI_WORD(coplc[x], value);
}

void
Copper::pokeCOPxLCL(int x, uint16_t value)
{
    assert(x < 2);
    
    debug("pokeCOP%dLCL(%X)\n", x, value);
    coplc[x] = REPLACE_LO_WORD(coplc[x], value & 0xFFFE);
}

void
Copper::scheduleEventRel(Cycle delta, int32_t type, int64_t data)
{
    Cycle trigger = amiga->dma.clock + delta;
    amiga->dma.eventHandler.scheduleEvent(COPPER_SLOT, trigger, type, data);
}

void
Copper::cancelEvent()
{
    amiga->dma.eventHandler.cancelEvent(COPPER_SLOT);
}

void
Copper::vsyncAction()
{
    // debug("Copper vsync\n");
    
    /* "At the start of each vertical blanking interval, COP1LC is automatically
     *  used to start the program counter. That is, no matter what the Copper is
     *  doing, when the end of vertical blanking occurs, the Copper is
     *  automatically forced to restart its operations at the address contained
     *  in COPlLC." [HRM]
     */

    // TODO: What is the exact timing here?
    scheduleEventRel(4, COPPER_JMP1);
}
