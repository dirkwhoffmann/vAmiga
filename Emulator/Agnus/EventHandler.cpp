// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "CIA.h"
#include "CPU.h"
#include "Keyboard.h"
#include "Paula.h"
#include "UART.h"

void
Agnus::scheduleNextBplEvent(isize hpos)
{
    assert(hpos >= 0 && hpos < HPOS_CNT);

    if (u8 next = nextBplEvent[hpos]) {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(next - pos.h), bplEvent[next]);
    }
    assert(hasEvent<SLOT_BPL>());
}

void
Agnus::scheduleBplEventForCycle(isize hpos)
{
    assert(hpos >= pos.h && hpos < HPOS_CNT);

    if (bplEvent[hpos] != EVENT_NONE) {
        scheduleRel<SLOT_BPL>(DMA_CYCLES(hpos - pos.h), bplEvent[hpos]);
    } else {
        scheduleNextBplEvent(hpos);
    }

    assert(hasEvent<SLOT_BPL>());
}

void
Agnus::scheduleNextDasEvent(isize hpos)
{
    assert(hpos >= 0 && hpos < HPOS_CNT);

    if (u8 next = nextDasEvent[hpos]) {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(next - pos.h), dasEvent[next]);
        assert(hasEvent<SLOT_DAS>());
    } else {
        scheduler.cancel<SLOT_DAS>();
    }
}

void
Agnus::scheduleDasEventForCycle(isize hpos)
{
    assert(hpos >= pos.h && hpos < HPOS_CNT);

    if (dasEvent[hpos] != EVENT_NONE) {
        scheduleRel<SLOT_DAS>(DMA_CYCLES(hpos - pos.h), dasEvent[hpos]);
    } else {
        scheduleNextDasEvent(hpos);
    }
}

void
Agnus::scheduleNextREGEvent()
{
    // Determine when the next register change happens
    Cycle nextTrigger = changeRecorder.trigger();

    // Schedule a register change event for that cycle
    scheduler.scheduleAbs<SLOT_REG>(nextTrigger, REG_CHANGE);
}
