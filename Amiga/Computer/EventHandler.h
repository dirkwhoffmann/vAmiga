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
    NUMBER_OF_EVENTS
} Event;


class EventHandler : public HardwareComponent {
    
public:
    /* Event slots
     * Each event consists of a trigger cycle and a payload in form of a
     * 64 bit data value. The default trigger cycle is INT64_MAX which means
     * that the event is not pending.
     */
    Cycle eventCycle[NUMBER_OF_EVENTS];
    int64_t payload[NUMBER_OF_EVENTS];
    
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
    void scheduleEvent(Event event, Cycle cycle);
    void scheduleEvent(Event event, Cycle cycle, int64_t data);
    void cancelEvent(Event event);
    
    // Returns true if the specified event is pending
    bool isPending(Event event);
    
    // Processes all events that are due at or prior to cycle.
    inline void executeUntil(Cycle cycle) {
        if (cycle >= nextTrigger) _executeUntil(cycle); }
    
    // Work horse for executeUntil()
    void _executeUntil(Cycle cycle);
};

#endif
