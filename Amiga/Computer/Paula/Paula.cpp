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
        &diskController,
        &uart
    });
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,    sizeof(clock),    0 },
        
        { &intreq,   sizeof(intreq),   0 },
        { &intena,   sizeof(intena),   0 },
        
        { &potgo,    sizeof(potgo),    0 },
        { &potCntX0, sizeof(potCntX0), 0 },
        { &potCntY0, sizeof(potCntY0), 0 },
        { &potCntX1, sizeof(potCntX1), 0 },
        { &potCntY1, sizeof(potCntY1), 0 },

        { &adkcon,   sizeof(adkcon),   0 },
    });
}

void
Paula::_initialize()
{
    agnus = &amiga->agnus;
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
    plaindebug(2, "pokeADKCON(%X)\n", value);

    // uint16_t oldAdkcon = adkcon;

    if (value & 0x8000) adkcon |= (value & 0x7FFF); else adkcon &= ~value;

    // Take care of a possible change of the UARTBRK bit
    uart.updateTXD();
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
Paula::peekPOTxDAT(int x)
{
    assert(x == 0 || x == 1);

    uint8_t potX, potY;
    uint16_t result;

    int outy = x ? 15 : 11;
    int daty = x ? 14 : 10;
    int outx = x ? 13 : 9;
    int datx = x ? 12 : 8;

    if GET_BIT(potgo, outy) { // POTY is configured as output?
        potY = GET_BIT(potgo, daty) ? 0xFF : 0x00;
    } else {
        potY = potCntY0;
    }

    if GET_BIT(potgo, outx) { // POTX is configured as output?
        potX = GET_BIT(potgo, datx) ? 0xFF : 0x00;
    } else {
        potX = potCntX0;
    }

    result = HI_LO(potY, potX);

    debug(POT_DEBUG, "peekPOT%dDAT() = %X\n", x, result);
    return result;
}

uint16_t
Paula::peekPOTGOR()
{
    uint16_t result = 0;

    bool outry = GET_BIT(potgo, 15);
    bool datry = GET_BIT(potgo, 14);
    bool outrx = GET_BIT(potgo, 13);
    bool datrx = GET_BIT(potgo, 12);
    bool outly = GET_BIT(potgo, 11);
    bool datly = GET_BIT(potgo, 10);
    bool outlx = GET_BIT(potgo,  9);
    bool datlx = GET_BIT(potgo,  8);

    bool poty0 = amiga->controlPort1.getPotY() & 0x80;
    bool potx0 = amiga->controlPort1.getPotX() & 0x80;
    bool poty1 = amiga->controlPort2.getPotY() & 0x80;
    bool potx1 = amiga->controlPort2.getPotX() & 0x80;

    WRITE_BIT(result, 14, outry ? datry : poty0);
    WRITE_BIT(result, 12, outrx ? datrx : potx0);
    WRITE_BIT(result, 10, outly ? datly : poty1);
    WRITE_BIT(result,  8, outlx ? datlx : potx1);

    // Connected devices can pull down lines even if they are configured
    // as outputs
    result &= amiga->controlPort1.potgor();
    result &= amiga->controlPort2.potgor();

    debug(POT_DEBUG, "peekPOTGOR = %X\n", result);
    return result;
}

void
Paula::pokePOTGO(uint16_t value)
{
    debug(POT_DEBUG, "pokePOTGO(%X)\n", value);

    potgo = value;

    // Writing into this register clears the potentiometer counter
    potCntX0 = 0;
    potCntY0 = 0;
    potCntX1 = 0;
    potCntY1 = 0;

    // Check the START bit
    if (GET_BIT(value, 0)) {
        debug(POT_DEBUG, "Starting potentiometer scan procedure\n");

        // Schedule the first DISCHARGE event
        agnus->schedulePos<POT_SLOT>(agnus->vpos, HPOS_MAX, POT_DISCHARGE);
        agnus->slot[POT_SLOT].data = 8;
    }
}

void
Paula::servePotEvent(EventID id)
{
    bool cont;

    debug(POT_DEBUG, "servePotEvent(%d)\n", id);

    switch (id) {

        case POT_DISCHARGE:

            agnus->slot[POT_SLOT].data--;
            if (agnus->slot[POT_SLOT].data) {

                // Schedule another DISCHARGE event
                potCntX0++;
                potCntY0++;
                potCntX1++;
                potCntY1++;
                agnus->scheduleInc<POT_SLOT>(DMA_CYCLES(HPOS_MAX), POT_DISCHARGE);

            } else {

                // Schedule first CHARGE event
                potCntX0 = 0;
                potCntY0 = 0;
                potCntX1 = 0;
                potCntY1 = 0;
                agnus->scheduleInc<POT_SLOT>(DMA_CYCLES(HPOS_MAX), POT_CHARGE);
            }
            break;

        case POT_CHARGE:

            // Increment pot counters if target value hasn't been reached yet
            cont = false;
            if (potCntX0 < amiga->controlPort1.getPotX()) { potCntX0++; cont = true; }
            if (potCntY0 < amiga->controlPort1.getPotY()) { potCntY0++; cont = true; }
            if (potCntX1 < amiga->controlPort2.getPotX()) { potCntX1++; cont = true; }
            if (potCntY1 < amiga->controlPort2.getPotY()) { potCntY1++; cont = true; }

            // Schedule next pot event if at least counter is still running
            if (cont) {
                agnus->scheduleInc<POT_SLOT>(DMA_CYCLES(HPOS_CNT), POT_CHARGE);
            } else {
                agnus->cancel<POT_SLOT>();
            }
            break;

        default:
            assert(false);
    }
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

void
Paula::debugSetINTENA(unsigned bit, bool value)
{
    assert(bit <= 14);

    debug("debugSetINTENA(%d, %d)\n", bit, value);

    amiga->suspend();
    setINTENA((value ? 0x8000 : 0) | (1 << bit));
    inspect();
    amiga->resume();
}

void
Paula::debugSetINTREQ(unsigned bit, bool value)
{
    assert(bit <= 14);

    debug("debugSetINTREQ(%d, %d)\n", bit, value);

    amiga->suspend();
    setINTREQ((value ? 0x8000 : 0) | (1 << bit));
    inspect();
    amiga->resume();
}

