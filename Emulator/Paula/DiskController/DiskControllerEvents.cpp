// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DiskController.h"
#include "Agnus.h"
#include "Drive.h"
#include <cmath>

void
DiskController::serviceDiskEvent()
{        
    // Receive next byte from the selected drive
    executeFifo();
    
    // Schedule next event
    scheduleNextDiskEvent();
}

void
DiskController::scheduleFirstDiskEvent()
{
    dskEventDelay = 0.0;
    
    if (turboMode()) {
        scheduler.cancel<SLOT_DSK>();
    } else {
        scheduler.scheduleImm<SLOT_DSK>(DSK_ROTATE);
    }
}

void
DiskController::scheduleNextDiskEvent()
{
    /* Advance the delay counter to achieve a disk rotation speed of 300rpm.
     * Rotation speed can be measured with AmigaTestKit.adf which calculates
     * the delay between consecutive index pulses. 300rpm corresponds to a
     * index pulse delay of 200ms.
     */
    dskEventDelay += 55.98;
    DMACycle rounded = DMACycle(round(dskEventDelay));
    dskEventDelay -= rounded;
    
    if (turboMode()) {
        scheduler.cancel<SLOT_DSK>();
    } else {
        agnus.scheduleRel<SLOT_DSK>(DMA_CYCLES(rounded), DSK_ROTATE);
    }
}

void
DiskController::serviceDiskChangeEvent(isize n)
{
    assert(n >= 0 && n <= 3);

    EventSlot slot = SLOT_DC0 + n;
    
    if (scheduler.id[slot] == EVENT_NONE) return;
    
    switch (scheduler.id[slot]) {

        case DCH_INSERT:

            trace(DSK_DEBUG, "DCH_INSERT (df%zd)\n", n);

            assert(diskToInsert != nullptr);
            df[n]->insertDisk(std::move(diskToInsert));
            assert(diskToInsert == nullptr);
            break;

        case DCH_EJECT:

            trace(DSK_DEBUG, "DCH_EJECT (df%zd)\n", n);

            df[n]->ejectDisk();
            break;

        default:
            fatalError;
    }

    switch (n) {
        case 0: scheduler.cancel<SLOT_DC0>(); break;
        case 1: scheduler.cancel<SLOT_DC1>(); break;
        case 2: scheduler.cancel<SLOT_DC2>(); break;
        case 3: scheduler.cancel<SLOT_DC3>(); break;
    }
}
