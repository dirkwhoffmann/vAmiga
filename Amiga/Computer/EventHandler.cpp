// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

EventHandler::EventHandler()
{
    setDescription("EventHandler");
}
    
void
EventHandler::_powerOn()
{
    for (unsigned i = 0; i < EVENT_SLOT_COUNT; i++) {
        eventSlot[i].triggerCycle = INT64_MAX;
        eventSlot[i].id = (EventID)0;
        eventSlot[i].data = 0;
    }
}

void
EventHandler::_powerOff()
{
    
}

void
EventHandler::_reset()
{
    
}

void
EventHandler::_ping()
{
    
}

void
EventHandler::_dump()
{
    plainmsg("Master clock: %lld\n", amiga->masterClock);
    plainmsg("   DMA clock: %lld\n", amiga->dma.clock);
    plainmsg("        hpos: %d\n", amiga->dma.hpos());
    plainmsg("        vpos: %d\n", amiga->dma.vpos());
    plainmsg("\n");
    for (unsigned i = 0; i < EVENT_SLOT_COUNT; i++) {
        plainmsg("Slot %d: %s (Cycle: %lld Type: %d Data: %lld)\n",
                 i,
                 isPending((EventSlot)i) ? "pending" : "not pending",
                 eventSlot[i].triggerCycle,
                 eventSlot[i].id,
                 eventSlot[i].data);
    }
}

void
EventHandler::scheduleEvent(EventSlot s, Cycle cycle, EventID id, int64_t data)
{
    assert(isEventSlot(s));
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].id = id;
    eventSlot[s].data = data;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

void
EventHandler::rescheduleEvent(EventSlot s, Cycle addon)
{
    assert(isEventSlot(s));
    
    Cycle cycle = eventSlot[s].triggerCycle + addon;
    eventSlot[s].triggerCycle = cycle;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

void
EventHandler::cancelEvent(EventSlot s)
{
    assert(isEventSlot(s));
    
    eventSlot[s].triggerCycle = INT64_MAX;
}
                     
void
EventHandler::_executeUntil(Cycle cycle) {
    
    // Check for a CIA A event
    if (isDue(CIAA_SLOT, cycle)) {
        
        switch(eventSlot[CIAA_SLOT].id) {
                
            case CIA_EXECUTE:
                amiga->ciaA.executeOneCycle();
                break;
                
            case CIA_WAKEUP:
                amiga->ciaA.wakeUp();
                break;
                
            default:
                assert(false);
        }
    }
    
    // Check for a CIA B event
    if (isDue(CIAB_SLOT, cycle)) {
        
        switch(eventSlot[CIAB_SLOT].id) {
                
            case CIA_EXECUTE:
                amiga->ciaB.executeOneCycle();
                break;
                
            case CIA_WAKEUP:
                amiga->ciaB.wakeUp();
                break;
                
            default:
                assert(false);
        }
    }
 
    // Check for a Copper event
    if (isDue(COP_SLOT, cycle)) {
        amiga->dma.copper.processEvent(eventSlot[COP_SLOT].id, eventSlot[COP_SLOT].data);
    }
 
    // Check for a Blitter event
    if (isDue(BLT_SLOT, cycle)) {
        // amiga->dma.blitter.processEvent(eventSlot[BLT_SLOT].type, eventSlot[BLT_SLOT].data);
    }

    // Check for a raster event
    if (isDue(RAS_SLOT, cycle)) {
        amiga->dma.hsyncHandler();
    }
    

    // Determine the next trigger cycle
    nextTrigger = eventSlot[0].triggerCycle;
    for (unsigned i = 1; i < EVENT_SLOT_COUNT; i++)
        if (eventSlot[i].triggerCycle < nextTrigger)
            nextTrigger = eventSlot[i].triggerCycle;
}
