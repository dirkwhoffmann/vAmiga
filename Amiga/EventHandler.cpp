// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
EventHandler::_powerOn()
{
    memset(eventCycle, 0xFF, sizeof(eventCycle));
    memset(payload, 0x00, sizeof(payload));
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
        if (isPending((Event)i)) {
            plainmsg("Event %d: Triggers at cycle %lld [%lld]\n", eventCycle[i], payload[i]);
        }
    }
}

void
EventHandler::scheduleEvent(Event event, uint64_t cycle)
{
    assert(event < NUMBER_OF_EVENTS);
    
    eventCycle[event] = cycle;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

void
EventHandler::scheduleEvent(Event event, uint64_t cycle, uint64_t data)
{
    assert(event < NUMBER_OF_EVENTS);
    
    eventCycle[event] = cycle;
    payload[event] = data;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

void
EventHandler::cancelEvent(Event event)
{
    eventCycle[event] = UINT64_MAX;
}
                     
bool
EventHandler::isPending(Event event)
{
    assert(event < NUMBER_OF_EVENTS);
    
    return eventCycle[event] < UINT64_MAX;
}
                     


void
EventHandler::_processUntil(uint64_t cycle) {
    
    nextTrigger = UINT64_MAX;
    
    // Iterate through all events
    for (unsigned i = 0; i < NUMBER_OF_EVENTS; i++) {
        
        if (cycle < UINT64_MAX) {
            
            // Event is pending. Check whether it is due.
            if (cycle >= eventCycle[i]) {
                
                // Process event
                switch (i) {
                    case EVENT_DEBUG1:
                        break;
                        
                    case EVENT_DEBUG2:
                        break;
                        
                    default:
                        assert(false);
                }
                
                // Delete event
                eventCycle[i] = UINT64_MAX;
                
            } else {
                
                // Determine the next trigger cycle
                if (eventCycle[i] < nextTrigger) {
                    nextTrigger = eventCycle[i];
                }
            }
        }
    }
}
