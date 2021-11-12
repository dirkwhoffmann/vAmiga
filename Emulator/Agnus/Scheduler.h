// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SchedulerTypes.h"
#include "SubComponent.h"
// #include "Event.h"

/* About the event scheduler
 *
 * vAmiga is an event triggered emulator. If an action has to be performed at
 * a specific DMA cycle (e.g., activating the Copper at a certain beam
 * position), the action is scheduled via the event handling API and executed
 * when the trigger cycle is reached.
 * Scheduled events are stored in so called event slots. Each slot is either
 * empty or contains a single event and is bound to a specific component. E.g.,
 * there is slot for Copper events, a slot for the Blitter events, a slot
 * for UART event, and so forth.
 * From a theoretical point of view, each event slot represents a state machine
 * running in parallel to the ones in the other slots. Keep in mind that the
 * state machines interact with each other in various ways (e.g., by blocking
 * the DMA bus). As a result, the slot ordering is important: If two events
 * trigger at the same cycle, the the slot with a smaller number is served
 * first.
 * To optimize speed, the event slots are categorized into primary slots and
 * secondary slots. Primary slots manage frequently occurring events (CIA
 * execution, DMA operations, etc.). Secondary slots manage events that only
 * occurr occasionally (e.g., a signal change on the serial port). Accordingly,
 * we call an event a primary event if if it scheduled in a primary slot and a
 * secondary event if it is scheduled in a secondary slot.
 * By default, the event handler only checks the primary event slots on a
 * regular basis. To make the event handler check all slots, a special event
 * has to be scheduled in the SEC_SLOT (which is a primary slot and therefore
 * always checked). Triggering this event works like a wakeup call by telling
 * the event handler to check for secondary events as well. Hence, whenever an
 * event is schedules in a secondary slot, it has to be ensured that SEC_SLOT
 * contains a wakeup with a trigger cycle matching the smallest trigger cycle
 * of all secondary events.
 * Scheduling the wakeup event in SEC_SLOT is transparant for the callee. When
 * an event is scheduled, the event handler automatically checks if the
 * selected slot is primary or secondary and schedules the SEC_SLOT
 * automatically in the latter case.
 *
 * To schedule an event, an event slot, a trigger cycle, and an event id
 * need to be provided. The trigger cycle is measured in master cycles. It can
 * be specified in multiple ways:
 *
 *   Absolute (Abs):
 *   The trigger cycle is specified as an absolute value.
 *
 *   Immediate (Imm):
 *   The trigger cycle is the next DMA cycle.
 *
 *   Incremental (Inc):
 *   The trigger cycle is specified relative to the current slot value.
 *
 *   Relative (Rel): (Implemented by Agnus)
 *   The trigger cycle is specified relative to the current DMA clock.
 *
 *   Positional (Pos): (Implemented by Agnus)
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

class Scheduler : public SubComponent {

    // Result of the latest inspection
    mutable EventInfo info = {};
    mutable EventSlotInfo slotInfo[SLOT_COUNT];

public:
    
    // Trigger cycle
    Cycle trigger[SLOT_COUNT] = { };

    // The event identifier
    EventID id[SLOT_COUNT] = { };

    // An optional data value
    i64 data[SLOT_COUNT] = { };
    
    // Next trigger cycle
    Cycle nextTrigger = NEVER;
    
    
    //
    // Class methods
    //
    
    static const char *eventName(EventSlot slot, EventID id);
    
    
    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;
        
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "Scheduler"; }
    void _dump(dump::Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _initialize() override;
    void _reset(bool hard) override;
    void _inspect() const override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        worker
        
        // >> slot
        << trigger
        << id
        << data
        << nextTrigger;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Analyzing
    //
    
public:
    
    EventInfo getInfo() const { return AmigaComponent::getInfo(info); }
    EventSlotInfo getSlotInfo(isize nr) const;

private:
    
    void inspectSlot(EventSlot nr) const;
    
    
    //
    // Checking events
    //
    
public:
    
    // Returns true iff the specified slot contains any event
    template<EventSlot s> bool hasEvent() const { return this->id[s] != (EventID)0; }
    
    // Returns true iff the specified slot contains a specific event
    template<EventSlot s> bool hasEvent(EventID id) const { return this->id[s] == id; }
    
    // Returns true iff the specified slot contains a pending event
    template<EventSlot s> bool isPending() const { return this->trigger[s] != NEVER; }
    
    // Returns true iff the specified slot contains a due event
    template<EventSlot s> bool isDue(Cycle cycle) const { return cycle >= this->trigger[s]; }
    
    
    //
    // Scheduling events
    //
    
public:
    
    template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id)
    {
        this->trigger[s] = cycle;
        this->id[s] = id;
        
        if (cycle < nextTrigger) nextTrigger = cycle;
        
        if constexpr (isTertiarySlot(s)) {
            if (cycle < trigger[SLOT_TER]) trigger[SLOT_TER] = cycle;
        }
        if constexpr (isSecondarySlot(s)) {
            if (cycle < trigger[SLOT_SEC]) trigger[SLOT_SEC] = cycle;
        }
    }
    
    template<EventSlot s> void scheduleAbs(Cycle cycle, EventID id, i64 data)
    {
        scheduleAbs<s>(cycle, id);
        this->data[s] = data;
    }
    
    template<EventSlot s> void scheduleImm(EventID id)
    {
        scheduleAbs<s>(0, id);
    }
    
    template<EventSlot s> void scheduleImm(EventID id, i64 data)
    {
        scheduleAbs<s>(0, id);
        this->data[s] = data;
    }
        
    template<EventSlot s> void scheduleInc(Cycle cycle, EventID id)
    {
        scheduleAbs<s>(trigger[s] + cycle, id);
    }
    
    template<EventSlot s> void scheduleInc(Cycle cycle, EventID id, i64 data)
    {
        scheduleAbs<s>(trigger[s] + cycle, id);
        this->data[s] = data;
    }
        
    template<EventSlot s> void rescheduleAbs(Cycle cycle)
    {
        trigger[s] = cycle;
        if (cycle < nextTrigger) nextTrigger = cycle;
        
        if constexpr (isTertiarySlot(s)) {
            if (cycle < trigger[SLOT_TER]) trigger[SLOT_TER] = cycle;
        }
        if constexpr (isSecondarySlot(s)) {
            if (cycle < trigger[SLOT_SEC]) trigger[SLOT_SEC] = cycle;
        }
    }
    
    template<EventSlot s> void rescheduleInc(Cycle cycle)
    {
        rescheduleAbs<s>(trigger[s] + cycle);
    }
        
    template<EventSlot s> void cancel()
    {
        id[s] = (EventID)0;
        data[s] = 0;
        trigger[s] = NEVER;
    }
    
    //
    // Processing events
    //

public:

    // Processes all events up to a given master cycle
    void executeUntil(Cycle cycle);
};
