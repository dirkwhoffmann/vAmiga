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
#include "Beam.h"
#include "Blitter.h"
#include "ChangeRecorder.h"
#include "Copper.h"
#include "DmaDebugger.h"
#include "Sequencer.h"
#include "Memory.h"

namespace vamiga {

/* Bitplane event modifiers
 *
 *                DRAW_ODD : Starts the shift registers of the odd bitplanes
 *                           to generate pixels.
 *               DRAW_EVEN : Starts the shift registers of the even bitplanes
 *                           to generate pixels.
 */
static constexpr usize DRAW_ODD =  0b001;
static constexpr usize DRAW_EVEN = 0b010;
static constexpr usize DRAW_BOTH = 0b011;

class Agnus : public SubComponent {

    // Current configuration
    AgnusConfig config = {};

    // Result of the latest inspection
    mutable AgnusInfo info = {};
    mutable EventInfo eventInfo = {};
    mutable EventSlotInfo slotInfo[SLOT_COUNT];

    // Current workload
    AgnusStats stats = {};


    //
    // Sub components
    //
    
public:

    Sequencer sequencer = Sequencer(amiga);
    Copper copper = Copper(amiga);
    Blitter blitter = Blitter(amiga);
    DmaDebugger dmaDebugger = DmaDebugger(amiga);


    //
    // Event scheduler
    //

public:
    
    // Trigger cycle
    Cycle trigger[SLOT_COUNT] = { };

    // The event identifier
    EventID id[SLOT_COUNT] = { };

    // An optional data value
    i64 data[SLOT_COUNT] = { };
    
    // Next trigger cycle
    Cycle nextTrigger = NEVER;
    
    // Pending register changes
    RegChangeRecorder<8> changeRecorder;

    // An optional sync event to be processed in serviceRegEvent()
    EventID syncEvent = EVENT_NONE;

    
    //
    // Counters
    //

    // Agnus has been emulated up to this master clock cycle
    Cycle clock = 0;

    // The current beam position
    Beam pos = { };

    // Latched beam position (recorded when BPLCON0::ERSY is set)
    Beam latchedPos = { };

    
    //
    // Registers
    //

    // Memory mask (determines the width of all DMA memory pointer registers)
    u32 ptrMask = 0;
    
    // A copy of BPLCON0 and BPLCON1 (Denise has its own copies)
    u16 bplcon0 = 0;
    u16 bplcon0Initial = 0;
    u16 bplcon1 = 0;
    u16 bplcon1Initial = 0;

    // The DMA control register
    u16 dmacon = 0;
    u16 dmaconInitial = 0;
    
    // The disk DMA pointer
    u32 dskpt = 0;

    // The audio DMA pointers and pointer latches
    u32 audpt[4] = { };
    u32 audlc[4] = { };

    // The bitplane DMA pointers
    u32 bplpt[6] = { };

    // The bitplane modulo registers for odd bitplanes
    i16 bpl1mod = 0;

    // The bitplane modulo registers for even bitplanes
    i16 bpl2mod = 0;

    // The sprite DMA pointers
    u32 sprpt[8] = { };


    //
    // Derived values
    //

    // Bitplane resolution (derived from bplcon0)
    Resolution res = 0;

    // Bitplane offsets (derived from bplcon1)
    i8 scrollOdd = 0;
    i8 scrollEven = 0;
    

    //
    // Data bus
    //

public:
    
    // Recorded DMA values for all cycles in the current rasterline
    u16 busValue[HPOS_CNT] = { };

    // Recorded DMA usage for all cycles in the current rasterline
    BusOwner busOwner[HPOS_CNT] = { };

    
    //
    // Signals from other components
    //
    
private:

    // DMA requests from Paula
    bool audxDR[4] = { };
    bool audxDSR[4] = { };
    
    /* Blitter slow down. The BLS signal indicates that the CPU's request to
     * access the bus has been denied for three or more consecutive cycles.
     */
    bool bls = false;


    //
    // Sprites
    //

public:
    
    /* The vertical trigger positions of all 8 sprites. Note that Agnus knows
     * nothing about the horizontal trigger positions (only Denise does).
     */
    isize sprVStrt[8] = { };
    isize sprVStop[8] = { };

    // The current DMA states of all 8 sprites
    SprDMAState sprDmaState[8] = { };

    
    //
    // Initializing
    //
    
public:
    
    Agnus(Amiga& ref);
    
    
    //
    // Class methods
    //
    
    static const char *eventName(EventSlot slot, EventID id);

    
    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override { return "Agnus"; }
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _reset(bool hard) override;
    void _inspect() const override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.revision
        << config.slowRamMirror
        << ptrMask;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker

            << clock;
        }

        worker
        
        << trigger
        << id
        << data
        << nextTrigger
        >> changeRecorder
        << syncEvent
        
        >> pos
        >> latchedPos
        
        << bplcon0
        << bplcon0Initial
        << bplcon1
        << bplcon1Initial
        << dmacon
        << dmaconInitial
        << dskpt
        << audpt
        << audlc
        << bplpt
        << bpl1mod
        << bpl2mod
        << sprpt
        << res
        << scrollOdd
        << scrollEven
        
        << busValue
        << busOwner

        << audxDR
        << audxDSR
        << bls

        << sprVStrt
        << sprVStop
        << sprDmaState;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Configuring
    //
    
public:
    
    const AgnusConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);

    void setVideoFormat(VideoFormat newFormat);


    //
    // Querying chip properties
    //

public:

    bool isOCS() const;
    bool isECS() const;
    bool isPAL() const { return pos.type == PAL; }
    bool isNTSC() const { return !isPAL(); }

    // Returns the chip identification bits of this Agnus (show up in VPOSR)
    u16 idBits() const;
    
    // Returns the maximum amout of Chip Ram in KB this Agnus can handle
    isize chipRamLimit() const;

    // Returns the line in which the VERTB interrupt is triggered
    isize vStrobeLine() const { return config.revision == AGNUS_OCS_OLD ? 1 : 0; }
    
    // Returns a bitmask indicating the used bits in DDFSTRT / DDFSTOP
    u16 ddfMask() const { return isOCS() ? 0xFC : 0xFE; }
    
    // Checks whether Agnus is able to access Slow Ram
    bool slowRamIsMirroredIn() const;

    
    //
    // Analyzing
    //
    
public:
    
    AgnusInfo getInfo() const { return CoreComponent::getInfo(info); }
    EventInfo getEventInfo() const { return CoreComponent::getInfo(eventInfo); }
    EventSlotInfo getSlotInfo(isize nr) const;
    const AgnusStats &getStats() { return stats; }
    
private:
    
    void inspectSlot(EventSlot nr) const;
    void clearStats();
    void updateStats();


    //
    // Examining the current rasterline
    //

public:

    // Indicates if the electron beam is inside the VBLANK area
    bool inVBlankArea(isize posv) const { return posv < 26; }
    bool inVBlankArea() const { return inVBlankArea(pos.v); }

    // Indicates if the current rasterline is the last line in this frame
    bool inLastRasterline(isize posv) const { return posv == pos.vMax(); }
    bool inLastRasterline() const { return inLastRasterline(pos.v); }


    //
    // Querying graphic modes
    //
    
public:

    // Computes the bitmap resolution from a given BPLCON0 value
    Resolution resolution(u16 v);

    // Queries the currently set bitmap resolution
    bool lores() { return res == LORES; }
    bool hires() { return res == HIRES; }
    bool shres() { return res == SHRES; }

    // Returns the external synchronization bit from BPLCON0
    static bool ersy(u16 value) { return GET_BIT(value, 1); }
    bool ersy() { return ersy(bplcon0); }


    //
    // Operating the device
    //
    
public:

    // Executes Agnus for a single cycle
    void execute();

    // Executes Agnus for a certain amount of cycles
    void execute(DMACycle cycles);
    
    // Executes Agnus to the beginning of the next E clock cycle
    void syncWithEClock();

    // Executes Agnus until the CPU can acquire the bus
    void executeUntilBusIsFree();
    void executeUntilBusIsFreeForCIA();

    // Schedules a register to change its value
    void recordRegisterChange(Cycle delay, u32 addr, u16 value, Accessor acc = 0);

private:

    // Processes all events up to a given master cycle
    void executeUntil(Cycle cycle);

    // Executes the first sprite DMA cycle
    template <isize nr> void executeFirstSpriteCycle();

    // Executes the second sprite DMA cycle
    template <isize nr> void executeSecondSpriteCycle();

    // Updates the sprite DMA status in cycle 0xDF
    void updateSpriteDMA();

    // Finishes up the current line
    void eolHandler();

    // Finishes up the current frame
    void eofHandler();

    // Called at the beginning of the HSYNC area
    void hsyncHandler();

    // Called at the beginning of the VSYNC area
    void vsyncHandler();


    //
    // Controlling DMA (AgnusDma.cpp)
    //

public:
    
    // Returns true if the Blitter has priority over the CPU
    static bool bltpri(u16 value) { return GET_BIT(value, 10); }
    bool bltpri() const { return bltpri(dmacon); }

    // Returns true if a certain DMA channel is enabled
    template <int x> static bool auddma(u16 v);
    template <int x> bool auddma() const { return auddma<x>(dmacon); }

    static bool bpldma(u16 v) { return (v & DMAEN) && (v & BPLEN); }
    static bool copdma(u16 v) { return (v & DMAEN) && (v & COPEN); }
    static bool bltdma(u16 v) { return (v & DMAEN) && (v & BLTEN); }
    static bool sprdma(u16 v) { return (v & DMAEN) && (v & SPREN); }
    static bool dskdma(u16 v) { return (v & DMAEN) && (v & DSKEN); }
    bool bpldma() const { return bpldma(dmacon); }
    bool copdma() const { return copdma(dmacon); }
    bool bltdma() const { return bltdma(dmacon); }
    bool sprdma() const { return sprdma(dmacon); }
    bool dskdma() const { return dskdma(dmacon); }
    

    //
    // Performing DMA (AgnusDma.cpp)
    //

public:

    // Checks if the bus is currently available for the specified resource
    template <BusOwner owner> bool busIsFree();

    // Attempts to allocate the bus for the specified resource
    template <BusOwner owner> bool allocateBus();

    // Performs a DMA read
    u16 doDiskDmaRead();
    template <int channel> u16 doAudioDmaRead();
    template <int channel> u16 doBitplaneDmaRead();
    template <int channel> u16 doSpriteDmaRead();
    u16 doCopperDmaRead(u32 addr);
    u16 doBlitterDmaRead(u32 addr);

    // Performs a DMA write
    void doDiskDmaWrite(u16 value);
    void doCopperDmaWrite(u32 addr, u16 value);
    void doBlitterDmaWrite(u32 addr, u16 value);

    // Transmits a DMA request from Agnus to Paula
    template <int channel> void setAudxDR() { audxDR[channel] = true; }
    template <int channel> void setAudxDSR() { audxDSR[channel] = true; }

    // Getter and setter for the BLS signal (Blitter slow down)
    bool getBLS() { return bls; }
    void setBLS(bool value) { bls = value; }


    //
    // Accessing registers (AgnusRegisters.cpp)
    //
    
public:

    u16 peekDMACONR() const;
    template <Accessor s> void pokeDMACON(u16 value);
    void setDMACON(u16 oldValue, u16 newValue);
    void setBPLEN(bool value);
    void setCOPEN(bool value);
    void setBLTEN(bool value);
    void setSPREN(bool value);
    void setDSKEN(bool value);
    void setAUD0EN(bool value);
    void setAUD1EN(bool value);
    void setAUD2EN(bool value);
    void setAUD3EN(bool value);

    u16 peekVHPOSR() const;
    void pokeVHPOS(u16 value);
    void setVHPOS(u16 value);

    u16 peekVPOSR() const;
    void pokeVPOS(u16 value);
    void setVPOS(u16 value);

    template <Accessor s> void pokeBPLCON0(u16 value);
    void setBPLCON0(u16 oldValue, u16 newValue);

    void pokeBPLCON1(u16 value);
    void setBPLCON1(u16 oldValue, u16 newValue);

    template <Accessor s> void pokeDIWSTRT(u16 value);
    template <Accessor s> void pokeDIWSTOP(u16 value);
    template <Accessor s> void pokeDIWHIGH(u16 value);

    void pokeBPL1MOD(u16 value);
    void setBPL1MOD(u16 value);

    void pokeBPL2MOD(u16 value);
    void setBPL2MOD(u16 value);
    
    template <int x> void pokeSPRxPOS(u16 value);
    template <int x> void pokeSPRxCTL(u16 value);

    void pokeBEAMCON0(u16 value);

    
    //
    // Accessing DMA pointer registers (AgnusRegisters.cpp)
    //
    
public:
    
    template <Accessor s> void pokeDSKPTH(u16 value);
    void setDSKPTH(u16 value);

    template <Accessor s> void pokeDSKPTL(u16 value);
    void setDSKPTL(u16 value);

    template <int x, Accessor s> void pokeAUDxLCH(u16 value);
    template <int x, Accessor s> void pokeAUDxLCL(u16 value);
    template <int x> void reloadAUDxPT() { audpt[x] = audlc[x]; }

    template <int x, Accessor s> void pokeBPLxPTH(u16 value);
    template <int x> void setBPLxPTH(u16 value);

    template <int x, Accessor s> void pokeBPLxPTL(u16 value);
    template <int x> void setBPLxPTL(u16 value);

    template <int x, Accessor s> void pokeSPRxPTH(u16 value);
    template <int x> void setSPRxPTH(u16 value);

    template <int x, Accessor s> void pokeSPRxPTL(u16 value);
    template <int x> void setSPRxPTL(u16 value);

private:
    
    // Checks whether a write to a pointer register should be dropped
    bool dropWrite(BusOwner owner);


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
    template<EventSlot s> void scheduleRel(Cycle cycle, EventID id) {
        scheduleAbs<s>(clock + cycle, id);
    }

    template<EventSlot s> void scheduleRel(Cycle cycle, EventID id, i64 data) {
        scheduleAbs<s>(clock + cycle, id, data);
    }

    template<EventSlot s> void schedulePos(isize vpos, isize hpos, EventID id) {

        assert(vpos > pos.v || (vpos == pos.v && hpos >= pos.h));
        scheduleRel<s>(DMA_CYCLES(pos.diff(vpos, hpos)), id);
    }

    template<EventSlot s> void schedulePos(isize vpos, isize hpos, EventID id, i64 data) {

        assert(vpos > pos.v || (vpos == pos.v && hpos >= pos.h));
        scheduleRel<s>(DMA_CYCLES(pos.diff(vpos, hpos)), id, data);
    }
    
    template<EventSlot s> void rescheduleRel(Cycle cycle) {
        rescheduleAbs<s>(clock + cycle);
    }

    template<EventSlot s> void reschedulePos(i16 vpos, i16 hpos) {

        assert(vpos > pos.v || (vpos == pos.v && hpos >= pos.h));
        rescheduleRel<s>(DMA_CYCLES(pos.diff(vpos, hpos)));
    }

    template<EventSlot s> void cancel()
    {
        id[s] = (EventID)0;
        data[s] = 0;
        trigger[s] = NEVER;
    }

    
    //
    // Scheduling specific events (AgnusEvents.cpp)
    //

public:

    // Schedules the first BPL event
    void scheduleFirstBplEvent();

    // Schedules the next BPL event relative to a given DMA cycle
    void scheduleNextBplEvent(isize hpos);

    // Schedules the next BPL event relative to the currently emulated DMA cycle
    void scheduleNextBplEvent() { scheduleNextBplEvent(pos.h); }

    // Schedules the earliest BPL event that occurs at or after the given DMA cycle
    void scheduleBplEventForCycle(isize hpos);

    // Updates the scheduled BPL event according to the current event table
    void updateBplEvent() { scheduleBplEventForCycle(pos.h); }

    // Schedules the first BPL event
    void scheduleFirstDasEvent();

    // Schedules the next DAS event relative to a given DMA cycle
    void scheduleNextDasEvent(isize hpos);

    // Schedules the next DAS event relative to the currently emulated DMA cycle
    void scheduleNextDasEvent() { scheduleNextDasEvent(pos.h); }

    // Schedules the earliest DAS event that occurs at or after the given DMA cycle
    void scheduleDasEventForCycle(isize hpos);

    // Updates the scheduled DAS event according to the current event table
    void updateDasEvent() { scheduleDasEventForCycle(pos.h); }

    // Schedules the next register change event
    void scheduleNextREGEvent();

    // Schedules a strobe event in the VBL slot
    void scheduleStrobe0Event();
    void scheduleStrobe1Event();
    void scheduleStrobe2Event();


    //
    // Servicing events (AgnusEvents.cpp)
    //

public:

    // Services a register change event
    void serviceREGEvent(Cycle until);

    // Services a bitplane event
    void serviceBPLEvent(EventID id);
    template <isize nr> void serviceBPLEventLores();
    template <isize nr> void serviceBPLEventHires();
    template <isize nr> void serviceBPLEventShres();

    // Services a vertical blank interrupt
    void serviceVBLEvent(EventID id);

    // Renews the trigger cycle of a pending VBL event
    void rectifyVBLEvent();

    // Services a Disk, Audio, or Sprite event
    void serviceDASEvent(EventID id);
    
    // Services an inspection event
    void serviceINSEvent(EventID id);
};

}
