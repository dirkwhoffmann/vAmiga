// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "DiskController.h"
#include "Agnus.h"
#include "FloppyDrive.h"
#include <cmath>

namespace vamiga {

void
DiskController::serviceDiskEvent()
{        
    // Receive next byte from the selected drive
    transferByte();
    
    // Schedule next event
    scheduleNextDiskEvent();
}

void
DiskController::scheduleFirstDiskEvent()
{
    dskEventDelay = 0.0;
    
    if (turboMode()) {
        agnus.cancel<SLOT_DSK>();
    } else {
        agnus.scheduleImm<SLOT_DSK>(DSK_ROTATE);
    }
}

void
DiskController::scheduleNextDiskEvent()
{
    static constexpr double bytesPerTrack = 12668.0;

    // How many revolutions per minute are we supposed to achieve?
    FloppyDrive *drive = getSelectedDrive();
    isize rpm = drive ? drive->config.rpm : 300;

    // Compute the time span between two incoming bytes
    double delay;

    if (rpm) {

        // Calculate the delay according to the desired rpm
        delay = (SEC(1) * 60.0) / (double)rpm / bytesPerTrack;

    } else {

        // Use the value that was hard-coded in vAmiga up to version v2.2
        delay = 8 * 55.98;
    }

    dskEventDelay += delay;
    double rounded = round(dskEventDelay);
    dskEventDelay -= rounded;

    if (turboMode()) {
        agnus.cancel<SLOT_DSK>();
    } else {
        agnus.scheduleRel<SLOT_DSK>(Cycle(rounded), DSK_ROTATE);
    }
}

}
