// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

template <int nr>
StateMachine<nr>::StateMachine(Amiga& ref) : AmigaComponent(ref)
{
    // Set description
    switch (nr) {
        case 0: setDescription("StateMachine 0"); break;
        case 1: setDescription("StateMachine 1"); break;
        case 2: setDescription("StateMachine 2"); break;
        case 3: setDescription("StateMachine 3"); break;
        default: assert(false);
    }
}

template <int nr> void
StateMachine<nr>::_dump()
{
    printf("   State: %d\n", state);
    printf("  AUDxIP: %d\n", AUDxIP());
    printf("  AUDxON: %d\n", AUDxON());
}

template <int nr> void
StateMachine<nr>::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);

    info.state = state;
    info.audlenLatch = audlenLatch;
    info.audlen = audlen;
    info.audperLatch = audperLatch;
    info.audper = audper;
    info.audvolLatch = audvolLatch;
    info.audvol = audvol;
    info.auddatLatch = auddatLatch;
    info.auddat = auddat;
    info.audlcLatch = audlcLatch;

    pthread_mutex_unlock(&lock);
}

template <int nr> AudioChannelInfo
StateMachine<nr>::getInfo()
{
    AudioChannelInfo result;

    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);

    return result;
}

template <int nr> void
StateMachine<nr>::pokeAUDxLEN(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dLEN(%X)\n", nr, value);

    audlenLatch = value;
}

template <int nr> void
StateMachine<nr>::pokeAUDxPER(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dPER(%X)\n", nr, value);

    audperLatch = value;
}

template <int nr> void
StateMachine<nr>::pokeAUDxVOL(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dVOL(%X)\n", nr, value);

    // 1. Only the lowest 7 bits are evaluated.
    // 2. All values greater than 64 are treated as 64 (max volume).
    audvolLatch = MIN(value & 0x7F, 64);
}

template <int nr> void
StateMachine<nr>::pokeAUDxDAT(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dDAT(%X)\n", nr, value);

    auddatLatch = value;

    if (!AUDxON()) {

        // IRQ mode
        switch(state) {

            case 0b000:

                if (!AUDxIP()) move_000_010();
                break;
        }

    } else {

        // DMA mode
        switch(state) {

            case 0b000:

                move_000_001();
                break;

            case 0b001:

                move_001_101();
                break;

            case 0b101:

                move_101_010();
                break;

            case 0b010:
            case 0b011:

                if (!lenfin()) lencount();
                else {
                    lencntrld();
                    AUDxDSR();
                    intreq2 = true;
                }
                break;
        }
    }
}

template <int nr> void
StateMachine<nr>::pokeAUDxLCH(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dLCH(%X)\n", nr, value);

    audlcLatch = CHIP_PTR(REPLACE_HI_WORD(audlcLatch, value));
}

template <int nr> void
StateMachine<nr>::pokeAUDxLCL(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dLCL(%X)\n", nr, value);

    audlcLatch = REPLACE_LO_WORD(audlcLatch, value);
}

template <int nr> int16_t
StateMachine<nr>::pickSample(Cycle clock)
{
    int16_t result;

    int w  = samples.w;
    int r1 = samples.r;
    int r2 = samples.next(r1);

    if (r1 == w) {

        // Buffer is empty
        result = 0;

    } else if (r2 == w) {

        // Buffer contains a single element
        result = samples.elements[r1];

    } else {

        // Remove outdates samples
        while (r1 != samples.w && r2 != samples.w && samples.keys[r2] <= clock) {
            (void)samples.read();
            r1 = r2;
            r2 = samples.next(r1);
        }

        result = samples.elements[r1];
    }

    return result;
}

template <int nr> void
StateMachine<nr>::enableDMA()
{
    debug(AUD_DEBUG, "Disable DMA\n");

    switch (state) {

         case 0b000:

             move_000_001();
             break;
     }
}

template <int nr> void
StateMachine<nr>::disableDMA()
{
    debug(AUD_DEBUG, "Disable DMA\n");

    switch (state) {

        case 0b001:

            move_001_000();
            break;

        case 0b101:

            move_101_000();
            break;
    }
}

template <int nr> void
StateMachine<nr>::move_000_010() {

    debug(AUD_DEBUG, "move_000_010\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());
    assert(!AUDxIP());

    volcntrld();
    percntrld();
    pbufld1();
    AUDxIR();

    state = 0b010;
    penhi();
}

template <int nr> void
StateMachine<nr>::move_000_001() {

    debug(AUD_DEBUG, "move_000_001\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    lencntrld();
    // AUDxDSR();
    AUDxDR();

    state = 0b001;
}

template <int nr> void
StateMachine<nr>::move_001_000() {

    debug(AUD_DEBUG, "move_001_000\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());

    state = 0b000;
}

template <int nr> void
StateMachine<nr>::move_001_101() {

    debug(AUD_DEBUG, "move_001_101\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    AUDxIR();
    AUDxDR();
    AUDxDSR();
    if (!lenfin()) lencount();

    state = 0b101;
}

template <int nr> void
StateMachine<nr>::move_101_000() {

    debug(AUD_DEBUG, "move_101_000\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());

    state = 0b000;
}

template <int nr> void
StateMachine<nr>::move_101_010() {

    debug(AUD_DEBUG, "move_101_010\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    percntrld();
    volcntrld();
    pbufld1();
    if (napnav()) AUDxDR();

    state = 0b010;
    penhi();
}

template <int nr> void
StateMachine<nr>::move_010_011() {

    debug(AUD_DEBUG, "move_010_011\n");
    
    percntrld();
    
    // Check for attach period mode
    if (AUDxAP()) {
        
        pbufld2();
        
        if (AUDxON()) {
            
            // Additional DMA mode action
            AUDxDR();
            if (intreq2) { AUDxIR(); intreq2 = false; }
            
        } else {
            
            // Additional IRQ mode action
            AUDxIR();
        }
    }

    state = 0b011;
    penlo();
}

template <int nr> void
StateMachine<nr>::move_011_000() {

    debug(AUD_DEBUG, "move_011_000\n");

    const EventSlot slot = (EventSlot)(CH0_SLOT+nr);

    agnus.cancel<slot>();
    state = 0b000;
}

template <int nr> void
StateMachine<nr>::move_011_010()
{
    debug(AUD_DEBUG, "move_011_010\n");

    percntrld();
    pbufld1();
    volcntrld();
    
    if (napnav()) {

        if (AUDxON()) {

            // Additional DMA mode action
            AUDxDR();
            if (intreq2) { AUDxIR(); intreq2 = false; }

        } else {

            // Additional IRQ mode action
            AUDxIR();
        }
    }

    state = 0b010;
    penhi();
}

template <int nr> void
StateMachine<nr>::serviceEvent()
{
    const EventSlot slot = (EventSlot)(CH0_SLOT+nr);

    debug(AUD_DEBUG, "CHX_PERFIN state = %d\n", state);
    assert(agnus.slot[slot].id == CHX_PERFIN);

    switch (state) {

        case 0b010:

            move_010_011();
            return;

        case 0b011:

            (AUDxON() || !AUDxIP()) ? move_011_010() : move_011_000();
            return;

        default:
            assert(false);
    }
}

template <int nr> bool
StateMachine<nr>::AUDxON()
{
    return agnus.doAudDMA<nr>();
}

template <int nr> void
StateMachine<nr>::AUDxIR()
{
    paula.raiseIrq(nr == 0 ? INT_AUD0 :
                   nr == 1 ? INT_AUD1 :
                   nr == 2 ? INT_AUD2 : INT_AUD3);
}

template <int nr> void
StateMachine<nr>::percntrld()
{
    const EventSlot slot = (EventSlot)(CH0_SLOT+nr);

    if (audperLatch < 64) {
        // What shall we do with very small audper values?
        agnus.scheduleRel<slot>(DMA_CYCLES(64), CHX_PERFIN);
    } else {
        agnus.scheduleRel<slot>(DMA_CYCLES(audperLatch), CHX_PERFIN);
    }
}

template <int nr> bool
StateMachine<nr>::AUDxIP()
{
    return GET_BIT(paula.intreq, 7 + nr);
}

template <int nr> void
StateMachine<nr>::pbufld1()
{
    if (AUDxAV()) {
        // debug("Volume modulation %d (%d)\n", auddatLatch & 0x7F, (int16_t)auddatLatch);
        if (nr == 0) audioUnit.channel1.pokeAUDxVOL(auddatLatch);
        if (nr == 1) audioUnit.channel2.pokeAUDxVOL(auddatLatch);
        if (nr == 2) audioUnit.channel3.pokeAUDxVOL(auddatLatch);
    } else {
        buffer = auddatLatch;
    }
}

template <int nr> void
StateMachine<nr>::pbufld2()
{
    assert(AUDxAP());
    if (nr < 3) {
        // debug("Period modulation %d\n", auddatLatch);
        audioUnit.pokeAUDxPER(nr + 1, auddatLatch);
    }
}

template <int nr> bool
StateMachine<nr>::AUDxAV()
{
    return (paula.adkcon >> nr) & 0x01;
}

template <int nr> bool
StateMachine<nr>::AUDxAP()
{
    return (paula.adkcon >> nr) & 0x10;
}

template <int nr> void
StateMachine<nr>::penhi()
{
    int8_t sample = (int8_t)HI_BYTE(buffer);
    int16_t scaled = sample * audvol;

    debug(AUD_DEBUG, "penhi: %d %d\n", sample, scaled);

    samples.insert(agnus.clock, scaled);
}

template <int nr> void
StateMachine<nr>::penlo()
{
    int8_t sample = (int8_t)LO_BYTE(buffer);
    int16_t scaled = sample * audvol;

    debug(AUD_DEBUG, "penlo: %d %d\n", sample, scaled);

    samples.insert(agnus.clock, scaled);
}

template StateMachine<0>::StateMachine(Amiga &ref);
template StateMachine<1>::StateMachine(Amiga &ref);
template StateMachine<2>::StateMachine(Amiga &ref);
template StateMachine<3>::StateMachine(Amiga &ref);

template AudioChannelInfo StateMachine<0>::getInfo();
template AudioChannelInfo StateMachine<1>::getInfo();
template AudioChannelInfo StateMachine<2>::getInfo();
template AudioChannelInfo StateMachine<3>::getInfo();

template void StateMachine<0>::pokeAUDxLEN(uint16_t value);
template void StateMachine<1>::pokeAUDxLEN(uint16_t value);
template void StateMachine<2>::pokeAUDxLEN(uint16_t value);
template void StateMachine<3>::pokeAUDxLEN(uint16_t value);

template void StateMachine<0>::pokeAUDxPER(uint16_t value);
template void StateMachine<1>::pokeAUDxPER(uint16_t value);
template void StateMachine<2>::pokeAUDxPER(uint16_t value);
template void StateMachine<3>::pokeAUDxPER(uint16_t value);

template void StateMachine<0>::pokeAUDxVOL(uint16_t value);
template void StateMachine<1>::pokeAUDxVOL(uint16_t value);
template void StateMachine<2>::pokeAUDxVOL(uint16_t value);
template void StateMachine<3>::pokeAUDxVOL(uint16_t value);

template void StateMachine<0>::pokeAUDxDAT(uint16_t value);
template void StateMachine<1>::pokeAUDxDAT(uint16_t value);
template void StateMachine<2>::pokeAUDxDAT(uint16_t value);
template void StateMachine<3>::pokeAUDxDAT(uint16_t value);

template void StateMachine<0>::pokeAUDxLCH(uint16_t value);
template void StateMachine<1>::pokeAUDxLCH(uint16_t value);
template void StateMachine<2>::pokeAUDxLCH(uint16_t value);
template void StateMachine<3>::pokeAUDxLCH(uint16_t value);

template void StateMachine<0>::pokeAUDxLCL(uint16_t value);
template void StateMachine<1>::pokeAUDxLCL(uint16_t value);
template void StateMachine<2>::pokeAUDxLCL(uint16_t value);
template void StateMachine<3>::pokeAUDxLCL(uint16_t value);

template void StateMachine<0>::enableDMA();
template void StateMachine<1>::enableDMA();
template void StateMachine<2>::enableDMA();
template void StateMachine<3>::enableDMA();

template void StateMachine<0>::disableDMA();
template void StateMachine<1>::disableDMA();
template void StateMachine<2>::disableDMA();
template void StateMachine<3>::disableDMA();

template int16_t StateMachine<0>::pickSample(Cycle clock);
template int16_t StateMachine<1>::pickSample(Cycle clock);
template int16_t StateMachine<2>::pickSample(Cycle clock);
template int16_t StateMachine<3>::pickSample(Cycle clock);

template void StateMachine<0>::serviceEvent();
template void StateMachine<1>::serviceEvent();
template void StateMachine<2>::serviceEvent();
template void StateMachine<3>::serviceEvent();
