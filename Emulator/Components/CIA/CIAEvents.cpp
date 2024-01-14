// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CIA.h"
#include "Agnus.h"

namespace vamiga {

void
CIA::serviceEvent(EventID id)
{
    switch(id) {

        case CIA_EXECUTE:
            
            executeOneCycle();
            break;

        case CIA_WAKEUP:
            
            wakeUp();
            break;

        default:
            fatalError;
    }
}

void
CIA::scheduleNextExecution()
{
    if (isCIAA()) {
        agnus.scheduleAbs<SLOT_CIAA>(clock + CIA_CYCLES(1), CIA_EXECUTE);
    } else {
        agnus.scheduleAbs<SLOT_CIAB>(clock + CIA_CYCLES(1), CIA_EXECUTE);
    }
}

void
CIA::scheduleWakeUp()
{
    if (isCIAA()) {
        agnus.scheduleAbs<SLOT_CIAA>(wakeUpCycle, CIA_WAKEUP);
    } else {
        agnus.scheduleAbs<SLOT_CIAB>(wakeUpCycle, CIA_WAKEUP);
    }
}

}
