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

        { &state,           sizeof(state),           0 },

        { &audlen,          sizeof(audlen),          0 },
        { &audlenInternal,  sizeof(audlenInternal),  0 },
        { &audper,          sizeof(audper),          0 },
        { &audperInternal,  sizeof(audperInternal),  0 },
        { &audvol,          sizeof(audvol),          0 },
        { &audvolInternal,  sizeof(audvolInternal),  0 },
        { &auddat,          sizeof(auddat),          0 },
        { &auddatInternal,  sizeof(auddatInternal),  0 },
        { &audlcLatch,      sizeof(audlcLatch),      0 },
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
StateMachine::execute(DMACycle cycles)
{
    switch(state) {

        case 0b000:

            audlenInternal = audlen;
            _agnus->audlc[nr] = audlcLatch;
            audperInternal = 0;
            state = 0b001;
            break;

        case 0b001:

            if (audlenInternal > 1) audlenInternal--;

            // Trigger Audio interrupt
            _paula->pokeINTREQ(0x8000 | (0x80 << nr));

            state = 0b101;

            break;

        case 0b010:

            audperInternal -= cycles;

            if (audperInternal < 0) {

                audperInternal += audper;
                audvolInternal = audvol;

                // Put out the high byte
                auddatInternal = HI_BYTE(auddat);

                // Switch forth to state 3
                state = 0b011;
            }
            break;

        case 0b011:

            audperInternal -= cycles;

            if (audperInternal < 0) {

                audperInternal += audper;
                audvolInternal = audvol;

                // Put out the low byte
                auddatInternal = LO_BYTE(auddat);

                // Read the next two samples from memory
                auddat = _mem->peekChip16(_agnus->audlc[nr]);
                INC_DMAPTR(_agnus->audlc[nr]);

                // Decrease the length counter
                if (audlenInternal > 1) {
                    audlenInternal--;
                } else {
                    audlenInternal = audlen;
                    _agnus->audlc[nr] = audlcLatch;

                    // Trigger Audio interrupt
                    _paula->pokeINTREQ(0x8000 | (0x80 << nr));
                }

                // Switch back to state 2
                state = 0b010;

            }
            break;

        case 0b101:

            audvolInternal = audvol;
            audperInternal = 0;

            // Read the next two samples from memory
            auddat = _mem->peekChip16(_agnus->audlc[nr]);
            INC_DMAPTR(_agnus->audlc[nr]);

            if (audlenInternal > 1) {
                audlenInternal--;
            } else {
                audlenInternal = audlen;
                _agnus->audlc[nr] = audlcLatch;

                // Trigger Audio interrupt
                _paula->pokeINTREQ(0x8000 | (0x80 << nr));
            }

            // Transition to state 2
            state = 0b010;
            break;

        default:
            assert(false);
            break;

    }
}
