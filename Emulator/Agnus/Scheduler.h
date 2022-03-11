// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AgnusTypes.h"
#include "SubComponent.h"



class Scheduler : public SubComponent {

    // Result of the latest inspection

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
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;
        
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "Scheduler"; }
    void _dump(dump::Category category, std::ostream& os) const override { };
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _initialize() override;
    void _reset(bool hard) override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        worker
        
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
            if (cycle < trigger[SLOT_SEC]) trigger[SLOT_SEC] = cycle;
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
