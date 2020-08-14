// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
DiskController::serviceDiskEvent()
{
    if (asyncFifo) {
        
        // Receive next byte from the selected drive
        executeFifo();
    
        // Schedule next event
        agnus.scheduleRel<DSK_SLOT>(DMA_CYCLES(57), DSK_ROTATE);
    }
}

void
DiskController::serviceDiskChangeEvent()
{
    if (agnus.slot[DCH_SLOT].id == EVENT_NONE) return;
    
    int n = (int)agnus.slot[DCH_SLOT].data;
    assert(n >= 0 && n <= 3);

    switch (agnus.slot[DCH_SLOT].id) {

        case DCH_INSERT:

            debug(DSK_DEBUG, "DCH_INSERT (df%d)\n", n);

            assert(diskToInsert != NULL);
            df[n]->insertDisk(diskToInsert);
            diskToInsert = NULL;
            break;

        case DCH_EJECT:

            debug(DSK_DEBUG, "DCH_EJECT (df%d)\n", n);

            df[n]->ejectDisk();
            break;

        default:
            assert(false);
    }

    agnus.cancel<DCH_SLOT>();
}
