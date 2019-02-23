// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _EVENT_HANDLER_INC
#define _EVENT_HANDLER_INC

#include "HardwareComponent.h"

typedef enum
{
    EVENT_CIAA,
    EVENT_CIAB,
    EVENT_COPPER,
    NUMBER_OF_EVENTS
} EventType;

typedef enum
{
    EVENT_DEFAULT     = 0,
    
    EVENT_CIA_EXECUTE = 1,
    EVENT_CIA_WAKEUP  = 2,
    
    EVENT_COPPER_JMP1 = 1,
    EVENT_COPPER_JMP2 = 2,
} EventSubType;


struct Event {
    
    // Indicates when the event is due
    Cycle triggerCycle;
    
    /* Sub event type.
     * This value is evaluated inside the event handler to determine what
     * action needs to be taken.
     */
    EventSubType type;
    
    /* Data (optional)
     * This value can be used to pass data to the event handler.
     */
    int64_t data;
};

class EventHandler : public HardwareComponent {
    
public:
    /* Event slots.
     * There is one slot for each event type.
     */
    Event events[NUMBER_OF_EVENTS];
    
    // This variables indicates when the next event triggers.
    // INT64_MAX if no event is pending.
    Cycle nextTrigger = INT64_MAX;

    
    //
    // Constructing and destructing
    //
    
public:
    
    EventHandler();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
     void _powerOn() override;
     void _powerOff() override;
     void _reset() override;
     void _ping() override;
     void _dump() override;
    
public:
    
    // Schedules an event. The event will be executed at the specified cycle.
    void scheduleEvent(EventType event, Cycle cycle,
                       EventSubType type = EVENT_DEFAULT, int64_t data = 0);
    void cancelEvent(EventType event);
    
    // Returns true if the specified event is pending
    bool isPending(EventType event);
    
    // Processes all events that are due at or prior to cycle.
    inline void executeUntil(Cycle cycle) {
        if (cycle >= nextTrigger) _executeUntil(cycle); }
    
    // Work horse for executeUntil()
    void _executeUntil(Cycle cycle);
};

#endif
