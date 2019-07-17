// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* About the event handler.
 *
 * vAmiga is an event triggered emulator. If an action has to be performed at
 * a specific DMA cycle (e.g., activating the Copper at a certain beam
 * position), the action is scheduled via the event handling API and executed
 * when the trigger cycle is reached.
 * The event handler is part of Agnus, because this component is in charge of
 * synchronize timing between components.
 * Scheduled events are stored in so called event slots. Each slot is either
 * empty or contains a single event and is bound to a specific component. E.g.,
 * there is slot for Copper events, a slot for the Blitter events, and a slot
 * for managing UART event.
 * From a theoretical point of view, each event slot represents a state machine
 * running in parallel to the ones in the other slots. Keep in mind that the
 * state machines do interact with each other in various ways (e.g., by
 * blocking the DMA bus). As a result, the slot ordering is important: If two
 * events trigger at the same cycle, the the slot with a smaller number is
 * always served first.
 * To optimize speed, the event slots are categorized into primary slots and
 * secondary slots. The primary slots are those that who store frequently
 * occurring events (CIA execution, DMA operations, etc.) and the secondary
 * slots are those who store events that only occurr occasionally (e.g., a
 * signal change on the serial port). Correspondingly, we call an event a
 * primary event if if it scheduled in a primary slot and a secondary event if
 * it is called in a secondary slot.
 * By default, the event handler only checks the primary event slots on a
 * regular basis. To make the event handler check all slots, a special event
 * has to be scheduled in the SEC_SLOT (which is a primary slot and therefore
 * always checked). Triggering this event works like a wakeup by telling the
 * event handler to check for secondary events as well. Hence, whenever an
 * event is schedules in a secondary slot, it has to be ensured that SEC_SLOT
 * contains a wakeup with a trigger cycle matching the smallest trigger cycle
 * of all secondary events.
 * Scheduling the wakeup event in SEC_SLOT is transparant for the callee. When
 * an event is scheduled, the event handler automatically checks if the
 * selected slot is primary or secondary and schedules the SEC_SLOT
 * automatically in the latter case.
 */

public:

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
// Scheduling events
//

/* To schedule an event, an event slot, a trigger cycle, and an event id
 * need to be provided. The trigger cycle is measured in master cycles. It can
 * be specified in multiple ways:
 *
 *   Absolute (Abs):
 *   The trigger cycle is specified as an absolute value.
 *
 *   Relative (Rel):
 *   The time stamp is relative to the current DMA clock.
 *
 *   Incremental (Inc):
 *   The trigger cycle is specified relative to the current slot value.
 *
 *   Positional (Pos):
 *   The time stamp is specified as a beam position in the current frame.
 *
 * Events can also be rescheduled or canceled:
 *
 *   Rescheduling means that the event ID in the selected event slot
 *   remains unchanged.
 *
 *   Canceling means that the slot is emptied by deleting the setting the
 *   event ID and the event data to zero and the trigger cycle to NEVER.
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
    } else {
        checkScheduledEvent(s);
    }
}

template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id, int64_t data)
{
    scheduleAbs<s>(cycle, id);
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

template<EventSlot s> void scheduleInc(Cycle cycle, EventID id)
{
    scheduleAbs<s>(slot[s].triggerCycle + cycle, id);
}

template<EventSlot s> void scheduleInc(Cycle cycle, EventID id, int64_t data)
{
    scheduleAbs<s>(slot[s].triggerCycle + cycle, id);
    slot[s].data = data;
}

template<EventSlot s> void schedulePos(int16_t vpos, int16_t hpos, EventID id)
{
    scheduleAbs<s>(beamToCycle( Beam(vpos, hpos) ), id);
}

template<EventSlot s> void schedulePos(int16_t vpos, int16_t hpos, EventID id, int64_t data)
{
    scheduleAbs<s>(beamToCycle( Beam(vpos, hpos) ), id, data);
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
template<PokeSource s> void scheduleRegEvent(Cycle cycle, EventID id, int64_t data);


//
// Processing events
//

private:

/* Processes all events that are due prior to or at the provided cycle.
 * Called inside Agnus::executeUntil().
 */
void executeEventsUntil(Cycle cycle);

// Event handler for the DMA slot
void serviceBplEvent(EventID id);
void scheduleNextBplEvent();

// Event handler for the DAS slot
void serviceDASEvent(EventID id);

// Event handler for the SYNC slot
void serviceSYNCEvent(EventID id, int64_t data);

// Event handler for the various IRQ slots
void serviceIRQEvent(EventSlot slot, int irqBit);

// Event handler for slots REG_COPPER and REG_CPU
void serviceREGEvent(EventSlot slot);

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

