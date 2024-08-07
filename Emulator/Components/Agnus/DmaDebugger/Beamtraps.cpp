// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Beamtraps.h"
#include "Amiga.h"

namespace vamiga {

Beamtraps::Beamtraps(Agnus& ref) : GuardList(ref.emulator), agnus(ref)
{

}

void
Beamtraps::setNeedsCheck(bool value)
{
    // printf("Beamtraps::setNeedsCheck(%d)\n", value);
    scheduleNextEvent();
}

void
Beamtraps::serviceEvent()
{
    // printf("Beamtraps::serviceEvent()\n");
    agnus.amiga.setFlag(RL::BEAMTRAP_REACHED);
    scheduleNextEvent();
}

void 
Beamtraps::scheduleNextEvent()
{
    if (elements() == 0) {

        agnus.cancel<SLOT_BTR>();
        return;
    }

    isize next = INT_MAX;
    for (isize i = 0; i < elements(); i++) {

        const auto guard = guardNr(i);
        auto v = HI_WORD(guard->addr);
        auto h = LO_WORD(guard->addr);
        auto d = DMA_CYCLES(agnus.pos.diff(v, h));

        // printf("Beamtrap (%d,%d) diff: %lld\n", v, h, d);
        if (d < next) { next = d; agnus.scheduleRel<SLOT_BTR>(d, BTR_TRIGGER); }
    }
}

}
