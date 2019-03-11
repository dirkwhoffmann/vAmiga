// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Paula::Paula()
{
    setDescription("Paula");
    
    // Register sub components
    registerSubcomponents(vector<HardwareComponent *> {
        
        &audioUnit
    });
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,    sizeof(clock),    0 },
        
        { &intreq,   sizeof(intreq),   0 },
        { &intena,   sizeof(intena),   0 },

        { &dsklen,   sizeof(dsklen),   0 },
        { &dskdat,   sizeof(dskdat),   0 },

        { &serdat,   sizeof(serdat),   0 },
        { &serper,   sizeof(serper),   0 },

        { &potgo,    sizeof(potgo),    0 },
        
        { &audlen,   sizeof(audlen),   WORD_ARRAY },
        { &audper,   sizeof(audper),   WORD_ARRAY },
        { &audvol,   sizeof(audvol),   WORD_ARRAY },
        { &auddat,   sizeof(auddat),   WORD_ARRAY },

    });
    
}

void
Paula::_powerOn()
{
    
}

void
Paula::_setWarp(bool value)
{
    /* Warping has the unavoidable drawback that audio playback gets out of
     * sync. To cope with this issue, we ramp down the volume when entering
     * warp mode and fade in smoothly when warping ends.
     */
    if (value) {
        
        audioUnit.rampDown();
        
    } else {
        
        audioUnit.rampUp();
        audioUnit.alignWritePtr();
    }
}

PaulaInfo
Paula::getInfo()
{
    PaulaInfo info;
    
    info.intreq = intreq;
    info.intena = intena;
    
    return info;
}

uint16_t
Paula::peekINTREQ()
{
       return intreq;
}

void
Paula::pokeINTREQ(uint16_t value)
{
    // debug("pokeINTREQ(%X)\n", value);
    setINTREQ(value);
}

uint16_t
Paula::peekINTENA()
{
    return intena;
}

void
Paula::pokeINTENA(uint16_t value)
{
    // debug("pokeINTENA(%X)\n", value);
    setINTENA(value);
}

void
Paula::setINTREQ(uint16_t value)
{
    debug("setINTREQ(%X)\n", value);
    
    if (value & 0x8000) intreq |= (value & 0x7FFF); else intreq &= ~value;
    checkInterrupt();
}

void
Paula::setINTENA(uint16_t value)
{
    debug("setINTENA(%X)\n", value);
    
    if (value & 0x8000) intena |= (value & 0x7FFF); else intena &= ~value;
    checkInterrupt();
}

uint16_t
Paula::peekPOTGO()
{
    return 0xFFFF;
}

void
Paula::pokePOTGO(uint16_t value)
{
    debug("pokePOTGO(%X)\n", value);

    potgo = value;
}

void
Paula::pokeAUDxLEN(int x, uint16_t value)
{
    debug("pokeAUD%dLEN(%X)\n", x, value);
    assert(x < 4);
    
    audlen[x] = value;
}

void
Paula::pokeAUDxPER(int x, uint16_t value)
{
    debug("pokeAUD%dPER(%X)\n", x, value);
    assert(x < 4);
    
    audper[x] = value;
}

void
Paula::pokeAUDxVOL(int x, uint16_t value)
{
    debug("pokeAUD%dVOL(%X)\n", x, value);
    assert(x < 4);
    
    audvol[x] = MIN(value, 64);
}

void
Paula::pokeAUDxDAT(int x, uint16_t value)
{
    debug("pokeAUD%dDAT(%X)\n", x, value);
    assert(x < 4);
    
    auddat[x] = value;
}

int
Paula::interruptLevel()
{
    uint16_t mask = intreq & intena;
    
    if (intena & 0x4000) {
        if (mask & 0b0110000000000000) return 6;
        if (mask & 0b0001100000000000) return 5;
        if (mask & 0b0000011110000000) return 4;
        if (mask & 0b0000000001110000) return 3;
        if (mask & 0b0000000000001000) return 2;
        if (mask & 0b0000000000000111) return 1;
    }

    return 0;
}

void
Paula::checkInterrupt()
{
    int level = interruptLevel();
        
    if (level) {
        debug("*** TRIGGERING LEVEL %d INTERRUPT: mask = %X\n", level, intena & intreq);
    } else {
        debug("*** SETTING IRQ LEVEL TO 0\n");
    }
    
    m68k_set_irq(level);
}

