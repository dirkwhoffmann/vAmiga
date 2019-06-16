// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

StateMachine::StateMachine()
{
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {

        { &state,        sizeof(state),        0 },

        { &audlenLatch,  sizeof(audlenLatch),  0 },
        { &audlen,       sizeof(audlen),       0 },
        { &audperLatch,  sizeof(audperLatch),  0 },
        { &audper,       sizeof(audper),       0 },
        { &audvolLatch,  sizeof(audvolLatch),  0 },
        { &audvol,       sizeof(audvol),       0 },
        { &auddatLatch,  sizeof(auddatLatch),  0 },
        { &auddat,       sizeof(auddat),       0 },
        { &audlcLatch,   sizeof(audlcLatch),   0 },
    });
}

void
StateMachine::setNr(uint8_t nr)
{
    assert(nr < 4);
    this->nr = nr;

    switch (nr) {

        case 0: setDescription("StateMachine 0"); return;
        case 1: setDescription("StateMachine 1"); return;
        case 2: setDescription("StateMachine 2"); return;
        case 3: setDescription("StateMachine 3"); return;

        default: assert(false);
    }
}

void
StateMachine::_initialize()
{
    agnus = &amiga->agnus;
    paula = &amiga->paula;
}

void
StateMachine::_inspect()
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

AudioChannelInfo
StateMachine::getInfo()
{
    AudioChannelInfo result;

    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);

    return result;
}

int16_t
StateMachine::execute(DMACycle cycles)
{
    switch(state) {

        case 0b000: // State 0 (Idle)

            audlen = audlenLatch;
            agnus->audlc[nr] = audlcLatch;
            audper = 0;
            state = 0b001;
            break;

        case 0b001: // State 1

            if (audlen > 1) audlen--;

            // Trigger Audio interrupt
            paula->pokeINTREQ(0x8000 | (0x80 << nr));

            state = 0b101;
            break;

        case 0b010: // State 2

            audper -= cycles;

            if (audper < 0) {

                audper += audperLatch;
                audvol = audvolLatch;

                // Put out the high byte
                auddat = HI_BYTE(auddatLatch);

                // Switch forth to state 3
                state = 0b011;
            }
            break;

        case 0b011: // State 3

            /*    Always: (1) percount
             *
             * Condition: perfin && (AUDxON || !AUDxIP)
             *
             *       Actions: (2) pbufld1
             *                (3) AUDxDR if (AUDxON && napnav)
             *                (4) percntrld
             *                (5) Transition to 010
             *
             * Condition: lenfin && AUDxON && AUDxDAT
             *
             *       Actions: (6) lencntrld
             *                (7) intreq2
             *
             * Condition: !lenfin && AUDxON && AUDxDAT
             *
             *       Actions: (8) lencount
             *
             * Condition: perfin && !AUDxON && AUDxIP
             *
             *       Actions: (9) Transition to 000
             */

            // audper -= cycles;
            percount(cycles); // (1)

            // if (audper < 0) {
            if (perfin()) {

                // audper += audperLatch;
                percntrld(); // (4)

                audvol = audvolLatch; // ????

                // Put out the low byte
                auddat = LO_BYTE(auddatLatch);

                // Read the next two samples from memory
                auddatLatch = agnus->doAudioDMA(nr);

                if (lenfin()) {

                    lencntrld(); // (6)
                    agnus->audlc[nr] = audlcLatch; // ???
                    paula->pokeINTREQ(0x8000 | (0x80 << nr)); // (7)

                } else {

                    lencount(); // (8)
                }

                /*
                // Decrease the length counter
                if (audlen > 1) {
                    audlen--;
                } else {
                    audlen = audlenLatch;
                    agnus->audlc[nr] = audlcLatch;

                    // Trigger Audio interrupt
                    paula->pokeINTREQ(0x8000 | (0x80 << nr));
                }
                */

                // (5)
                state = 0b010;

            }

            /* "As long as the interrupt is cleared by the processor in time,
             *  the machine remains in the main loop. Otherwise, it enters the
             *  idle state. Interrupts are generated on every word transition
             *  (011-010).
             */
            // MISSING: Transition back to 000
            break;

        case 0b101: // State 5

            /* Condition: AUDxON && AUDxDAT
             *
             *   Actions: (1) volcntrld
             *            (2) percntrld
             *            (3) pbufldl
             *            (4) AUDxDR if napna
             *            (5) Transition to 010
             *
             * Condition: !AUDxON
             *
             *   Actions: (6) Transition to 000
             */

            volcntrld();

            // (2)
            audper = 0; // ???? SHOULD BE: percntrld();

            // Read the next two samples from memory
            auddatLatch = agnus->doAudioDMA(nr);

            if (audlen > 1) {
                audlen--;
            } else {
                audlen = audlenLatch;
                agnus->audlc[nr] = audlcLatch;

                // Trigger Audio interrupt
                paula->pokeINTREQ(0x8000 | (0x80 << nr));
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
