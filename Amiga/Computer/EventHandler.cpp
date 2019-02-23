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
        events[i].triggerCycle = INT64_MAX;
        events[i].type = EVENT_DEFAULT;
        events[i].data = 0;
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
        plainmsg("Event %d: %s (Cycle: %lld Type: %d Data: %lld)\n",
                 isPending((EventType)i) ? "pending" : "not pending",
                 events[i].triggerCycle,
                 events[i].type,
                 events[i].data);
    }
}

void
EventHandler::scheduleEvent(EventType event, Cycle cycle,
                            EventSubType type, int64_t data)
{
    assert(event < NUMBER_OF_EVENTS);
    
    events[event].triggerCycle = cycle;
    events[event].type = type;
    events[event].data = data;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

void
EventHandler::cancelEvent(EventType event)
{
    events[event].triggerCycle = INT64_MAX;
}
                     
bool
EventHandler::isPending(EventType event)
{
    assert(event < NUMBER_OF_EVENTS);
    
    return events[event].triggerCycle != INT64_MAX;
}

void
EventHandler::_executeUntil(Cycle cycle) {
    
    // Iterate through all events
    for (unsigned i = 0; i < NUMBER_OF_EVENTS; i++) {
        
        // Check if event is due
        if (cycle >= events[i].triggerCycle) {
     
            // Process it
            switch (i) {
                case EVENT_CIAA:
                
                    if (events[i].type == EVENT_CIA_EXECUTE) {
                        amiga->ciaA.executeOneCycle();
                    } else {
                        assert(events[i].type == EVENT_CIA_WAKEUP);
                        amiga->ciaA.wakeUp();
                    }
                    break;
                    
                case EVENT_CIAB:
                    
                    if (events[i].type == EVENT_CIA_EXECUTE) {
                        amiga->ciaB.executeOneCycle();
                    } else {
                        assert(events[i].type == EVENT_CIA_WAKEUP);
                        amiga->ciaB.wakeUp();
                    }
                    break;
                    
                default:
                    assert(false);
            }
        }
    }
    
    // Determine the next trigger cycle
    nextTrigger = events[0].triggerCycle;
    for (unsigned i = 1; i < NUMBER_OF_EVENTS; i++)
        if (events[i].triggerCycle < nextTrigger)
            nextTrigger = events[i].triggerCycle;
}
