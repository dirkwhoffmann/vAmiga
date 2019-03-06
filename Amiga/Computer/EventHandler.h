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

#define NEVER INT64_MAX

/* Event slots forming the primary event list
 * Each event slot represents a state machine that runs in parallel to the
 * ones in the other slots. Keep in mind that the state machines interact
 * with each other in various ways (e.g., by blocking the DMA bus).
 * As a result, the slot is of great importance: If two events trigger at the
 * same cycle, the the slot with a smaller number is served first.
 * The secondary event slot is very different to the others. Triggering an
 * event in this slot causes the event handler to crawl through the secondary
 * event list which is designed similar to the primary list.
 * The separation into two event lists has been done for speed reasons. The
 * secondary list contains events that fire infrequently, e.g., the interrupt
 * events. This keeps the primary list short which has to be crawled through
 * whenever an event is processed.
 */
typedef enum
{
    // Primary slot table
    
    CIAA_SLOT = 0,    // CIA A execution
    CIAB_SLOT,        // CIA B execution
    DMA_SLOT,         // Disk, Audio, Sprite, and Bitplane DMA
    COP_SLOT,         // Copper DMA
    BLT_SLOT,         // Blitter DMA
    RAS_SLOT,         // Raster line events
    SEC_SLOT,         // Secondary events
    EVENT_SLOT_COUNT,
    
    // Secondary slot table
    
    HSYNC_SLOT = 0,   // HSYNC event
    TBE_IRQ_SLOT,     // Source 0 IRQ (Serial port transmit buffer empty)
    DSKBLK_IRQ_SLOT,  // Source 1 IRQ (Disk block finished)
    SOFT_IRQ_SLOT,    // Source 2 IRQ (Software-initiated)
    PORTS_IRQ_SLOT,   // Source 3 IRQ (I/0 ports and timers)
    COPR_IRQ_SLOT,    // Source 4 IRQ (Copper)
    VERTB_IRQ_SLOT,   // Source 5 IRQ (Start of vertical blank)
    BLIT_IRQ_SLOT,    // Source 6 IRQ (Blitter finished)
    AUD0_IRQ_SLOT,    // Source 7 IRQ (Audio channel 0 block finished)
    AUD1_IRQ_SLOT,    // Source 8 IRQ (Audio channel 1 block finished)
    AUD2_IRQ_SLOT,    // Source 9 IRQ (Audio channel 2 block finished)
    AUD3_IRQ_SLOT,    // Source 10 IRQ (Audio channel 3 block finished)
    RBF_IRQ_SLOT,     // Source 11 IRQ (Serial port receive buffer full)
    DSKSYN_IRQ_SLOT,  // Source 12 IRQ (Disk sync register matches disk data)
    SEC_SLOT_COUNT,
    
} EventSlot;

static inline bool isEventSlot(int32_t s) { return s <= EVENT_SLOT_COUNT; }

typedef enum
{
    EVENT_NONE = 0,
    
    //
    // Events in primary event table
    //
    
    // CIA slots
    CIA_EXECUTE = 1,
    CIA_WAKEUP,
    CIA_EVENT_COUNT,
    
    // DMA slot
    DMA_DISK = 1,
    DMA_A0,
    DMA_A1,
    DMA_A2,
    DMA_A3,
    DMA_S0,
    DMA_S1,
    DMA_S2,
    DMA_S3,
    DMA_S4,
    DMA_S5,
    DMA_S6,
    DMA_S7,
    DMA_L1,
    DMA_L2,
    DMA_L3,
    DMA_L4,
    DMA_L5,
    DMA_L6,
    DMA_H1,
    DMA_H2,
    DMA_H3,
    DMA_H4,
    DMA_EVENT_COUNT,
    
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
    BLT_INIT = 1,
    BLT_EXECUTE,
    BLT_EVENT_COUNT,
    
    // HSYNC slot
    RAS_HSYNC = 1,
    RAS_EVENT_COUNT,
    
    // SEC slot
    SEC_TRIGGER = 1,
    SEC_EVENT_COUNT,
    
    
    //
    // Events in secondary event table
    //
    
    // IRQ slots
    IRQ_SET = 1,
    IRQ_CLEAR,
    IRQ_EVENT_COUNT,
    
    // HSYNC slot
    HSYNC_EOL = 1,
    HSYNC_EVENT_COUNT
    
} EventID;

static inline bool isCiaEvent(EventID id) { return id <= CIA_EVENT_COUNT; }
static inline bool isDmaEvent(EventID id) { return id <= DMA_EVENT_COUNT; }
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
    
    //
    // Main events
    //
    
    // The primary event table
    Event eventSlot[EVENT_SLOT_COUNT];
    
    // Next trigger cycle for an event in the primary event table
    Cycle nextTrigger = NEVER;

    // The secondary event table
    Event secondarySlot[SEC_SLOT_COUNT];
    
    // Next trigger cycle for an event in the secondary event table
    Cycle nextSecTrigger = NEVER;

    
    /* Trace flags
     * Setting the n-th bit to 1 will produce debug messages for events in
     * slot number n.
     */
    uint16_t trace = 0;
    

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
    
    // Helper functions
    void _dumpPrimaryTable();
    void _dumpSecondaryTable();
    void _dumpSlot(const char *slotName, const char *eventName, const Event event);

public:
    
    /* Schedules a new event with an absolute time stamp
     * The time stamp unit is "master clock cycles"
     */
    void scheduleAbs(EventSlot s, Cycle cycle, EventID id);

    /* Schedules a new event with a relative time stamp
     * The time stamp is given in master clock cycles and relative to the
     * current value of the DMA clock.
     */
    void scheduleRel(EventSlot s, Cycle cycle, EventID id);

    // Schedules an event at the specified beam position.
    void schedulePos(EventSlot s, int16_t vpos, int16_t hpos, EventID id);

    /* Reschedules the event in the specified slot
     * The new trigger cycle is computed by adding the specified amout of
     * cycles to the old trigger cycle.
     */
    void reschedule(EventSlot s, Cycle offset);

    /* Reschedules the event in the specified slot with a changed event ID
     */
    void reschedule(EventSlot s, Cycle offset, EventID id);

    
    /* Disables the event in the specified slot
     * The event is kept in the slot, but the trigger time is set to infinity.
     */
    void disable(EventSlot s);

    /* Deletes the event in the specified slot
     */
    void cancel(EventSlot s);

    // Returns true if the specified event slot contains an event ID
    inline bool hasEvent(EventSlot s) {
        assert(isEventSlot(s)); return eventSlot[s].id != 0; }

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
