// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Scheduler.h"
#include "Agnus.h"
#include "Paula.h"
#include "Keyboard.h"

void
Scheduler::_initialize()
{
    // Wipe out event slots
    // std::memset(slot, 0, sizeof(slot));
}

void
Scheduler::_reset(bool hard)
{
    auto insEvent = slot[SLOT_INS].id;
    
    RESET_SNAPSHOT_ITEMS(hard)
    
    // Initialize all event slots
    for (isize i = 0; i < SLOT_COUNT; i++) {
        
        slot[i].triggerCycle = NEVER;
        slot[i].id = (EventID)0;
        slot[i].data = 0;
    }
    
    // Reschedule the old inspection event if there was one
    if (insEvent) scheduleAbs <SLOT_INS> (0, insEvent);

}

void
Scheduler::_inspect() const
{
    synchronized {
    }
}

void
Scheduler::executeUntil(Cycle cycle) {

    //
    // Check primary slots
    //

    if (isDue<SLOT_RAS>(cycle)) {
        agnus.serviceRASEvent();
    }
    if (isDue<SLOT_REG>(cycle)) {
        agnus.serviceREGEvent(cycle);
    }
    if (isDue<SLOT_CIAA>(cycle)) {
        agnus.serviceCIAEvent<0>();
    }
    if (isDue<SLOT_CIAB>(cycle)) {
        agnus.serviceCIAEvent<1>();
    }
    if (isDue<SLOT_BPL>(cycle)) {
        agnus.serviceBPLEvent();
    }
    if (isDue<SLOT_DAS>(cycle)) {
        agnus.serviceDASEvent();
    }
    if (isDue<SLOT_COP>(cycle)) {
        copper.serviceEvent(slot[SLOT_COP].id);
    }
    if (isDue<SLOT_BLT>(cycle)) {
        blitter.serviceEvent(slot[SLOT_BLT].id);
    }

    if (isDue<SLOT_SEC>(cycle)) {

        //
        // Check secondary slots
        //

        if (isDue<SLOT_CH0>(cycle)) {
            paula.channel0.serviceEvent();
        }
        if (isDue<SLOT_CH1>(cycle)) {
            paula.channel1.serviceEvent();
        }
        if (isDue<SLOT_CH2>(cycle)) {
            paula.channel2.serviceEvent();
        }
        if (isDue<SLOT_CH3>(cycle)) {
            paula.channel3.serviceEvent();
        }
        if (isDue<SLOT_DSK>(cycle)) {
            paula.diskController.serviceDiskEvent();
        }
        if (isDue<SLOT_DCH>(cycle)) {
            paula.diskController.serviceDiskChangeEvent();
        }
        if (isDue<SLOT_VBL>(cycle)) {
            agnus.serviceVblEvent();
        }
        if (isDue<SLOT_IRQ>(cycle)) {
            paula.serviceIrqEvent();
        }
        if (isDue<SLOT_KBD>(cycle)) {
            keyboard.serviceKeyboardEvent(slot[SLOT_KBD].id);
        }
        if (isDue<SLOT_TXD>(cycle)) {
            uart.serviceTxdEvent(slot[SLOT_TXD].id);
        }
        if (isDue<SLOT_RXD>(cycle)) {
            uart.serviceRxdEvent(slot[SLOT_RXD].id);
        }
        if (isDue<SLOT_POT>(cycle)) {
            paula.servicePotEvent(slot[SLOT_POT].id);
        }
        if (isDue<SLOT_IPL>(cycle)) {
            paula.serviceIplEvent();
        }
        if (isDue<SLOT_INS>(cycle)) {
            agnus.serviceINSEvent();
        }

        // Determine the next trigger cycle for all secondary slots
        Cycle next = slot[SLOT_SEC + 1].triggerCycle;
        for (isize i = SLOT_SEC + 2; i < SLOT_COUNT; i++) {
            if (slot[i].triggerCycle < next) {
                next = slot[i].triggerCycle;
            }
        }
        rescheduleAbs<SLOT_SEC>(next);
    }

    // Determine the next trigger cycle for all primary slots
    Cycle next = slot[0].triggerCycle;
    for (isize i = 1; i <= SLOT_SEC; i++) {
        if (slot[i].triggerCycle < next) {
            next = slot[i].triggerCycle;
        }
    }
    scheduler.nextTrigger = next;
}
