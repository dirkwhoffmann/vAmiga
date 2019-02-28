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

// Note: The slot order does matter. If two events trigger at the same cycle,
// the slot with a smaller number is processed first.
typedef enum
{
    CIAA_SLOT = 0,    // CIA A execution
    CIAB_SLOT,        // CIA B execution
    BPL_SLOT,         // Bitplane DMA
    DAS_SLOT,         // Disk, Audio, and Sprite DMA
    COP_SLOT,         // Copper DMA
    BLT_SLOT,         // Blitter DMA
    RAS_SLOT,         // Raster line events
    EVENT_SLOT_COUNT
    
} EventSlot;

static inline bool isEventSlot(int32_t s) { return s <= EVENT_SLOT_COUNT; }

typedef enum
{
    EVENT_NONE = 0,
    
    // CIA slots
    CIA_EXECUTE = 1,
    CIA_WAKEUP,
    CIA_EVENT_COUNT,
    
    // Bitplane slot
    BPL_FETCH_LORES = 1,
    BPL_FETCH_HIRES,
    BPL_EVENT_COUNT,
    
    // Disk, Audio, and Sprite slot
    DAS_DISK = 1,
    DAS_AUDIO,
    DAS_SPRITE0,
    DAS_SPRITE1,
    DAS_SPRITE2,
    DAS_SPRITE3,
    DAS_SPRITE4,
    DAS_SPRITE5,
    DAS_SPRITE6,
    DAS_SPRITE7,
    DAS_EVENT_COUNT,
    
    // Copper slot
    COP_REQUEST_DMA = 1,
    COP_FETCH,
    COP_MOVE,
    COP_WAIT_OR_SKIP,
    COP_WAIT,
    COP_SKIP, 
    COP_JMP1,
    COP_JMP2,
    COP_EVENT_COUNT,
    
    // Blitter slot
    BLT_TODO,
    BLT_EVENT_COUNT,
    
    // HSYNC slot
    RAS_HSYNC = 1,
    RAS_EVENT_COUNT
    
} EventID;

static inline bool isCiaEvent(EventID id) { return id <= CIA_EVENT_COUNT; }
static inline bool isBplEvent(EventID id) { return id <= BPL_EVENT_COUNT; }
static inline bool isDasEvent(EventID id) { return id <= DAS_EVENT_COUNT; }
static inline bool isCopEvent(EventID id) { return id <= COP_EVENT_COUNT; }
static inline bool isBltEvent(EventID id) { return id <= BLT_EVENT_COUNT; }
static inline bool isRasEvent(EventID id) { return id <= RAS_EVENT_COUNT; }

struct Event {
    
    // Indicates when the event is due
    Cycle triggerCycle;
    
    /* Trigger beam position
     * This is an optional value that should be removed when the emulator
     * is stable enough. The variables are only written to if an event is
     * scheduled to trigger at a specific beam position. When the event
     * triggers (which is determined solely by the cycle value), the current
     * beam position is compared with the stored values. If they don't match,
     * emulation halts with an error message.
     */
    int32_t triggerBeam;
     
    /* Event id.
     * This value is evaluated inside the event handler to determine the
     * action that needs to be taken.
     */
    EventID id;
    
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
    Event eventSlot[EVENT_SLOT_COUNT];
    
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
    
    // Schedules an event at the specified cycle.
    void scheduleEvent(EventSlot s, Cycle cycle, EventID id, int64_t data = 0);

    // Schedules an event at the specified beam position.
    void scheduleEvent(EventSlot s, int16_t vpos, int16_t hpos, EventID id, int64_t data = 0);

    // Schedules an event at the specified beam position in the next frame.
    void scheduleEventInNextFrame(EventSlot s, int16_t vpos, int16_t hpos, EventID id, int64_t data = 0);

    // Reschedules the event in the specified slot
    void rescheduleEvent(EventSlot s, Cycle addon);

    // Cancels the event in the specified slot
    void cancelEvent(EventSlot s);
    
    // Returns true if the specified event slot contains a scheduled event
    inline bool isPending(EventSlot s) {
        assert(isEventSlot(s)); return eventSlot[s].triggerCycle != INT64_MAX; }

    // Returns true if the specified event slot is due at the provided cycle
    inline bool isDue(EventSlot s, Cycle cycle) { return cycle >= eventSlot[s].triggerCycle; }

    // Performs some debugging checks. Won't be executed in release build.
    bool checkScheduledEvent(EventSlot s);
    bool checkTriggeredEvent(EventSlot s);

    
    // Processes all events that are due at or prior to cycle.
    inline void executeUntil(Cycle cycle) {
        if (cycle >= nextTrigger) _executeUntil(cycle); }
    
    // Work horse for executeUntil()
    void _executeUntil(Cycle cycle);
};

#endif
