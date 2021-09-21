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
#include "DDF.h"
#include "DmaDebugger.h"
#include "Scheduler.h"
#include "Frame.h"
#include "Memory.h"

/* Hsync handler action flags
 *
 *       HSYNC_PREDICT_DDF : Forces the hsync handler to recompute the
 *                           display data fetch window.
 *  HSYNC_UPDATE_BPL_TABLE : Forces the hsync handler to update the bitplane
 *                           DMA event table.
 *  HSYNC_UPDATE_DAS_TABLE : Forces the hsync handler to update the disk,
 *                          audio, sprite DMA event table.
 */
static constexpr usize HSYNC_PREDICT_DDF =      0b001;
static constexpr usize HSYNC_UPDATE_BPL_TABLE = 0b010;
static constexpr usize HSYNC_UPDATE_DAS_TABLE = 0b100;

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

    // Current workload
    AgnusStats stats = {};


    //
    // Sub components
    //
    
public:

    Scheduler scheduler = Scheduler(amiga);
    Copper copper = Copper(amiga);
    Blitter blitter = Blitter(amiga);
    DmaDebugger dmaDebugger = DmaDebugger(amiga);


    //
    // Event tables
    //
    
private:
    
    // Lookup tables
    static EventID bplDMA[2][7][HPOS_CNT]; // [Hires][Bitplane][DMA cycle]
    static EventID dasDMA[64][HPOS_CNT];   // [Bits 0 .. 5 of DMACON]

    // Currently scheduled events
    EventID bplEvent[HPOS_CNT];
    EventID dasEvent[HPOS_CNT];

    // Jump tables connecting the scheduled events
    u8 nextBplEvent[HPOS_CNT];
    u8 nextDasEvent[HPOS_CNT];
    

    //
    // Execution control
    //

public:
    
    // Action flags controlling the HSYNC handler
    usize hsyncActions;

    // Pending register changes (used for emulating register delays)
    RegChangeRecorder<8> changeRecorder;

    
    //
    // Counters
    //
    
    // Agnus has been emulated up to this master clock cycle
    Cycle clock;

    // The current beam position
    Beam pos;
    
    // Latched beam position (recorded when BPLCON0::ERSY is set)
    Beam latchedPos;
    
    // Information about the current frame
    Frame frame;

    
    //
    // Registers
    //

    // Memory mask (determines the width of all DMA memory pointer registers)
    u32 ptrMask;
    
    // A copy of BPLCON0 and BPLCON1 (Denise has its own copies)
    u16 bplcon0;
    u16 bplcon1;
    
    // The DMA control register
    u16 dmacon;

    // The disk DMA pointer
    u32 dskpt;

    // The audio DMA pointers and pointer latches
    u32 audpt[4];
    u32 audlc[4];

    // The bitplane DMA pointers
    u32 bplpt[6];

    // The bitplane modulo registers for odd bitplanes
    i16 bpl1mod;

    // The bitplane modulo registers for even bitplanes
    i16 bpl2mod;

    // The sprite DMA pointers
    u32 sprpt[8];


    //
    // Derived values
    //
    
    /* Values of BPLCON0 and DMACON at the DDFSTRT trigger cycle. Both
     * variables are set at the beginning of each rasterline and updated
     * on-the-fly if BPLCON0 or DMACON changes before the trigger conditions
     * has been reached.
     */
    u16 bplcon0AtDDFStrt;
    u16 dmaconAtDDFStrt;
    
    /* This value is updated in the hsync handler with the lowest 6 bits of
     * dmacon if the master enable bit is 1 or set to 0 if the master enable
     * bit is 0. It is used as an offset into the DAS lookup tables.
     */
    u16 dmaDAS;

    /* Horizontal shift values derived from BPLCON1. All four values are
     * extracted in setBPLCON1() and utilized to emulate horizontal scrolling.
     * They control at which DMA cycles the BPLDAT registers are transfered
     * into the shift registers.
     */
    i8 scrollLoresOdd;
    i8 scrollLoresEven;
    i8 scrollHiresOdd;
    i8 scrollHiresEven;

    // Set in the hsync handler to remember the returned value of inBplDmaLine()
    bool bplDmaLine;

    
    //
    // Data bus
    //

public:
    
    // Recorded DMA values for all cycles in the current rasterline
    u16 busValue[HPOS_CNT];

    // Recorded DMA usage for all cycles in the current rasterline
    BusOwner busOwner[HPOS_CNT];

    
    //
    // Signals from other components
    //
    
private:

    // DMA requests from Paula
    bool audxDR[4];
    bool audxDSR[4];
    
    /* Blitter slow down. The BLS signal indicates that the CPU's request to
     * access the bus has been denied for three or more consecutive cycles.
     */
    bool bls;


    //
    // Display Data Fetch (DDF)
    //

public:

    // The display data fetch registers
    u16 ddfstrt;
    u16 ddfstop;

    /* At the end of a rasterline, these variables conain the DMA cycles
     * where the hpos counter matched ddfstrt or ddfstop, respectively. A
     * value of -1 indicates that no matching event took place.
     */
    isize ddfstrtReached;
    isize ddfstopReached;

    /* At the end of a rasterline, this variable contains the DDF state.
     */
    DDFState ddfState;

    /* This variable is used to emulate the OCS "scanline effect". If DDFSTRT
     * is set to a value smaller than the left hardware stop at 0x18, early DMA
     * access is enabled every other line. In this case, this variable stores
     * the number of the next line where early DMA is possible.
     */
    isize ocsEarlyAccessLine;

    // DDF flipflops
    bool ddfVFlop;

    // Display data fetch window in lores and hires mode
    DDF<false> ddfLores;
    DDF<true> ddfHires;
    
    
    //
    // Display Window (DIW)
    //

    /* The Amiga limits the visible screen area by an upper, a lower, a left,
     * and a right border. The border encloses an area called the Display
     * Window (DIW). The color of the pixels inside the display window depends
     * on the bitplane data. The pixels of the border area are always drawn in
     * the background color (which might change inside the border area).
     * The size of the display window is controlled by two registers called
     * DIWSTRT and DIWSTOP. They contain the vertical and horizontal positions
     * at which the window starts and stops. The resolution of vertical start
     * and stop is one scan line. The resolution of horizontal start and stop
     * is one low-resolution pixel.
     *
     * I haven't found detailed information about the how the DIW logic is
     * implemented in hardware inside Agnus. If you have such information,
     * please let me know. For the time being, I base my implementation on the
     * following assumptions:
     *
     * 1. Denise contains a single flipflop controlling the display window
     *    horizontally. The flop is cleared inside the border area and set
     *    inside the display area.
     * 2. When hpos matches the position in DIWSTRT, the flipflop is set.
     * 3. When hpos matches the position in DIWSTOP, the flipflop is reset.
     * 4. The smallest valid value for DIWSTRT is $02. If it is smaller, it is
     *    not recognised.
     * 5. The largest valid value for DIWSTOP is $(1)C7. If it is larger, it is
     *    not recognised.
     */

    // Register values as they have been written by pokeDIWSTRT/STOP()
    u16 diwstrt;
    u16 diwstop;

    /* Extracted display window coordinates
     *
     * The coordinates are computed out of diwstrt and diwstop and set in
     * pokeDIWSTRT/STOP(). The following horizontal values are possible:
     *
     *    diwHstrt : $02  ... $FF   or -1
     *    diwHstop : $100 ... $1C7  or -1
     *
     * A -1 is assigned if DIWSTRT or DIWSTOP are written with values that
     * result in coordinates outside the valid range.
     */
    isize diwHstrt;
    isize diwHstop;
    isize diwVstrt;
    isize diwVstop;

    /* Value of the DIW flipflops. Variable diwVFlop stores the value of the
     * vertical DIW flipflop. The value is updated at the beginning of each
     * rasterline and cannot change thereafter. Variable diwHFlop stores the
     * value of the horizontal DIW flipflop as it was at the beginning of the
     * rasterline. To find out the value of the horizontal flipflop inside or
     * at the end of a rasterline, hFlopOn and hFlopOff need to be evaluated.
     */
    bool diwVFlop;
    bool diwHFlop;

    /* At the end of a rasterline, these variable conains the pixel coordinates
     * where the hpos counter matched diwHstrt or diwHstop, respectively. A
     * value of -1 indicates that no matching event took place.
     */
    isize diwHFlopOn;
    isize diwHFlopOff;


    //
    // Sprites
    //

    /* The vertical trigger positions of all 8 sprites. Note that Agnus knows
     * nothing about the horizontal trigger positions (only Denise does).
     */
    isize sprVStrt[8];
    isize sprVStop[8];

    // The current DMA states of all 8 sprites
    SprDMAState sprDmaState[8];


    //
    // Initializing
    //
    
public:
    
    Agnus(Amiga& ref);
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "Agnus"; }
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;
    void _inspect() const override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.revision
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
        
        << bplEvent
        << dasEvent
        << nextBplEvent
        << nextDasEvent

        << hsyncActions
        >> changeRecorder

        >> pos
        >> latchedPos
        >> frame

        << bplcon0
        << bplcon1
        << dmacon
        << dskpt
        << audpt
        << audlc
        << bplpt
        << bpl1mod
        << bpl2mod
        << sprpt

        << bplcon0AtDDFStrt
        << dmaconAtDDFStrt
        << dmaDAS
        << scrollLoresOdd
        << scrollLoresEven
        << scrollHiresOdd
        << scrollHiresEven
        << bplDmaLine
        
        << busValue
        << busOwner

        << audxDR
        << audxDSR
        << bls

        << ddfstrt
        << ddfstop
        << ddfstrtReached
        << ddfstopReached
        << ddfState
        << ocsEarlyAccessLine
        << ddfVFlop
        >> ddfLores
        >> ddfHires

        << diwstrt
        << diwstop
        << diwHstrt
        << diwHstop
        << diwVstrt
        << diwVstop
        << diwVFlop
        << diwHFlop
        << diwHFlopOn
        << diwHFlopOff

        << sprVStrt
        << sprVStop
        << sprDmaState;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Configuring
    //
    
public:
    
    static AgnusConfig getDefaultConfig();
    const AgnusConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    
    
    //
    // Querying chip properties
    //
    
    bool isOCS() const;
    bool isECS() const;
    
    // Returns the chip identification bits of this Agnus (show up in VPOSR)
    u16 idBits();
    
    // Returns the maximum amout of Chip Ram in KB this Agnus can handle
    isize chipRamLimit();
        
    // Returns the line in which the VERTB interrupt is triggered
    isize vStrobeLine() { return config.revision == AGNUS_OCS_DIP ? 1 : 0; }
    
    // Returns a bitmask indicating the used bits in DDFSTRT / DDFSTOP
    u16 ddfMask() { return isOCS() ? 0xFC : 0xFE; }
    
    // Checks whether Agnus is able to access Slow Ram
    bool slowRamIsMirroredIn();
        
    
    //
    // Analyzing
    //
    
public:
    
    AgnusInfo getInfo() const { return AmigaComponent::getInfo(info); }
    const AgnusStats &getStats() { return stats; }
    
private:
    
    void clearStats() { stats = { }; }
    void updateStats();
    

    //
    // Examining the current frame
    //

public:

    /* Returns the number of master cycles in the current frame. The result
     * depends on the number of lines that are drawn. This values varies
     * between long and short frames.
     */
    Cycle cyclesInFrame() const;

    /* Returns the master cycle belonging to beam position (0,0). The first
     * function treats (0,0) as the upper left position of the current frame.
     * The second function referes to the next frame.
     */
    Cycle startOfFrame() const;
    Cycle startOfNextFrame() const;

    // Indicates if the provided master cycle belongs to a specific frame.
    bool belongsToPreviousFrame(Cycle cycle) const;
    bool belongsToCurrentFrame(Cycle cycle) const;
    bool belongsToNextFrame(Cycle cycle) const;


    //
    // Examining the current rasterline
    //

public:

    // Indicates if the electron beam is inside the VBLANK area
    bool inVBlankArea(isize posv) const { return posv < 26; }
    bool inVBlankArea() const { return inVBlankArea(pos.v); }

    // Indicates if the current rasterline is the last line in this frame
    bool inLastRasterline(isize posv) const { return posv == frame.lastLine(); }
    bool inLastRasterline() const { return inLastRasterline(pos.v); }

    // Indicates if bitplane DMA is enabled in the current rasterline
    bool inBplDmaLine(u16 dmacon, u16 bplcon0) const;
    bool inBplDmaLine() const { return inBplDmaLine(dmacon, bplcon0); }

    // Returns the pixel position for the current horizontal position
    Pixel ppos(isize posh) const { return (posh * 4) + 2; }
    Pixel ppos() const { return ppos(pos.h); }

    
    //
    // Working with the beam position
    //

public:

    /* Translates a beam position to a master cycle. The beam position must be
     * a position inside the current frame.
     */
    Cycle beamToCycle(Beam beam) const;

    /* Translates a master cycle to a beam position. The beam position must
     * belong to the current frame.
     */
    Beam cycleToBeam(Cycle cycle) const;

    // Advances a beam position by a given number of cycles (DEPRECATED)
    // TODO: REMOVE AFTER V1.1 BETA TESTING
    Beam addToBeam(Beam beam, Cycle cycles) const;

    
    //
    // Querying graphic modes
    //
    
public:

    /* Returns the Agnus view of the BPU bits. The value determines the number
     * of enabled DMA channels. It is computed out of the three BPU bits stored
     * in BPLCON0, but not identical with them. The value differs if the BPU
     * bits reflect an invalid bit pattern. Compare with Denise::bpu() which
     * returns the Denise view of the BPU bits.
     */
    static u8 bpu(u16 v);
    u8 bpu() const { return bpu(bplcon0); }

    // Checks whether Hires or Lores mode is selected
    bool hires() { return GET_BIT(bplcon0, 15); }
    bool lores() { return GET_BIT(bplcon0, 10); }
    
    // Returns the external synchronization bit from BPLCON0
    bool ersy() { return GET_BIT(bplcon0, 1); }


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
    void recordRegisterChange(Cycle delay, u32 addr, u16 value);

private:

    // Executes the first sprite DMA cycle
    template <isize nr> void executeFirstSpriteCycle();

    // Executes the second sprite DMA cycle
    template <isize nr> void executeSecondSpriteCycle();

    // Updates the sprite DMA status in cycle 0xDF
    void updateSpriteDMA();

    // Finishes up the current rasterline
    void hsyncHandler();

    // Finishes up the current frame
    void vsyncHandler();

    
    //
    // Managing the data fetch window (AgnusDDF.cpp)
    //
    
    // Sets up the likely DDF values for the next rasterline
    void predictDDF();

private:

    void computeDDFWindow();
    void computeDDFWindowOCS();
    void computeDDFWindowECS();

    
    //
    // Controlling DMA (AgnusDma.cpp)
    //

private:
    
    // Initializes the static lookup tables
    void initBplEventTableLores();
    void initBplEventTableHires();
    void initDasEventTable();

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
    
private:
    
    void enableBplDmaOCS();
    void disableBplDmaOCS();
    void enableBplDmaECS();
    void disableBplDmaECS();

    
    //
    // Managing the DMA time slot tables (AgnusDma.cpp)
    //
    
public:

    // Removes all events from the BPL event table
    void clearBplEvents();

    // Renews all events in the BPL event table
    void updateBplEvents(u16 dmacon, u16 bplcon0, isize first = 0);
    void updateBplEvents(isize first = 0) { updateBplEvents(dmacon, bplcon0, first); }
        
    // Renews all events in the the DAS event table
    void updateDasEvents(u16 dmacon);

private:

    // Updates the jump table for the bplEvent table
    void updateBplJumpTable();

    // Updates the jump table for the dasEvent table
    void updateDasJumpTable(i16 end = HPOS_MAX);
    
    // Updates the drawing flags in the bplEvent table
    void updateHiresDrawingFlags();
    void updateLoresDrawingFlags();

    
    //
    // Performing DMA (AgnusDma.cpp)
    //

public:
        
    // Checks if the bus is currently available for the specified resource
    template <BusOwner owner> bool busIsFree() const;

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

    u16 peekDMACONR();
    void pokeDMACON(u16 value);
    void setDMACON(u16 oldValue, u16 newValue);
    
    u16 peekVHPOSR();
    void pokeVHPOS(u16 value);
    void setVHPOS(u16 value);

    u16 peekVPOSR();
    void pokeVPOS(u16 value);
    void setVPOS(u16 value);

    void pokeDSKPTH(u16 value);
    void pokeDSKPTL(u16 value);

    void pokeBPLCON0(u16 value);
    void setBPLCON0(u16 oldValue, u16 newValue);

    void pokeBPLCON1(u16 value);
    void setBPLCON1(u16 oldValue, u16 newValue);

    template <Accessor s> void pokeDIWSTRT(u16 value);
    void setDIWSTRT(u16 value);

    template <Accessor s> void pokeDIWSTOP(u16 value);
    void setDIWSTOP(u16 value);

    void pokeDDFSTRT(u16 value);
    void setDDFSTRT(u16 old, u16 value);

    void pokeDDFSTOP(u16 value);
    void setDDFSTOP(u16 old, u16 value);

    template <int x> void pokeAUDxLCH(u16 value);
    template <int x> void pokeAUDxLCL(u16 value);
    template <int x> void reloadAUDxPT() { audpt[x] = audlc[x]; }

    template <int x> void pokeBPLxPTH(u16 value);
    template <int x> void setBPLxPTH(u16 value);

    template <int x> void pokeBPLxPTL(u16 value);
    template <int x> void setBPLxPTL(u16 value);

    void pokeBPL1MOD(u16 value);
    void setBPL1MOD(u16 value);

    void pokeBPL2MOD(u16 value);
    void setBPL2MOD(u16 value);

    template <int x> void pokeSPRxPTH(u16 value);
    template <int x> void setSPRxPTH(u16 value);
    
    template <int x> void pokeSPRxPTL(u16 value);
    template <int x> void setSPRxPTL(u16 value);
    
    template <int x> void pokeSPRxPOS(u16 value);
    template <int x> void pokeSPRxCTL(u16 value);

private:
    
    // Checks whether a write to a pointer register should be dropped
    bool dropWrite(BusOwner owner);


    //
    // Scheduling events (AgnusEvents.cpp)
    //
    
public:
    
    template<EventSlot s> void scheduleRel(Cycle cycle, EventID id) {
        scheduler.scheduleAbs<s>(clock + cycle, id);
    }
    
    template<EventSlot s> void scheduleRel(Cycle cycle, EventID id, i64 data) {
        scheduler.scheduleAbs<s>(clock + cycle, id, data);
    }
    
    template<EventSlot s> void schedulePos(Beam pos, EventID id, i64 data) {
        scheduler.scheduleAbs<s>(beamToCycle(pos), id, data);
    }

    template<EventSlot s> void schedulePos(isize vpos, isize hpos, EventID id) {
        scheduler.scheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ), id);
    }

    template<EventSlot s> void schedulePos(isize vpos, isize hpos, EventID id, i64 data) {
        scheduler.scheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ), id, data);
    }
    
    template<EventSlot s> void rescheduleRel(Cycle cycle) {
        scheduler.rescheduleAbs<s>(clock + cycle);
    }

    template<EventSlot s> void reschedulePos(Beam pos) {
        scheduler.rescheduleAbs<s>(beamToCycle(pos));
    }

    template<EventSlot s> void reschedulePos(i16 vpos, i16 hpos) {
        scheduler.rescheduleAbs<s>(beamToCycle( Beam { vpos, hpos } ));
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

    // Services a raster event
    void serviceRASEvent();

    // Services a bitplane event
    void serviceBPLEvent(EventID id);
    template <isize nr> void serviceBPLEventHires();
    template <isize nr> void serviceBPLEventLores();

    // Services a vertical blank interrupt
    void serviceVblEvent(EventID id);
    
    // Services a Disk, Audio, or Sprite event
    void serviceDASEvent(EventID id);
    
    // Services an inspection event
    void serviceINSEvent(EventID id);
};
