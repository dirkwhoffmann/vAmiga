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
    for (unsigned i = 0; i < NUMBER_OF_EVENTS; i++) {
        eventCycle[i] = INT64_MAX;
        payload[i] = 0;
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
    for (unsigned i = 0; i < NUMBER_OF_EVENTS; i++) {
        plainmsg("Event %d: %s (Cycle: %lld Payload: %lld)\n",
                 isPending((Event)i) ? "pending" : "not pending", eventCycle[i], payload[i]);
    }
}

void
EventHandler::scheduleEvent(Event event, Cycle cycle)
{
    assert(event < NUMBER_OF_EVENTS);
    
    eventCycle[event] = cycle;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

void
EventHandler::scheduleEvent(Event event, Cycle cycle, int64_t data)
{
    assert(event < NUMBER_OF_EVENTS);
    
    eventCycle[event] = cycle;
    payload[event] = data;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

void
EventHandler::cancelEvent(Event event)
{
    eventCycle[event] = -1;
}
                     
bool
EventHandler::isPending(Event event)
{
    assert(event < NUMBER_OF_EVENTS);
    
    return eventCycle[event] != INT64_MAX;
}

void
EventHandler::_executeUntil(Cycle cycle) {
    
    // Iterate through all events
    for (unsigned i = 0; i < NUMBER_OF_EVENTS; i++) {
        
        // Check if event is due
        if (cycle >= eventCycle[i]) {
     
            // Process it
            switch (i) {
                case EVENT_CIAA:
                    
                    // Meaning of payload: 0 = Execute, 1 = Wakeup
                    if (payload[i] == 0) {
                        amiga->ciaA.executeOneCycle();
                    } else {
                        amiga->ciaA.wakeUp();
                    }
                    break;
                    
                case EVENT_CIAB:
                    
                    // Meaning of payload: 0 = Execute, 1 = Wakeup
                    if (payload[i] == 0) {
                        amiga->ciaB.executeOneCycle();
                    } else {
                        amiga->ciaB.wakeUp();
                    }
                    break;
                    
                default:
                    assert(false);
            }
        }
    }
    
    // Determine the next trigger cycle
    nextTrigger = eventCycle[0];
    for (unsigned i = 1; i < NUMBER_OF_EVENTS; i++)
        if (eventCycle[i] < nextTrigger) nextTrigger = eventCycle[i];
}
