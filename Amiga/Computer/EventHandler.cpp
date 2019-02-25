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
    for (unsigned i = 0; i < EVENT_SLOTS; i++) {
        eventSlot[i].triggerCycle = INT64_MAX;
        eventSlot[i].type = 0;
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
    for (unsigned i = 0; i < EVENT_SLOTS; i++) {
        plainmsg("Slot %d: %s (Cycle: %lld Type: %d Data: %lld)\n",
                 isPending((EventSlot)i) ? "pending" : "not pending",
                 eventSlot[i].triggerCycle,
                 eventSlot[i].type,
                 eventSlot[i].data);
    }
}

void
EventHandler::scheduleEvent(EventSlot s, Cycle cycle, int32_t type, int64_t data)
{
    assert(s < EVENT_SLOTS);
    
    eventSlot[s].triggerCycle = cycle;
    eventSlot[s].type = type;
    eventSlot[s].data = data;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

void
EventHandler::cancelEvent(EventSlot s)
{
    eventSlot[s].triggerCycle = INT64_MAX;
}
                     
void
EventHandler::_executeUntil(Cycle cycle) {
    
    // Check for a CIA A event
    if (isDue(CIAA_SLOT, cycle)) {
        
        switch(eventSlot[CIAA_SLOT].type) {
                
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
        
        switch(eventSlot[CIAB_SLOT].type) {
                
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
    if (isDue(COPPER_SLOT, cycle)) {
        amiga->dma.copper.processEvent(eventSlot[COPPER_SLOT].type, eventSlot[COPPER_SLOT].data);
    }
 
    // Check for a Blitter event
    if (isDue(BLITTER_SLOT, cycle)) {
        // amiga->dma.blitter.processEvent(eventSlot[BLITTER_SLOT].type, eventSlot[BLITTER_SLOT].data);
    }
    
    // Determine the next trigger cycle
    nextTrigger = eventSlot[0].triggerCycle;
    for (unsigned i = 1; i < EVENT_SLOTS; i++)
        if (eventSlot[i].triggerCycle < nextTrigger)
            nextTrigger = eventSlot[i].triggerCycle;
}
