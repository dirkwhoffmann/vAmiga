// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


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


// Helper functions

private:

void _inspectEvents();
void _inspectSlot(EventSlot slot);
void _dumpPrimaryTable();
void _dumpSecondaryTable();
void _dumpSlot(const char *slotName, const char *eventName, const Event event);

public:

void _dumpEvents();

// Returns the latest internal state recorded by inspect()
EventHandlerInfo getEventInfo();
EventSlotInfo getSlotInfo(int nr);

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
// Processing events
//

public:

/* Processes all events that are due prior to or at the provided cycle.
 * This function is called inside the execution function of Agnus.
 */
void executeEventsUntil(Cycle cycle) {
    if (cycle >= nextTrigger) _executeEventsUntil(cycle); }

private:

// Called by executeUntil(...) to process events in the primary table.
void _executeEventsUntil(Cycle cycle);

// Called by executeUntil(...) to process events in the secondary table.
void _executeSecEventsUntil(Cycle cycle);


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

public:

template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id)
{
    // Schedule event
    slot[s].triggerCycle = cycle;
    slot[s].id = id;
    if (cycle < nextTrigger) nextTrigger = cycle;

    // Perform special actions for secondary events
    if (isSecondarySlot(s)) {

        if (cycle < slot[SEC_SLOT].triggerCycle) slot[SEC_SLOT].triggerCycle = cycle;
    }
}

template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id, int64_t data)
{
    scheduleAbs<s>(cycle, id);
    slot[s].data = data;
}

template<EventSlot s> void scheduleInc(Cycle cycle, EventID id)
{
    scheduleAbs<s>(slot[s].triggerCycle + cycle, id);
}

template<EventSlot s> void scheduleInc(Cycle cycle, EventID id, int64_t data)
{
    scheduleAbs<s>(slot[s].triggerCycle + cycle, id);
    slot[s].data = data;
}

template<EventSlot s> void scheduleRel(Cycle cycle, EventID id)
{
    scheduleAbs<s>(clock + cycle, id);
}

template<EventSlot s> void scheduleRel(Cycle cycle, EventID id, int64_t data)
{
    scheduleAbs<s>(clock + cycle, id);
    slot[s].data = data;
}


template<EventSlot s> void schedulePos(int16_t vpos, int16_t hpos, EventID id)
{
    scheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ), id);
}

template<EventSlot s> void schedulePos(int16_t vpos, int16_t hpos, EventID id, int64_t data)
{
    scheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ), id, data);
}

template<EventSlot s> void rescheduleAbs(Cycle cycle)
{
    slot[s].triggerCycle = cycle;
    if (cycle < nextTrigger) nextTrigger = cycle;
}

template<EventSlot s> void rescheduleInc(Cycle cycle)
{
    rescheduleAbs<s>(slot[s].triggerCycle + cycle);
}

template<EventSlot s> void rescheduleRel(Cycle cycle)
{
    rescheduleAbs<s>(clock + cycle);
}

template<EventSlot s> void reschedulePos(int16_t vpos, int16_t hpos)
{
    rescheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ));
}

template<EventSlot s> void cancel()
{
    slot[s].id = (EventID)0;
    slot[s].data = 0;
    slot[s].triggerCycle = NEVER;
}

void cancel(EventSlot s)
{
    slot[s].id = (EventID)0;
    slot[s].data = 0;
    slot[s].triggerCycle = NEVER;
}

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

