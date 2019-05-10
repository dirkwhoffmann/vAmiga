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
    
    // Register subcomponents
    registerSubcomponents(vector<HardwareComponent *> {
        
        &audioUnit,
        &diskController
    });
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,    sizeof(clock),    0 },
        
        { &intreq,   sizeof(intreq),   0 },
        { &intena,   sizeof(intena),   0 },
        
        { &serdat,   sizeof(serdat),   0 },
        { &serper,   sizeof(serper),   0 },

        { &potgo,    sizeof(potgo),    0 },
    
        { &adkcon,   sizeof(adkcon),   0 },
    });
}

void
Paula::_powerOn()
{
    
}

void
Paula::_powerOff()
{
    
}

void
Paula::_reset()
{
    
}

void
Paula::_ping()
{
    
}

void
Paula::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.intreq = intreq;
    info.intena = intena;
    info.adkcon = adkcon;
    
    pthread_mutex_unlock(&lock);
}

void
Paula::_dump()
{
    
}

PaulaInfo
Paula::getInfo()
{
    PaulaInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
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

void
Paula::pokeINTREQ(uint16_t value)
{
    // debug("pokeINTREQ(%X)\n", value);
    setINTREQ(value);
}

void
Paula::pokeINTENA(uint16_t value)
{
    // debug("pokeINTENA(%X)\n", value);
    setINTENA(value);
}

void
Paula::pokeADKCON(uint16_t value)
{
    plaindebug("pokeADKCON(%X)\n", value);
    if (value & 0x8000) adkcon |= (value & 0x7FFF); else adkcon &= ~value;
}

void
Paula::setINTREQ(uint16_t value)
{
    // debug("setINTREQ(%X)\n", value);
    
    if (value & 0x8000) intreq |= (value & 0x7FFF); else intreq &= ~value;
    checkInterrupt();
}

void
Paula::setINTENA(uint16_t value)
{
    // debug("setINTENA(%X)\n", value);
    
    if (value & 0x8000) intena |= (value & 0x7FFF); else intena &= ~value;
    checkInterrupt();
}

uint16_t
Paula::peekPOTGOR()
{
    uint16_t result = 0xFFFF;
    
    result &= _port1->potgor();
    result &= _port2->potgor();
    
    return result;
}

void
Paula::pokePOTGO(uint16_t value)
{
    debug(2, "pokePOTGO(%X)\n", value);
    potgo = value;
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
    
    /*
    if (level) {
        uint16_t mask = intena & intreq;
        debug("*** intena: %X inetrq: %X\n", intena, intreq);
        if (mask & 0x0001) { debug("*** SERPORT IRQ (level %d)\n", level); }
        if (mask & 0x0002) { debug("*** DISK DMA IRQ (level %d)\n", level); }
        if (mask & 0x0004) { debug("*** SW IRQ (level %d)\n", level); }
        if (mask & 0x0008) { debug("*** CIA A IRQ (level %d)\n", level); }
        if (mask & 0x0010) { debug("*** COPPER IRQ (level %d)\n", level); }
        if (mask & 0x0020) { debug("*** VERTB IRQ (level %d)\n", level); }
        if (mask & 0x0040) { debug("*** BLIT IRQ (level %d)\n", level); }
        if (mask & 0x0080) { debug("*** AUD0 IRQ (level %d)\n", level); }
        if (mask & 0x0100) { debug("*** AUD1 IRQ (level %d)\n", level); }
        if (mask & 0x0200) { debug("*** AUD2 IRQ (level %d)\n", level); }
        if (mask & 0x0400) { debug("*** AUD3 IRQ (level %d)\n", level); }
        if (mask & 0x0800) { debug("*** Input buf ser port IRQ (level %d)\n", level); }
        if (mask & 0x1000) { debug("*** DISK SYNC IRQ (level %d)\n", level); }
        if (mask & 0x2000) { debug("*** CIA B (level %d)\n", level); }
    } else {
        // debug("*** SETTING IRQ LEVEL TO 0\n");
    }
    */
    
    m68k_set_irq(level);
}
