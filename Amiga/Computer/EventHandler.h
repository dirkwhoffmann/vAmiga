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
    CIAA_SLOT = 0,
    CIAB_SLOT,
    COPPER_SLOT,
    BLITTER_SLOT,
    EVENT_SLOTS
    
} EventSlot;

static inline bool isEventSlot(int32_t s) { return s <= EVENT_SLOTS; }

typedef enum
{
    CIA_EXECUTE = 1,
    CIA_WAKEUP,
    CIA_EVENTS
    
} CiaEvent;

static inline bool isCiaEvent(int32_t e) { return e <= CIA_EVENTS; }

typedef enum
{
    COPPER_REQUEST_DMA = 1,
    COPPER_FETCH,
    COPPER_MOVE,
    COPPER_WAIT_OR_SKIP,
    COPPER_WAIT,
    COPPER_SKIP, 
    COPPER_JMP1,
    COPPER_JMP2,

    COPPER_EVENTS
    
} CopperEvent;

static inline bool isCopperEvent(int32_t e) { return e <= COPPER_EVENTS; }

struct Event {
    
    // Indicates when the event is due
    Cycle triggerCycle;
    
    /* Event type.
     * This value is evaluated inside the event handler to determine the
     * action that needs to be taken.
     */
    int32_t type;
    
    /* Data (optional)
     * This value can be used to pass data to the event handler.
     */
    int64_t data;
};

class EventHandler : public HardwareComponent {
    
public:
    /* Event slots.
     * There is one slot for each event owner. In each slot, a single event
     * can be scheduled at a time
     */
    Event eventSlot[EVENT_SLOTS];
    
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
    void scheduleEvent(EventSlot s, Cycle cycle, int32_t type, int64_t data = 0);
    void cancelEvent(EventSlot s);
    
    // Returns true if the specified event slot contains a scheduled event
    inline bool isPending(EventSlot s) {
        assert(isEventSlot(s)); return eventSlot[s].triggerCycle != INT64_MAX; }

    // Returns true if the specified event slot is due at the provided cycle
    inline bool isDue(EventSlot s, int64_t cycle) { return cycle >= eventSlot[s].triggerCycle; }

    // Processes all events that are due at or prior to cycle.
    inline void executeUntil(Cycle cycle) {
        if (cycle >= nextTrigger) _executeUntil(cycle); }
    
    // Work horse for executeUntil()
    void _executeUntil(Cycle cycle);
};

#endif
