// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Paula.h"
#include "CIA.h"
#include "ControlPort.h"

namespace vamiga {

u16
Paula::peekADKCONR() const
{
    debug(AUDREG_DEBUG || DSKREG_DEBUG, "peekADKCON() = %x\n", adkcon);
    
    return adkcon;
}

void
Paula::pokeADKCON(u16 value)
{
    debug(AUDREG_DEBUG || DSKREG_DEBUG, "pokeADKCON(%x)\n", value);

    bool set = value & 0x8000;
    bool clr = !set;
    
    // Report unusual values
    if (set && (GET_BIT(value, 13) || GET_BIT(value, 14))) {
        xfiles("ADKCON: PRECOMP set (%x)\n", value);
    }
    if (clr && GET_BIT(value, 12)) {
        xfiles("ADKCON: MFMPREC cleared (GCR) (%x)\n", value);
    }
    if (set && GET_BIT(value, 9)) {
        xfiles("ADKCON: MSBSYNC set (GCR) (%x)\n", value);
    }
    if (clr && GET_BIT(value, 8)) {
        xfiles("ADKCON: FAST cleared (GCR) (%x)\n", value);
    }

    if (set) adkcon |= (value & 0x7FFF); else adkcon &= ~value;

    // Take care of a possible change of the UARTBRK bit
    uart.updateTXD();

    if (adkcon & 0b1110111) {
        trace(AUDREG_DEBUG, "ADKCON MODULATION: %x\n", adkcon);
    }
}

u16
Paula::peekINTREQR() const
{
    trace(INTREG_DEBUG, "peekINTREQR(): %x (INTENA = %x)\n", intreq, intena);

    return intreq;
}

template <Accessor s> void
Paula::pokeINTREQ(u16 value)
{
    trace(INTREG_DEBUG, "pokeINTREQ(%x) (INTENA = %x INTREQ = %x)\n", value, intena, intreq);

    agnus.recordRegisterChange(DMA_CYCLES(1), SET_INTREQ, value);
}

void
Paula::setINTREQ(bool setclr, u16 value)
{
    assert(!(value & 0x8000));

    trace(INTREG_DEBUG, "setINTREQ(%d,%x)\n", setclr, value);

    if (setclr) {
        intreq |= value;
    } else {
        intreq &= ~value;
    }

    if (ciaa.getIrq() == 0) SET_BIT(intreq, 3);
    if (ciab.getIrq() == 0) SET_BIT(intreq, 13);

    checkInterrupt();
}

u16
Paula::peekINTENAR() const
{
    trace(INTREG_DEBUG, "peekINTENAR(): %x (INTREQ = %x)\n", intena, intreq);

    return intena;
}

template <Accessor s> void
Paula::pokeINTENA(u16 value)
{
    trace(INTREG_DEBUG, "pokeINTENA(%x)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), SET_INTENA, value);
}

void
Paula::setINTENA(bool setclr, u16 value)
{
    assert(!(value & 0x8000));

    trace(INTREG_DEBUG, "setINTENA(%d,%x)\n", setclr, value);

    if (setclr) intena |= value; else intena &= ~value;
    checkInterrupt();
}

template <isize x> u16
Paula::peekPOTxDAT() const
{
    u16 result;
    
    if constexpr (x == 0) result = HI_LO(potCntY0, potCntX0);
    if constexpr (x == 1) result = HI_LO(potCntY1, potCntX1);

    trace(POTREG_DEBUG, "peekPOT%ldDAT() = %x\n", x, result);
    return result;
}

u16
Paula::peekPOTGOR() const
{
    u16 result = 0;

    REPLACE_BIT(result, 14, chargeY1 >= 1.0);
    REPLACE_BIT(result, 12, chargeX1 >= 1.0);
    REPLACE_BIT(result, 10, chargeY0 >= 1.0);
    REPLACE_BIT(result,  8, chargeX0 >= 1.0);

    trace(POT_DEBUG, "charges: %f %f %f %f\n", chargeY1, chargeX1, chargeY0, chargeX0);
    
    // A connected device may force the output level to a specific value
    controlPort1.changePotgo(result);
    controlPort2.changePotgo(result);

    trace(POTREG_DEBUG, "peekPOTGOR() = %x (potgo = %x)\n", result, potgo);
    return result;
}

void
Paula::pokePOTGO(u16 value)
{
    trace(POTREG_DEBUG, "pokePOTGO(%x)\n", value);

    potgo = value;

    // Take care of bits that are configured as outputs
    if (GET_BIT(value, 9))  chargeX0 = GET_BIT(value, 8)  ? 1.0 : 0.0;
    if (GET_BIT(value, 11)) chargeY0 = GET_BIT(value, 10) ? 1.0 : 0.0;
    if (GET_BIT(value, 13)) chargeX1 = GET_BIT(value, 12) ? 1.0 : 0.0;
    if (GET_BIT(value, 15)) chargeY1 = GET_BIT(value, 14) ? 1.0 : 0.0;

    // Check the START bit
    if (GET_BIT(value, 0)) {

        trace(POT_DEBUG, "Starting potentiometer scan procedure\n");

        // Clear potentiometer counters
        potCntX0 = 0;
        potCntY0 = 0;
        potCntX1 = 0;
        potCntY1 = 0;

        // Schedule the first DISCHARGE event
        agnus.schedulePos<SLOT_POT>(agnus.pos.v, HPOS_MAX_PAL, POT_DISCHARGE, 8);
    }
}

template void Paula::pokeINTREQ<ACCESSOR_CPU>(u16 value);
template void Paula::pokeINTREQ<ACCESSOR_AGNUS>(u16 value);
template void Paula::pokeINTENA<ACCESSOR_CPU>(u16 value);
template void Paula::pokeINTENA<ACCESSOR_AGNUS>(u16 value);
template u16 Paula::peekPOTxDAT<0>() const;
template u16 Paula::peekPOTxDAT<1>() const;

}
