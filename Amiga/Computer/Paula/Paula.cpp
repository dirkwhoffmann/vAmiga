// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Paula::Paula(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("Paula");

    subComponents = vector<HardwareComponent *> {
        
        &audioUnit,
        &diskController,
        &uart
    };
}

void
Paula::_reset()
{
    RESET_SNAPSHOT_ITEMS

    for (int i = 0; i < 16; i++) setIntreq[i] = NEVER;
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
Paula::_warpOn()
{
    /* Warping has the unavoidable drawback that audio playback gets out of
     * sync. To cope with this issue, we ramp down the volume when warping is
     * switched on and fade in smoothly when it is switched off.
     */
    audioUnit.rampDown();
}

void
Paula::_warpOff()
{
    audioUnit.rampUp();
    audioUnit.alignWritePtr();
}

uint16_t
Paula::peekINTREQR()
{
    uint16_t result = intreq;

    // if (ciaa.irqPin() == 0) SET_BIT(result, 3);
    // if (ciab.irqPin() == 0) SET_BIT(result, 13);

    debug(INTREG_DEBUG, "peekINTREQR(): %x\n", result);

    return result;
}

void
Paula::pokeINTREQ(uint16_t value)
{
    debug(INTREG_DEBUG, "pokeINTREQ(%X)\n", value);

    // agnus.recordRegisterChange(DMA_CYCLES(2), REG_INTREQ, value);
    paula.setINTREQ(value);
}

void
Paula::pokeINTENA(uint16_t value)
{
    debug(INTREG_DEBUG, "pokeINTENA(%X)\n", value);

    // agnus.recordRegisterChange(DMA_CYCLES(2), REG_INTENA, value);
    paula.setINTENA(value);
}

void
Paula::pokeADKCON(uint16_t value)
{
    plaindebug(AUDREG_DEBUG, "pokeADKCON(%X)\n", value);

    // uint16_t oldAdkcon = adkcon;

    if (value & 0x8000) adkcon |= (value & 0x7FFF); else adkcon &= ~value;

    // Take care of a possible change of the UARTBRK bit
    uart.updateTXD();

    if (adkcon & 0b1110111) debug(AUDREG_DEBUG, "ADKCON MODULATION: %x\n", adkcon);
}

void
Paula::setINTREQ(bool setclr, uint16_t value)
{
    assert(!(value & 0x8000));

    debug(INTREG_DEBUG, "setINTREQ(%d,%x)\n", setclr, value);

    if (setclr) {
        intreq |= value;
    } else {
        intreq &= ~value;
    }

    if (ciaa.irqPin() == 0) SET_BIT(intreq, 3);
    if (ciab.irqPin() == 0) SET_BIT(intreq, 13);

    checkInterrupt();
}

void
Paula::setINTENA(bool setclr, uint16_t value)
{
    assert(!(value & 0x8000));

    debug(INTREG_DEBUG, "setINTENA(%d,%x)\n", setclr, value);

    if (setclr) intena |= value; else intena &= ~value;
    checkInterrupt();
}

void
Paula::raiseIrq(IrqSource src)
{
    setINTREQ(true, 1 << src);
}

void
Paula::scheduleIrqAbs(IrqSource src, Cycle trigger)
{
    assert(isIrqSource(src));
    assert(trigger != 0);
    assert(agnus.slot[IRQ_SLOT].id == IRQ_CHECK);

    debug(INT_DEBUG, "scheduleIrq(%d, %d)\n", src, trigger);

    // Record the request
    if (trigger < setIntreq[src])
        setIntreq[src] = trigger;

    // Service the request with the proper delay
    if (trigger < agnus.slot[IRQ_SLOT].triggerCycle) {
        agnus.scheduleAbs<IRQ_SLOT>(trigger, IRQ_CHECK);
    }
}

void
Paula::scheduleIrqRel(IrqSource src, Cycle trigger)
{
    assert(trigger != 0);
    scheduleIrqAbs(src, agnus.clock + trigger);
}

void
Paula::serviceIrqEvent()
{
    assert(agnus.slot[IRQ_SLOT].id == IRQ_CHECK);

    Cycle clock = agnus.clock;
    Cycle next = NEVER;

    // Check all interrupt sources
    for (int src = 0; src < 16; src++) {

        // Check if the interrupt source is due
        if (clock >= setIntreq[src]) {
            setINTREQ(true, 1 << src);
            setIntreq[src] = NEVER;
        } else {
             next = MIN(next, setIntreq[src]);
        }
    }

    // Schedule next event
    agnus.scheduleAbs<IRQ_SLOT>(next, IRQ_CHECK);
}

void
Paula::serviceIplEvent()
{
    assert(agnus.slot[IPL_SLOT].id == IPL_CHANGE);

    cpu.setIPL(agnus.slot[IPL_SLOT].data);
    agnus.cancel<IPL_SLOT>();
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

    bool poty0 = controlPort1.getPotY() & 0x80;
    bool potx0 = controlPort1.getPotX() & 0x80;
    bool poty1 = controlPort2.getPotY() & 0x80;
    bool potx1 = controlPort2.getPotX() & 0x80;

    WRITE_BIT(result, 14, outry ? datry : poty0);
    WRITE_BIT(result, 12, outrx ? datrx : potx0);
    WRITE_BIT(result, 10, outly ? datly : poty1);
    WRITE_BIT(result,  8, outlx ? datlx : potx1);

    // Connected devices can pull down lines even if they are configured
    // as outputs
    result &= controlPort1.potgor();
    result &= controlPort2.potgor();

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
        agnus.schedulePos<POT_SLOT>(agnus.pos.v, HPOS_MAX, POT_DISCHARGE);
        agnus.slot[POT_SLOT].data = 8;
    }
}

void
Paula::servicePotEvent(EventID id)
{
    debug(POT_DEBUG, "servicePotEvent(%d)\n", id);

    switch (id) {

        case POT_DISCHARGE:
        {
            agnus.slot[POT_SLOT].data--;
            if (agnus.slot[POT_SLOT].data) {

                // Schedule another discharge event
                potCntX0++;
                potCntY0++;
                potCntX1++;
                potCntY1++;
                agnus.scheduleRel<POT_SLOT>(DMA_CYCLES(HPOS_CNT), POT_DISCHARGE);

            } else {

                // Schedule first charge event
                potCntX0 = 0;
                potCntY0 = 0;
                potCntX1 = 0;
                potCntY1 = 0;
                agnus.scheduleRel<POT_SLOT>(DMA_CYCLES(HPOS_CNT), POT_CHARGE);
            }
            break;
        }
        case POT_CHARGE:
        {
            // Increment pot counters if target value hasn't been reached yet
            bool cont = false;
            debug("POT_CHARGE %d %d\n", controlPort1.getPotX(), controlPort1.getPotY());
            if (potCntX0 < controlPort1.getPotX()) { potCntX0++; cont = true; }
            if (potCntY0 < controlPort1.getPotY()) { potCntY0++; cont = true; }
            if (potCntX1 < controlPort2.getPotX()) { potCntX1++; cont = true; }
            if (potCntY1 < controlPort2.getPotY()) { potCntY1++; cont = true; }

            // Schedule next pot event if at least counter is still running
            if (cont) {
                agnus.scheduleRel<POT_SLOT>(DMA_CYCLES(HPOS_CNT), POT_CHARGE);
            } else {
                agnus.cancel<POT_SLOT>();
            }
            break;
        }
        default:
            assert(false);
    }
}

int
Paula::interruptLevel()
{
    if (intena & 0x4000) {

        uint16_t mask = intreq;

        // if (ciaa.irqPin() == 0) SET_BIT(mask, 3);
        // if (ciab.irqPin() == 0) SET_BIT(mask, 13);

        mask &= intena;

        // debug("INT: %d intena: %x intreq: %x mask: %x\n", ciaa.irqPin(), intena, intreq, mask);

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
    agnus.scheduleRel<IPL_SLOT>(DMA_CYCLES(4), IPL_CHANGE, interruptLevel());
}
