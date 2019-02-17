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
        if (true) { // isPending((Event)i)) {
            plainmsg("Event %d: Triggers at cycle %lld [%lld]\n", i, eventCycle[i], payload[i]);
        }
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
    // if (cycle < nextTrigger) nextTrigger = cycle;
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
EventHandler::_executeUntil(Cycle cycle) {
    
    nextTrigger = UINT64_MAX;
        
    // Iterate through all events
    for (unsigned i = 0; i < NUMBER_OF_EVENTS; i++) {
        
        Cycle eventcycle = eventCycle[i];
        
        if (cycle < UINT64_MAX) {
            
            // Event is pending. Check whether it is due.
            if (cycle >= eventcycle) {
                
                // Process event
                switch (i) {
                    case EVENT_CIAA:
                        
                        if (payload[i] == 0) { // Execute
                            amiga->ciaA.executeOneCycle();
                        } else { // Wakeup
                            amiga->ciaA.wakeUp();
                        }
                        if (!amiga->ciaA.isUpToDate()) {
                            amiga->ciaA.dump();
                        }
                        assert(amiga->ciaA.isUpToDate());

                        break;
                        
                    case EVENT_CIAB:
                        
                        if (payload[i] == 0) { // Execute
                            amiga->ciaB.executeOneCycle();
                        } else { // Wakeup
                            amiga->ciaB.wakeUp();
                        }
                        if (!amiga->ciaB.isUpToDate()) {
                            amiga->ciaB.dump();
                        }
                        assert(amiga->ciaB.isUpToDate());
                        
                        break;
                        
                    default:
                        assert(false);
                }
                
            } else {
                
                // Determine the next trigger cycle
                if (eventCycle[i] < nextTrigger) {
                    nextTrigger = eventCycle[i];
                }
            }
        }
    }
}
