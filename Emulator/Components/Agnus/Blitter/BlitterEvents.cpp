// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Blitter.h"
#include "Agnus.h"

namespace vamiga {

void
Blitter::serviceEvent()
{
    serviceEvent(agnus.id[SLOT_BLT]);
}

void
Blitter::serviceEvent(EventID id)
{
    switch (id) {

        case BLT_STRT1:

            // Initialize internal Blitter variables
            prepareBlit();

            // Postpone the operation if Blitter DMA is disabled
            if (!agnus.bltdma()) {
                agnus.rescheduleAbs<SLOT_BLT>(NEVER);
                break;
            }

            // Only proceed if the bus is free
            if (!agnus.busIsFree<BUS_BLITTER>()) {
                trace(BLTTIM_DEBUG, "BLT_STRT1: Blocked by %d\n", agnus.busOwner[agnus.pos.h]);
                break;
            }

            // Proceed to the next state
            agnus.scheduleRel<SLOT_BLT>(DMA_CYCLES(1), BLT_STRT2);
            break;

        case BLT_STRT2:

            // Only proceed if the bus is a free
            if (!agnus.busIsFree<BUS_BLITTER>()) {
                trace(BLTTIM_DEBUG, "BLT_STRT2: Blocked by %d\n", agnus.busOwner[agnus.pos.h]);
                break;
            }

            // Proceed to the next state
            beginBlit();
            break;

        case BLT_COPY_SLOW:

            trace(BLT_DEBUG, "Copy instruction %d:%d\n", bltconUSE(), bltpc);
            (this->*copyBlitInstr[bltconUSE()][0][bltconFE()][bltpc])();
            break;

        case BLT_COPY_FAKE:

            trace(BLT_DEBUG, "Copy fake %d:%d\n", bltconUSE(), bltpc);
            (this->*copyBlitInstr[bltconUSE()][1][bltconFE()][bltpc])();
            break;

        case BLT_LINE_SLOW:
            
            trace(BLT_DEBUG, "Line instruction %d:%d\n", bltconUSEB(), bltpc);
            (this->*lineBlitInstr[bltconUSEBC()][0][bltpc])();
            break;

        case BLT_LINE_FAKE:
            
            trace(BLT_DEBUG, "Line fake %d:%d\n", bltconUSEB(), bltpc);
            (this->*lineBlitInstr[bltconUSEBC()][1][bltpc])();
            break;

        default:
            fatalError;
    }
}

}
