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

// Time stamp used for messages that never trigger
#define NEVER INT64_MAX

/* About the event handler.
 * vAmiga is an event triggered emulator. If an action has to be performed at
 * a specific cycle (e.g., activating the Copper at a specific beam position),
 * the action is scheduled via the event handler and executed when the trigger
 * cycle is reached.
 * The event handler is part of Agnus, because this component is in charge of
 * synchronize timing between the various components.
 * Scheduled events are stored in so called event slots. Each slot is either
 * empty or contains a single event and is bound to a specific component. E.g.,
 * there is slot for Copper events, a slot for the Blitter events, and a slot
 * storing rasterline events (pixel drawing, HSYNC action).
 * From a theoretical point of view, each event slot represents a state machine
 * running in parallel to the ones in the other slots. Keep in mind that the
 * state machines do interact with each other in various ways (e.g., by blocking
 * the DMA bus). As a result, the slot ordering is of great importance: If two
 * events trigger at the same cycle, the the slot with a smaller number is
 * served first.
 * The available event slots are stored in two different tables: The primary
 * event table and the secondary event table. The primary table contains the
 * slots for all frequently occurring events (CIA execution, DMA operations,
 * etc.). The secondary table contains the slots for events that occurr
 * occasionally (e.g., a serial port interrupt). The separation into two event
 * tables has been done for speed reasons. It keeps the primary table short
 * which has to be crawled through whenever an event is processed.
 * The secondary event table is linked to the primary table via the secondary
 * event slot (SEC_SLOT). Triggering an event in this slot causes the event
 * handler to process all pending events in the secondary event list. Hence,
 * whenever a secondary event is scheduled, a primary event is scheduled in
 * SEC_SLOT with a trigger cycle matching the smallest trigger cycle of all
 * secondary events.
 */

struct Event
{
    // Indicates when the event is due.
    Cycle triggerCycle;
    
    // Identifier of the scheduled event.
    EventID id;
    
    // Optional data value
    // Can be used to pass additional information to the event handler.
    int64_t data;
};

class EventHandler : public HardwareComponent
{
    // Quick-access references
    class CIAA *ciaA;
    class CIAB *ciaB;
    class Memory *mem;
    class Agnus *agnus;
    class Copper *copper;
    class Denise *denise;
    class Paula *paula; 
    
    // Information shown in the GUI inspector panel
    EventHandlerInfo info;
    
    
    //
    // Event tables
    //
    
    public:

    // The event table
    Event slot[SLOT_COUNT];

    // The primary event table (DEPRECATED)
    // Event primSlot[LAST_PRIM_SLOT + 1];
    
    // Next trigger cycle for an event in the primary event table
    Cycle nextPrimTrigger = NEVER;
    
    // The secondary event table (DEPRECATED)
    // Event secSlot[LAST_SEC_SLOT + 1];
    
    // Next trigger cycle for an event in the secondary event table
    Cycle nextSecTrigger = NEVER;
    
    
    //
    // Constructing and destructing
    //
    
    public:
    
    EventHandler();
    
    
    //
    // Methods from HardwareComponent
    //
    
    private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _inspect() override; 
    void _dump() override;
    
    // Helper functions
    void _inspectPrimSlot(uint32_t slot);
    void _inspectSecSlot(uint32_t slot);
    void _dumpPrimaryTable();
    void _dumpSecondaryTable();
    void _dumpSlot(const char *slotName, const char *eventName, const Event event);
    
    
    //
    // Reading the internal state
    //
    
    public:
    
    // Returns the latest internal state recorded by inspect()
    EventHandlerInfo getInfo();
    EventSlotInfo getPrimarySlotInfo(int slot);
    EventSlotInfo getSecondarySlotInfo(int slot);
    
    // Returns the number of event slots in the primary or secondary table.
    inline long primSlotCount() { return LAST_PRIM_SLOT + 1; }
    inline long secSlotCount() { return  LAST_SEC_SLOT - FIRST_SEC_SLOT + 1; }


    // Returns true iff the specified slot contains an event.
    template<EventSlot s> bool hasEvent() {
        assert(s < SLOT_COUNT); return slot[s].id != (EventID)0; }

    // Returns true iff the specified slot contains a pending event.
    template<EventSlot s> bool isPending() {
        assert(s < SLOT_COUNT); return slot[s].triggerCycle != NEVER; }

    // Returns true iff the specified slot contains a due event.
    template<EventSlot s> bool isDue(Cycle cycle) {
        assert(s < SLOT_COUNT); return cycle >= slot[s].triggerCycle; }


    //
    // OLD API (DEPRECATED)
    //

    // Checks whether a particular slot in the primary table contains an event.
    inline bool hasEvent(EventSlot s) {
        assert(isPrimarySlot(s)); return slot[s].id != 0; }
    
    // Checks whether a particular slot in the secondary table contains an event.
    inline bool hasEventSec(EventSlot s) {
        assert(isSecondarySlot(s)); return slot[s].id != 0; }
    
    // Checks whether a particular slot in the primary table contains a pending event.
    inline bool isPending(EventSlot s) {
        assert(isPrimarySlot(s)); return slot[s].triggerCycle != NEVER; }
    
    // Checks whether a particular slot in the secondary table contains a pending event.
    inline bool isPendingSec(EventSlot s) {
        assert(isSecondarySlot(s)); return slot[s].triggerCycle != NEVER; }
    
    // Checks whether a particular slot in the primary table contains a due event.
    // inline bool isDue(EventSlot s, Cycle cycle) {
    //     assert(isPrimarySlot(s)); return cycle >= slot[s].triggerCycle; }
    
    // Checks whether a particular slot in the secondary table contains a due event.
    // inline bool isDueSec(EventSlot s, Cycle cycle) {
    //     assert(isSecondarySlot(s)); return cycle >= slot[s].triggerCycle; }
    
    
    //
    // Processing events
    //
    
    public:
    
    /* Processes all events that are due prior to or at the provided cycle.
     * This function is called inside the execution function of Agnus.
     */
    inline void executeUntil(Cycle cycle) {
        if (cycle >= nextPrimTrigger) _executeUntil(cycle); }
    
    private:
    
    // Called by executeUntil(...) to process events in the primary table.
    void _executeUntil(Cycle cycle);
    
    // Called by executeUntil(...) to process events in the secondary table.
    void _executeSecUntil(Cycle cycle);
    
    
    //
    // Scheduling events
    //
    
    /* To schedule an event, an event slot, a trigger cycle, and an event id
     * needs to be provided. The trigger cycle can be specified in three ways:
     *
     *   Absolute (Abs):
     *   The time stamp is an absolute value measured in master clock cycles.
     *
     *   Incremental (Inc):
     *   The time stamp is relative to the current time stamp in the slot.
     *
     *   Relative (Rel):
     *   The time stamp is relative to the current DMA clock and measured in
     *   master clock cycles.
     *
     *   Positional (Pos):
     *   The time stamp is provided as a beam position in the current frame.
     *
     * Events can also be rescheduled, disabled, or canceled:
     *
     *   Rescheduling means that the event ID in the selected event slot
     *   remains unchanged.
     *
     *   Disabling means that the trigger cycle is set to NEVER. All other slot
     *   items are untouched.
     *
     *   Canceling means that the slot is emptied by deleting the event ID
     *   and setting the trigger cycle to NEVER.
     */

    Cycle relToCycle(Cycle cycle);
    Cycle posToCycle(int16_t vpos, int16_t hpos);

    public:

    template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id)
    {
        assert(isPrimarySlot(s));

        slot[s].triggerCycle = cycle;
        slot[s].id = id;
        if (cycle < nextPrimTrigger) nextPrimTrigger = cycle;

        assert(checkScheduledEvent(s));
    }

    template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id, int64_t data)
    {
        scheduleAbs<s>(cycle, id);
        slot[s].data = data;
    }

    template<EventSlot s> void scheduleInc(Cycle cycle, EventID id)
    {
        scheduleAbs<s>(cycle, slot[s].triggerCycle + cycle);
    }

    template<EventSlot s> void scheduleInc(Cycle cycle, EventID id, int64_t data)
    {
        scheduleAbs<s>(cycle, slot[s].triggerCycle + cycle);
        slot[s].data = data;
    }

    template<EventSlot s> void scheduleRel(Cycle cycle, EventID id)
    {
        scheduleAbs<s>(relToCycle(cycle), id);
    }

    template<EventSlot s> void scheduleRel(Cycle cycle, EventID id, int64_t data)
    {
        scheduleAbs<s>(relToCycle(cycle), id);
        slot[s].data = data;
    }


    template<EventSlot s> void schedulePos(int16_t vpos, int16_t hpos, EventID id)
    {
        scheduleAbs<s>(posToCycle(vpos, hpos), id);
    }

    template<EventSlot s> void schedulePos(int16_t vpos, int16_t hpos, EventID id, int64_t data)
    {
        scheduleAbs<s>(posToCycle(vpos, hpos), id, data);
        slot[s].data = data;
    }

    template<EventSlot s> void rescheduleAbs(Cycle cycle)
    {
        assert(isPrimarySlot(s));

        slot[s].triggerCycle = cycle;
        if (cycle < nextPrimTrigger) nextPrimTrigger = cycle;

        assert(checkScheduledEvent(s));
    }

    template<EventSlot s> void rescheduleInc(Cycle cycle)
    {
        rescheduleAbs<s>(relToCycle(cycle), slot[s].triggerCycle + cycle);
    }

    template<EventSlot s> void rescheduleRel(Cycle cycle)
    {
        rescheduleAbs<s>(relToCycle(cycle));
    }

    template<EventSlot s> void reschedulePos(int16_t vpos, int16_t hpos)
    {
        rescheduleAbs<s>(posToCycle(vpos, hpos));
    }

    template<EventSlot s> void cancel()
    {
        slot[s].id = (EventID)0;
        slot[s].data = 0;
        slot[s].triggerCycle = NEVER;
    }

    //
    // OLD API (DEPRECATED)
    //

    // Schedules a new event in the primary event table.
    void scheduleAbs(EventSlot s, Cycle cycle, EventID id);
    void scheduleRel(EventSlot s, Cycle cycle, EventID id);
    void schedulePos(EventSlot s, int16_t vpos, int16_t hpos, EventID id);
    
    // Reschedules an existing event in the primary event table.
    void rescheduleAbs(EventSlot s, Cycle cycle);
    void rescheduleRel(EventSlot s, Cycle cycle);
    
    // Disables an event in the primary event table. (DEPRECATED)
    void disable(EventSlot s);
    
    // Deletes an event in the primary event table.
    void cancel(EventSlot s);
    
    
    // Schedules a new event in the secondary event table.
    void scheduleSecAbs(EventSlot s, Cycle cycle, EventID id);
    void scheduleSecAbs(EventSlot s, Cycle cycle, EventID id, int64_t data);
    void scheduleSecRel(EventSlot s, Cycle cycle, EventID id);
    void scheduleSecRel(EventSlot s, Cycle cycle, EventID id, int64_t data);
    void scheduleSecPos(EventSlot s, int16_t vpos, int16_t hpos, EventID id);
    void scheduleSecPos(EventSlot s, int16_t vpos, int16_t hpos, EventID id, int64_t data);

    // Reschedules an existing event in the secondary event table.
    void rescheduleSecAbs(EventSlot s, Cycle cycle);
    void rescheduleSecRel(EventSlot s, Cycle cycle);
    
    // Disables an event in the secondary event table.
    void disableSec(EventSlot s);
    
    // Deletes an event in the secondary event table.
    void cancelSec(EventSlot s);

    // Schedules a register write event
    void scheduleRegEvent(EventSlot slot, Cycle cycle, EventID id, int64_t data);

    private:

    // Serves an interrupt slot event
    void serveIRQEvent(EventSlot slot, int irqBit);

    // Serves a register write event
    void serveRegEvent(EventSlot slot);

    // Serves an inspection event
    void serveINSEvent();

    
    //
    // Debugging
    //
    
    private:
    
    /* Performs some debugging checks. Won't be executed in release build.
     * The provided slot must be a slot in the primary event table.
     */
    bool checkScheduledEvent(EventSlot s);
    bool checkTriggeredEvent(EventSlot s);
    
};

#endif
