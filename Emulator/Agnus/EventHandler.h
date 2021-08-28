// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Aliases.h"



public:

// Returns true iff the specified slot contains any event
template<EventSlot s> bool hasEvent() const { return scheduler.slot[s].id != (EventID)0; }

// Returns true iff the specified slot contains a specific event
template<EventSlot s> bool hasEvent(EventID id) const { return scheduler.slot[s].id == id; }

// Returns true iff the specified slot contains a pending event
template<EventSlot s> bool isPending() const { return scheduler.slot[s].triggerCycle != NEVER; }

// Returns true iff the specified slot contains a due event
template<EventSlot s> bool isDue(Cycle cycle) const { return cycle >= scheduler.slot[s].triggerCycle; }


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
 *   Immediate (Imm):
 *   The trigger cycle is the next DMA cycle.
 *
 *   Relative (Rel):
 *   The trigger cycle is specified relative to the current DMA clock.
 *
 *   Incremental (Inc):
 *   The trigger cycle is specified relative to the current slot value.
 *
 *   Positional (Pos):
 *   The trigger cycle is specified in form of a beam position.
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
    scheduler.slot[s].triggerCycle = cycle;
    scheduler.slot[s].id = id;
    if (cycle < scheduler.nextTrigger) scheduler.nextTrigger = cycle;

    if (isSecondarySlot(s) && cycle < scheduler.slot[SLOT_SEC].triggerCycle)
        scheduler.slot[SLOT_SEC].triggerCycle = cycle;
}

template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id, i64 data)
{
    scheduleAbs<s>(cycle, id);
    scheduler.slot[s].data = data;
}

template<EventSlot s> void scheduleImm(EventID id)
{
    scheduleAbs<s>(0, id);
}

template<EventSlot s> void scheduleImm(EventID id, i64 data)
{
    scheduleAbs<s>(0, id);
    scheduler.slot[s].data = data;
}

template<EventSlot s> void scheduleRel(Cycle cycle, EventID id)
{
    scheduleAbs<s>(clock + cycle, id);
}

template<EventSlot s> void scheduleRel(Cycle cycle, EventID id, i64 data)
{
    scheduleAbs<s>(clock + cycle, id);
    scheduler.slot[s].data = data;
}

template<EventSlot s> void scheduleInc(Cycle cycle, EventID id)
{
    scheduleAbs<s>(scheduler.slot[s].triggerCycle + cycle, id);
}

template<EventSlot s> void scheduleInc(Cycle cycle, EventID id, i64 data)
{
    scheduleAbs<s>(scheduler.slot[s].triggerCycle + cycle, id);
    scheduler.slot[s].data = data;
}

template<EventSlot s> void schedulePos(isize vpos, isize hpos, EventID id)
{
    scheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ), id);
}

template<EventSlot s> void schedulePos(isize vpos, isize hpos, EventID id, i64 data)
{
    scheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ), id, data);
}

template<EventSlot s> void rescheduleAbs(Cycle cycle)
{
    scheduler.slot[s].triggerCycle = cycle;
    if (cycle < scheduler.nextTrigger) scheduler.nextTrigger = cycle;
    
     if (isSecondarySlot(s) && cycle < scheduler.slot[SLOT_SEC].triggerCycle)
         scheduler.slot[SLOT_SEC].triggerCycle = cycle;
}

template<EventSlot s> void rescheduleInc(Cycle cycle)
{
    rescheduleAbs<s>(scheduler.slot[s].triggerCycle + cycle);
}

template<EventSlot s> void rescheduleRel(Cycle cycle)
{
    rescheduleAbs<s>(clock + cycle);
}

template<EventSlot s> void reschedulePos(i16 vpos, i16 hpos)
{
    rescheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ));
}

template<EventSlot s> void cancel()
{
    scheduler.slot[s].id = (EventID)0;
    scheduler.slot[s].data = 0;
    scheduler.slot[s].triggerCycle = NEVER;
}


//
// Scheduling specific events
//

// Schedules the next BPL event relative to a given DMA cycle.
void scheduleNextBplEvent(isize hpos);

// Schedules the next BPL event relative to the currently emulated DMA cycle.
void scheduleNextBplEvent() { scheduleNextBplEvent(pos.h); }

// Schedules the earliest BPL event that occurs at or after the given DMA cycle.
void scheduleBplEventForCycle(isize hpos);

// Updates the scheduled BPL event according to the current event table.
void updateBplEvent() { scheduleBplEventForCycle(pos.h); }

// Schedules the next DAS event relative to a given DMA cycle.
void scheduleNextDasEvent(isize hpos);

// Schedules the next DAS event relative to the currently emulated DMA cycle.
void scheduleNextDasEvent() { scheduleNextDasEvent(pos.h); }

// Schedules the earliest DAS event that occurs at or after the given DMA cycle.
void scheduleDasEventForCycle(isize hpos);

// Updates the scheduled DAS event according to the current event table.
void updateDasEvent() { scheduleDasEventForCycle(pos.h); }

// Schedules the next register change event
void scheduleNextREGEvent();


//
// Processing events
//

private:

/* Executes the event handler up to a given master cycle.
 * This method is called inside Agnus::executeUntil().
 */
// void executeEventsUntil(Cycle cycle);

// Event handlers for specific slots
template <int nr> void serviceCIAEvent();
void serviceREGEvent(Cycle until);
void serviceBPLEvent();
template <int nr> void serviceBPLEventHires();
template <int nr> void serviceBPLEventLores();
void serviceBPLEventLores();
void serviceDASEvent();
void serviceRASEvent();

public:

void serviceINSEvent();

//
// Debugging
//

private:

/* Performs some debugging checks. Won't be executed in release build.
 * The provided slot must be a slot in the primary event table.
 */
bool checkScheduledEvent(EventSlot s);
bool checkTriggeredEvent(EventSlot s);

