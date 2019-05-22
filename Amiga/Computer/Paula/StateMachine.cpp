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
StateMachine::_initialize() {

    agnus = &amiga->agnus;
    paula = &amiga->paula;
}

int16_t
StateMachine::execute(DMACycle cycles)
{
    switch(state) {

        case 0b000:

            audlen = audlenLatch;
            agnus->audlc[nr] = audlcLatch;
            audper = 0;
            state = 0b001;
            break;

        case 0b001:

            if (audlen > 1) audlen--;

            // Trigger Audio interrupt
            paula->pokeINTREQ(0x8000 | (0x80 << nr));

            state = 0b101;
            break;

        case 0b010:

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

        case 0b011:

            audper -= cycles;

            if (audper < 0) {

                audper += audperLatch;
                audvol = audvolLatch;

                // Put out the low byte
                auddat = LO_BYTE(auddatLatch);

                // Read the next two samples from memory
                auddatLatch = amiga->mem.peekChip16(agnus->audlc[nr]);
                INC_DMAPTR(agnus->audlc[nr]);

                // Decrease the length counter
                if (audlen > 1) {
                    audlen--;
                } else {
                    audlen = audlenLatch;
                    agnus->audlc[nr] = audlcLatch;

                    // Trigger Audio interrupt
                    paula->pokeINTREQ(0x8000 | (0x80 << nr));
                }

                // Switch back to state 2
                state = 0b010;

            }
            break;

        case 0b101:

            audvol = audvolLatch;
            audper = 0;

            // Read the next two samples from memory
            auddatLatch = amiga->mem.peekChip16(agnus->audlc[nr]);
            INC_DMAPTR(agnus->audlc[nr]);

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
