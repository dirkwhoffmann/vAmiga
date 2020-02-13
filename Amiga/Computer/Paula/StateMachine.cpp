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

    audioUnit.executeUntil(agnus.clock);
    audlenLatch = value;
}

template <int nr> void
StateMachine<nr>::pokeAUDxPER(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dPER(%X)\n", nr, value);

    audioUnit.executeUntil(agnus.clock);
    audperLatch = value;
}

template <int nr> void
StateMachine<nr>::pokeAUDxVOL(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dVOL(%X)\n", nr, value);

    /* Behaviour: 1. Only the lowest 7 bits are evaluated.
     *            2. All values greater than 64 are treated as 64 (max volume).
     */
    audioUnit.executeUntil(agnus.clock);
    audvolLatch = MIN(value & 0x7F, 64);
}

template <int nr> void
StateMachine<nr>::pokeAUDxDAT(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dDAT(%X)\n", nr, value);


    // IRQ mode
    if (!AUDxON()) {

        switch(state) {

            case 0b000:

                if (!AUDxIP()) move_000_010();
                break;
        }
    }

    // DMA mode
    else {

        switch(state) {

            case 0b000:

                move_000_001();
                break;

            case 0b001:

                move_001_101();
                break;

            case 0b101:

                move_101_000();
                break;

            case 0b010:
            case 0b011:

                // TODO: lenctrld if lenfin
                // TODO: lencount if !lenfin
                // TODO: intreq2 if lenfin
                break;
        }
    }

    // OLD CODE (DEPRECATED)

    audioUnit.executeUntil(agnus.clock);
    auddatLatch = value;

    /* "In interrupt-driven operation, transfer to the main loop (states 010
     *  and 011) occurs immediately after data is written by the processor."
     * [HRM]
     */
    if (!AUDxON() && !AUDxIP()) {

        audvol = audvolLatch;
        audper += audperLatch;
        AUDxIR();
    }
}

template <int nr> void
StateMachine<nr>::pokeAUDxLCH(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dLCH(%X)\n", nr, value);

    audioUnit.executeUntil(agnus.clock);
    audlcLatch = CHIP_PTR(REPLACE_HI_WORD(audlcLatch, value));
}

template <int nr> void
StateMachine<nr>::pokeAUDxLCL(uint16_t value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dLCL(%X)\n", nr, value);

    audioUnit.executeUntil(agnus.clock);
    audlcLatch = REPLACE_LO_WORD(audlcLatch, value);
}

template <int nr> void
StateMachine<nr>::pushSample(int16_t data, Cycle clock)
{
    if (sampleData[0] != data) {

        sampleData[2] = sampleData[1];
        sampleData[1] = sampleData[0];
        sampleData[0] = data;

        sampleTime[2] = sampleTime[1];
        sampleTime[1] = sampleTime[0];
        sampleTime[0] = clock;

        // if (nr == 0) debug("Pushing sample %d (%d)\n", data, clock);
    }
}

template <int nr> int16_t
StateMachine<nr>::pickSample(Cycle clock)
{
    if (clock > sampleTime[0]) return sampleData[0];
    if (clock > sampleTime[1]) return sampleData[1];
    if (clock < sampleTime[2]) {
        debug("Pipeline: (clock = %d agnusClock = %d)\n", clock, agnus.clock);
        for (int i = 0; i < 3; i++) {
            printf("Data: %02X Time: %lld\n", (uint8_t)sampleData[i], sampleTime[i]);
        }
        printf("\n");
    }
    assert(clock >= sampleTime[2]);
    return sampleData[2];
}

template <int nr> void
StateMachine<nr>::move_000_010() {

    /*
    audvol = audvolLatch; // volcntrld

    // TODO // pbufld1
    // audxIR();  // AUDxIR
    // scheduleEvent(...)  // percntrld

    penhi();
    state = 0b010;
    */
}

template <int nr> void
StateMachine<nr>::move_000_001() {

    /*
    state = 0b001;
    */
}

template <int nr> void
StateMachine<nr>::move_001_000() {

    /*
    state = 0b000;
    */
}

template <int nr> void
StateMachine<nr>::move_001_101() {

    /*
    state = 0b101;
    */
}

template <int nr> void
StateMachine<nr>::move_101_000() {

    /*
    state = 0b000;
    */
}

template <int nr> void
StateMachine<nr>::move_101_010() {

    /*
    penhi();
    state = 0b010;
    */
}

template <int nr> void
StateMachine<nr>::move_010_011() {

    /*
    penlo();
    state = 0b011;
    */
}

template <int nr> void
StateMachine<nr>::move_011_000() {

    /*
    state = 0b000;
    */
}

template <int nr> void
StateMachine<nr>::move_011_010() {

    /*
    state = 0b010;
    */
}

template <int nr> void
StateMachine<nr>::serviceEvent()
{
    assert(agnus.slot[CH0_SLOT+nr].id == CHX_PERFIN);
    debug("serviceEvent");

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

template <int nr> bool
StateMachine<nr>::AUDxIP()
{
    return GET_BIT(paula.intreq, 7 + nr);
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
    bufferOut = (int16_t)HI_BYTE(buffer) * audvol;
    pushSample(bufferOut, agnus.clock);
}

template <int nr> void
StateMachine<nr>::penlo()
{
    bufferOut = (int16_t)LO_BYTE(buffer) * audvol;
    pushSample(bufferOut, agnus.clock);
}

template <int nr> void
StateMachine<nr>::outputHi(uint16_t sample)
{
    uint16_t modulate = (paula.adkcon >> nr) & 0x11;

    // Standard case: Output sound sample
    if (nr == 3 || modulate == 0) {
        auddat = HI_BYTE(sample);
        pushSample((int16_t)HI_BYTE(sample) * audvolLatch, clock);
        return;
    }

    auddat = 0;
}

template <int nr> void
StateMachine<nr>::outputLo(uint16_t sample)
{
    uint16_t modulate = (paula.adkcon >> nr) & 0x11;
    // modulate = 0;

    // Standard case: Output sound sample
    if (nr == 3 || modulate == 0) {
        auddat = LO_BYTE(sample);
        pushSample((int16_t)LO_BYTE(sample) * audvolLatch, clock);
        return;
    }

    // Special case: Use sound sample to modulate another channel
    switch ((paula.adkcon >> nr) & 0x11) {

        case 0x01: // Volume modulation

            audioUnit.pokeAUDxVOL(nr + 1, auddatLatch);
            break;
        case 0x10: // Period modulation

            audioUnit.pokeAUDxPER(nr + 1, auddatLatch);
            break;

        case 0x11: // Volumne + period modulation

            if (samplecnt++ & 1) {
                audioUnit.pokeAUDxPER(nr + 1, auddatLatch);
            } else {
                audioUnit.pokeAUDxVOL(nr + 1, auddatLatch);
            }
            break;
    }

    auddat = 0;
}

template <int nr> int16_t
StateMachine<nr>::execute(DMACycle cycles)
{
    Cycle endClock = clock + DMA_CYCLES(cycles);

    switch(state) {

        case 0b000: // State 0 (Idle)

            audlen = audlenLatch;
            agnus.audlc[nr] = audlcLatch;
            audper = 0;
            state = 0b001;
            break;

        case 0b001: // State 1

            if (audlen > 1) audlen--;

            // Trigger Audio interrupt
            AUDxIR();

            state = 0b101;
            break;

        case 0b010: // State 2

            audper -= cycles;

            if (audper < 0) {

                audper += audperLatch;
                audvol = audvolLatch;

                // Put out the high byte
                // auddat = HI_BYTE(auddatLatch);
                // clock = midClock; // FAKE
                outputHi(auddatLatch);

                // Switch forth to state 3
                state = 0b011;
            }
            break;

        case 0b011: // State 3

            // Decrease the period counter
            audper -= cycles;

            // Only continue if the period counter did underflow
            if (audper > 1) break;

            // Reload the period counter
            audper += audperLatch;

            // ??? Can't find this in the state machine (from WinFellow?)
            audvol = audvolLatch;

            // Put out the low byte
            // auddat = LO_BYTE(auddatLatch);
            // clock = endClock; // FAKE
            outputLo(auddatLatch);

            // Switch to next state
            state = 0b010;

            // Read the next two samples from memory
            // auddatLatch = agnus.doAudioDMA<nr>();

            // Perform DMA mode specific action
            if (AUDxON()) {

                // Decrease the length counter
                if (audlen > 1) {
                    audlen--;
                } else {
                    audlen = audlenLatch;
                    agnus.audlc[nr] = audlcLatch;

                    // Trigger Audio interrupt
                    AUDxIR();
                }

                // Read the next two samples from memory
                auddatLatch = agnus.doAudioDMA<nr>();

            // Perform non-DMA mode specific action
            } else {

                // Go idle if the audio IRQ hasn't been acknowledged
                if (AUDxIP()) state = 0b000;

                // Trigger Audio interrupt
                AUDxIR();
            }

            break;

        case 0b101: // State 5

            audvol = audvolLatch;

            // (2)
            audper = 0;

            // Read the first two samples from memory
            auddatLatch = agnus.doAudioDMA<nr>();

            if (audlen > 1) {
                audlen--;
            } else {
                audlen = audlenLatch;
                agnus.audlc[nr] = audlcLatch;

                // Trigger Audio interrupt
                AUDxIR();
            }

            // Transition to state 2
            state = 0b010;
            break;

        default:
            assert(false);
            break;

    }

    clock = endClock;
    return (int8_t)auddat * audvolLatch;
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

template int16_t StateMachine<0>::execute(DMACycle cycles);
template int16_t StateMachine<1>::execute(DMACycle cycles);
template int16_t StateMachine<2>::execute(DMACycle cycles);
template int16_t StateMachine<3>::execute(DMACycle cycles);

template int16_t StateMachine<0>::pickSample(Cycle clock);
template int16_t StateMachine<1>::pickSample(Cycle clock);
template int16_t StateMachine<2>::pickSample(Cycle clock);
template int16_t StateMachine<3>::pickSample(Cycle clock);

template void StateMachine<0>::serviceEvent();
template void StateMachine<1>::serviceEvent();
template void StateMachine<2>::serviceEvent();
template void StateMachine<3>::serviceEvent();
