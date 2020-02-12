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

    audioUnit.executeUntil(agnus.clock);
    auddatLatch = value;

    /* "In interrupt-driven operation, transfer to the main loop (states 010
     *  and 011) occurs immediately after data is written by the processor."
     * [HRM]
     */
    if (!dmaMode() && !irqIsPending()) {

        audvol = audvolLatch;
        audper += audperLatch;
        triggerIrq();
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

template <int nr> bool
StateMachine<nr>::dmaMode()
{
    return agnus.doAudDMA<nr>();
    // return audioUnit.dmaEnabled & (1 << nr);
}

template <int nr> void
StateMachine<nr>::triggerIrq()
{
    paula.raiseIrq(nr == 0 ? INT_AUD0 :
                   nr == 1 ? INT_AUD1 :
                   nr == 2 ? INT_AUD2 : INT_AUD3);
}

template <int nr> bool
StateMachine<nr>::irqIsPending()
{
    return GET_BIT(paula.intreq, 7 + nr);
}

template <int nr> void
StateMachine<nr>::outputHi(uint16_t sample)
{
    uint16_t modulate = (paula.adkcon >> nr) & 0x11;
    // modulate = 0;

    // Standard case: Output sound sample
    if (nr == 3 || modulate == 0) {
        auddat = HI_BYTE(sample);
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
            triggerIrq();

            state = 0b101;
            break;

        case 0b010: // State 2

            audper -= cycles;

            if (audper < 0) {

                audper += audperLatch;
                audvol = audvolLatch;

                // Put out the high byte
                // auddat = HI_BYTE(auddatLatch);
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
            outputLo(auddatLatch);

            // Switch to next state
            state = 0b010;

            // Read the next two samples from memory
            // auddatLatch = agnus.doAudioDMA<nr>();

            // Perform DMA mode specific action
            if (dmaMode()) {

                // Decrease the length counter
                if (audlen > 1) {
                    audlen--;
                } else {
                    audlen = audlenLatch;
                    agnus.audlc[nr] = audlcLatch;

                    // Trigger Audio interrupt
                    triggerIrq();
                }

                // Read the next two samples from memory
                auddatLatch = agnus.doAudioDMA<nr>();

            // Perform non-DMA mode specific action
            } else {

                // Go idle if the audio IRQ hasn't been acknowledged
                if (irqIsPending()) state = 0b000;

                // Trigger Audio interrupt
                triggerIrq();
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
                triggerIrq();
            }

            // Transition to state 2
            state = 0b010;
            break;

        default:
            assert(false);
            break;

    }

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
