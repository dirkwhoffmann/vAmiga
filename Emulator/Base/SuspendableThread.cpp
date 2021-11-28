// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "SuspendableThread.h"

void
SuspendableThread::suspend()
{
    debug(RUN_DEBUG, "Suspending (%ld)...\n", suspendCounter);
    
    if (suspendCounter || isRunning()) {
        pause();
        suspendCounter++;
    }
}

void
SuspendableThread::resume()
{
    debug(RUN_DEBUG, "Resuming (%ld)...\n", suspendCounter);
    
    if (suspendCounter && --suspendCounter == 0) {
        run();
    }
}
