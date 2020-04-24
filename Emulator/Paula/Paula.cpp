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

u16
Paula::peekINTREQR()
{
    u16 result = intreq;

    // if (ciaa.irqPin() == 0) SET_BIT(result, 3);
    // if (ciab.irqPin() == 0) SET_BIT(result, 13);

    debug(INTREG_DEBUG, "peekINTREQR(): %x\n", result);

    return result;
}

void
Paula::pokeINTREQ(u16 value)
{
    debug(INTREG_DEBUG, "pokeINTREQ(%X)\n", value);

    // agnus.recordRegisterChange(DMA_CYCLES(2), REG_INTREQ, value);
    paula.setINTREQ(value);
}

void
Paula::pokeINTENA(u16 value)
{
    debug(INTREG_DEBUG, "pokeINTENA(%X)\n", value);

    // agnus.recordRegisterChange(DMA_CYCLES(2), REG_INTENA, value);
    paula.setINTENA(value);
}

void
Paula::pokeADKCON(u16 value)
{
    plaindebug(AUDREG_DEBUG, "pokeADKCON(%X)\n", value);

    // u16 oldAdkcon = adkcon;

    if (value & 0x8000) adkcon |= (value & 0x7FFF); else adkcon &= ~value;

    // Take care of a possible change of the UARTBRK bit
    uart.updateTXD();

    if (adkcon & 0b1110111) debug(AUDREG_DEBUG, "ADKCON MODULATION: %x\n", adkcon);
}

void
Paula::setINTREQ(bool setclr, u16 value)
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
Paula::setINTENA(bool setclr, u16 value)
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
    
    cpu.setIPL((iplPipe >> 32) & 0xFF);
    iplPipe = (iplPipe << 8) | (iplPipe & 0xFF);
    
    // Reschedule event until the pipe has been shifted through entirely
    i64 repeat = agnus.slot[IPL_SLOT].data;
    if (repeat) {
        agnus.scheduleRel<IPL_SLOT>(DMA_CYCLES(1), IPL_CHANGE, repeat - 1);
    } else {
        agnus.cancel<IPL_SLOT>();
    }
}

template <int x> u16
Paula::peekPOTxDAT()
{
    assert(x == 0 || x == 1);

    u16 result = x ? HI_LO(potCntY1, potCntX1) : HI_LO(potCntY0, potCntX0);
    debug(POT_DEBUG, "peekPOT%dDAT() = %X\n", x, result);

    return result;
}

u16
Paula::peekPOTGOR()
{
    u16 result = 0;

    REPLACE_BIT(result, 14, chargeY1 >= 1.0);
    REPLACE_BIT(result, 12, chargeX1 >= 1.0);
    REPLACE_BIT(result, 10, chargeY0 >= 1.0);
    REPLACE_BIT(result,  8, chargeX0 >= 1.0);

    // A connected device may force the output level to a specific value
    controlPort1.changePotgo(result);
    controlPort2.changePotgo(result);

    debug(POT_DEBUG, "peekPOTGOR = %X (potgo = %x)\n", result, potgo);
    return result;
}

void
Paula::pokePOTGO(u16 value)
{
    debug(POT_DEBUG, "pokePOTGO(%X)\n", value);

    potgo = value;

    // Take care of bits that are configured as outputs
    if (GET_BIT(value, 9))  chargeX0 = GET_BIT(value, 8)  ? 1.0 : 0.0;
    if (GET_BIT(value, 11)) chargeY0 = GET_BIT(value, 10) ? 1.0 : 0.0;
    if (GET_BIT(value, 13)) chargeX1 = GET_BIT(value, 12) ? 1.0 : 0.0;
    if (GET_BIT(value, 15)) chargeY1 = GET_BIT(value, 14) ? 1.0 : 0.0;

    // Check the START bit
    if (GET_BIT(value, 0)) {

        debug(POT_DEBUG, "Starting potentiometer scan procedure\n");

        // Clear potentiometer counters
        potCntX0 = 0;
        potCntY0 = 0;
        potCntX1 = 0;
        potCntY1 = 0;

        // Schedule the first DISCHARGE event
        agnus.schedulePos<POT_SLOT>(agnus.pos.v, HPOS_MAX, POT_DISCHARGE, 8);
    }
}

void
Paula::servicePotEvent(EventID id)
{
    debug(POT_DEBUG, "servicePotEvent(%d)\n", id);

    switch (id) {

        case POT_DISCHARGE:
        {
            if (--agnus.slot[POT_SLOT].data) {

                // Discharge capacitors
                if (!OUTLY()) chargeY0 = 0.0;
                if (!OUTLX()) chargeX0 = 0.0;
                if (!OUTRY()) chargeY1 = 0.0;
                if (!OUTRX()) chargeX1 = 0.0;

                agnus.scheduleRel<POT_SLOT>(DMA_CYCLES(HPOS_CNT), POT_DISCHARGE);

            } else {

                // Reset counters
                // For input pins, we need to set the couter value to -1. It'll
                // wrap over to 0 in the hsync handler.
                potCntY0 = OUTLY() ? 0 : -1;
                potCntX0 = OUTLX() ? 0 : -1;
                potCntY1 = OUTRY() ? 0 : -1;
                potCntX1 = OUTRX() ? 0 : -1;

                // Schedule first charge event
                agnus.scheduleRel<POT_SLOT>(DMA_CYCLES(HPOS_CNT), POT_CHARGE);
            }
            break;
        }
        case POT_CHARGE:
        {
            bool cont = false;

            // Get delta charges for each line
            double dy0 = controlPort1.getChargeDY();
            double dx0 = controlPort1.getChargeDX();
            double dy1 = controlPort2.getChargeDY();
            double dx1 = controlPort2.getChargeDX();

            // Charge capacitors
            if (dy0 && chargeY0 < 1.0 && !OUTLY()) { chargeX0 += dy0; cont = true; }
            if (dx0 && chargeX0 < 1.0 && !OUTLX()) { chargeX0 += dx0; cont = true; }
            if (dy1 && chargeY1 < 1.0 && !OUTRY()) { chargeX0 += dy1; cont = true; }
            if (dx1 && chargeX1 < 1.0 && !OUTRX()) { chargeX0 += dx1; cont = true; }

            // Schedule next event
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

        u16 mask = intreq;

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
    int level = interruptLevel();
    
    if ((iplPipe & 0xFF) != level) {
        
        iplPipe = (iplPipe & ~0xFF) | level;
        agnus.scheduleRel<IPL_SLOT>(0, IPL_CHANGE, 5);
    }
}

template u16 Paula::peekPOTxDAT<0>();
template u16 Paula::peekPOTxDAT<1>();


