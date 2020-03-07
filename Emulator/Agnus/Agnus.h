// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AGNUS_INC
#define _AGNUS_INC

#include "AmigaConstants.h"
#include "Beam.h"
#include "Blitter.h"
#include "Copper.h"
#include "DmaDebugger.h"
#include "Event.h"
#include "HardwareComponent.h"
#include "Memory.h"

// Hsync handler action flags
#define HSYNC_PREDICT_DDF        0b001
#define HSYNC_UPDATE_BPL_TABLE   0b010
#define HSYNC_UPDATE_DAS_TABLE   0b100

class Agnus : public AmigaComponent {
    
    // Current configuration
    AgnusConfig config;

    // Result of the latest inspection
    AgnusInfo info;
    EventInfo eventInfo;

    // Collected statistical information
    AgnusStats stats;


    //
    // Sub components
    //
    
public:

    Copper copper = Copper(amiga);
    Blitter blitter = Blitter(amiga);
    DmaDebugger dmaDebugger = DmaDebugger(amiga);


    //
    // Static lookup tables
    //

    /* Bitplane DMA events as they appear in a single rasterline.
     *
     * Parameters: bitplaneDMA[Resolution][Bitplanes][Cycle]
     *
     *             Resolution : 0 or 1        (0 = LORES / 1 = HIRES)
     *              Bitplanes : 0 .. 6        (Bitplanes in use, BPU)
     *                  Cycle : 0 .. HPOS_MAX (DMA cycle)
     *
     * The lookup table is used to quickly update the bplEvent table.
     * Depending on the current resoution and BPU value, a segment of this
     * lookup table is copied into the event table.
     */
    EventID bplDMA[2][7][HPOS_CNT];

    /* Fetch unit cycle numbers.
     *
     * Parameters: fetchUnitNr[Resolution][Cycle]
     *
     *             Resolution : 0 or 1        (0 = LORES / 1 = HIRES)
     *                  Cycle : 0 .. HPOS_MAX (DMA cycle)
     *
     * The lookup table is used to determine the position of a given DMA
     * cycle inside the fetch unit. The first cycle in a fetch unit in numbered
     * 0, the second cycle is numbered 1 and so on.
     */
    u8 fetchUnitNr[2][HPOS_CNT];

    /* Disk, Audio, Sprite DMA events as they appear in a single rasterline.
     *
     * Parameters: dasDMA[dmacon]
     *
     *             dmacon : Bits 0 .. 5 of register DMACON
     *
     * The lookup table is used to quickly update the dasEvent table.
     * Depending on the current resoution and BPU value, a segment of this
     * lookup table is copied into the event table.
     */
    EventID dasDMA[64][HPOS_CNT];


    //
    // Events
    //

public:

    // The event table
    Event slot[SLOT_COUNT];

    // Next trigger cycle for an event in the primary event table
    Cycle nextTrigger = NEVER;


    //
    // Event tables
    //

    /* Agnus utilizes two event tables to schedule DMA events in the DAS_SLOT
     * and BPL_SLOT. Together, both tables resembles Fig. 6-9 im the HRM (3rd
     * rev.). Assuming that sprite DMA is enabled and Denise draws 6 bitplanes
     * in lores mode starting at 0x28, the tables would look like this:
     *
     *     bplEvent[0x00] = EVENT_NONE   dasEvent[0x00] = EVENT_NONE
     *     bplEvent[0x01] = EVENT_NONE   dasEvent[0x01] = BUS_REFRESH
     *         ...                           ...
     *     bplEvent[0x28] = EVENT_NONE   dasEvent[0x28] = EVENT_NONE
     *     bplEvent[0x29] = BPL_L4       dasEvent[0x29] = DAS_S5_1
     *     bplEvent[0x2A] = BPL_L6       dasEvent[0x2A] = EVENT_NONE
     *     bplEvent[0x2B] = BPL_L2       dasEvent[0x2B] = DAS_S5_2
     *     bplEvent[0x2C] = EVENT_NONE   dasEvent[0x2C] = EVENT_NONE
     *     bplEvent[0x2D] = BPL_L3       dasEvent[0x2D] = DAS_S6_1
     *     bplEvent[0x2E] = BPL_L5       dasEvent[0x2E] = EVENT_NONE
     *     bplEvent[0x2F] = BPL_L1       dasEvent[0x2F] = DAS_S6_2
     *         ...                           ...
     *     bplEvent[0xE2] = BPL_EOL      dasEvent[0xE2] = BUS_REFRESH
     *
     * The BPL_EOL event doesn't perform DMA. It concludes the current line.
     */
    EventID bplEvent[HPOS_CNT];
    EventID dasEvent[HPOS_CNT];

    /* Each event table is accompanied by a jump table that points to the
     * next event. Given the example tables above, the jump tables would look
     * like this:
     *
     *     nextBplEvent[0x00] = 0x29     nextDasEvent[0x00] = 0x01
     *     nextBplEvent[0x01] = 0x29     nextDasEvent[0x01] = 0x03
     *           ...                           ...
     *     nextBplEvent[0x28] = 0x29     nextDasEvent[0x28] = 0x29
     *     nextBplEvent[0x29] = 0x2A     nextDasEvent[0x29] = 0x2B
     *     nextBplEvent[0x2A] = 0x2B     nextDasEvent[0x2A] = 0x2B
     *     nextBplEvent[0x2B] = 0x2D     nextDasEvent[0x2B] = 0x2D
     *     nextBplEvent[0x2C] = 0x2D     nextDasEvent[0x2C] = 0x2D
     *     nextBplEvent[0x2D] = 0x2E     nextDasEvent[0x2D] = 0x2F
     *     nextBplEvent[0x2E] = 0x2F     nextDasEvent[0x2E] = 0x2F
     *     nextBplEvent[0x2F] = 0x31     nextDasEvent[0x2F] = 0x31
     *           ...                           ...
     *     nextBplEvent[0xE2] = 0x00     nextDasEvent[0xE2] = 0x00
     *
     * Whenever one the DMA tables is modified, the corresponding jump table
     * has to be updated.
     */
    u8 nextBplEvent[HPOS_CNT];
    u8 nextDasEvent[HPOS_CNT];


    //
    // Operation control
    //

    // Action flags checked in the HSYNC handler
    u64 hsyncActions;


    //
    // Counters
    //

    // Agnus has been emulated up to this clock cycle.
    Cycle clock;
    
    /* The frame counter.
     * The value is increased on every VSYNC action.
     * DEPRECATED: will be replaced by frame.nr
     */
    Frame frame;

    // The current beam position
    Beam pos;

    // Information about the currently drawn frame
    struct {

        // Frame count (will eventually replace variable 'frame')
        Frame nr;

        // Indicates if this frame is drawn in interlace mode
        bool interlaced;

        // The number of rasterlines in the current frame
        i16 numLines;

    } frameInfo;

    // The long frame flipflop
    bool lof;

    /* The vertical trigger positions of all 8 sprites.
     * Note: The horizontal trigger positions are stored inside Denise. Agnus
     * knows nothing about them.
     */
    i16 sprVStrt[8];
    i16 sprVStop[8];

    // The current DMA states of all 8 sprites.
    SprDMAState sprDmaState[8];


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
    i16 diwHstrt;
    i16 diwHstop;
    i16 diwVstrt;
    i16 diwVstop;

    /* Value of the DIW flipflops
     * Variable vFlop stores the value of the vertical DIW flipflop. The value
     * is updated at the beginning of each rasterline and cannot change
     * thereafter. Variable hFlop stores the value of the horizontal DIW
     * flipflop as it was at the beginning of the rasterline. To find out
     * the value of the horizontal flipflop inside or at the end of a
     * rasterline, hFlopOn and hFlopOff need to be evaluated.
     */
    bool diwVFlop;
    bool diwHFlop;

    /* At the end of a rasterline, these variable conains the pixel coordinates
     * where the hpos counter matched diwHstrt or diwHstop, respectively. A
     * value of -1 indicates that no matching event took place.
     */
    i16 diwHFlopOn;
    i16 diwHFlopOff;


    //
    // Display Data Fetch (DDF)
    //

    /* Register DDFSTRT and DDFSTOP define the area where the system performs
     * bitplane DMA. From a hardware engineer's point of view, these registers
     * are completely independent of DIWSTRT and DIWSTOP. From a software
     * engineer's point of view they appear closely related though. To get
     * graphics output right, bitplane DMA has to start closely before the
     * display window opens (left border ends) and to stop closely after the
     * display window closes (right border begins).
     * DDFSTRT and DDFSTOP have a resolution of four lowres pixels (unlike
     * DIWSTRT and DIWSTOP which have a resolution of one lores pixels).
     *
     * I haven't found detailed information about the how the DDF logic is
     * implemented in hardware inside Agnus. If you have such information,
     * please let me know. For the time being, I base my implementation on the
     * following assumptions:
     *
     * 1. The four-pixel resolution is achieved by ignoring the two lower bits
     *    in DDFSTRT and DDFSTOP.
     * 2. The actual DMA start position depends solely on DDFSTRT. In hires
     *    mode, the start position always matches DDFSTRT. In lores mode, it
     *    matches DDFSTRT only if DDFSTRT is dividable by 8. Otherwise, the
     *    value is rounded up to the next position dividable by eight (because
     *    the lower two bits are always 0, this is equivalent to adding 4).
     * 3. The actual DMA stop position depends on both DDFSTRT and DDFSTOP.
     *    Hence, if DDFSTRT changes, the stop position needs to be recomputed
     *    even if DDFSTOP hasn't changed.
     * 4. Agnus switches bitplane DMA on and off by constantly comparing the
     *    horizontal raster position with the DMA start and stop positions that
     *    have been computed out of DDFSTRT and DDFSTOP. Hence, if DDFSTRT
     *    changes before DMA is switched on, the changed values takes effect
     *    immediately (i.e., in the same rasterline). If it changes when DMA is
     *    already on, the change takes effect in the next rasterline.
     * 5. The values written to DDFSTRT and DDFSTOP are not clipped if they
     *    describe a position outside the two hardware stops (at 0x18 and 0xD8).
     *    E.g., if a very small value is written to DDFSTRT, Agnus starts
     *    incrementing the bitplane pointers even if the left hardware stop is
     *    not crossed yet. Agnus simply refused to perform DMA until the
     *    hardware stop has been crossed.
     */

    // The display data fetch registers
    u16 ddfstrt;
    u16 ddfstop;

    /* At the end of a rasterline, these variables conain the DMA cycles
     * where the hpos counter matched ddfstrt or ddfstop, respectively. A
     * value of -1 indicates that no matching event took place.
     */
    i16 ddfstrtReached;
    i16 ddfstopReached;

    /* At the end of a rasterline, this variable contains the DDF state.
     */
    DDFState ddfState;

    /* This variable is used to emulate the OCS "scanline effect". If DDFSTRT
     * is set to value smaller than the left hardware stop at 0x18, early DMA
     * access is enables every other line. In this case, this variable stores
     * the number of the next line where early DMA is possible.
     */
    i16 ocsEarlyAccessLine;

    // DDF flipflops
    bool ddfVFlop;

    // The actual data fetch window
    i16 ddfStrtLores;      // First lores bitplane DMA cycle
    i16 ddfStopLores;      // Last lores bitplane DMA cycle + 1
    i16 ddfStrtHires;      // First hires bitplane DMA cycle
    i16 ddfStopHires;      // Last hires bitplane DMA cycle + 1

    bool inLoresDmaArea(i16 pos) { return pos >= ddfStrtLores && pos < ddfStopLores; }
    bool inHiresDmaArea(i16 pos) { return pos >= ddfStrtHires && pos < ddfStopHires; }

    //
    // Registers
    //

    // Ringbuffer for managing register change delays
    RegChangeRecorder<8> chngRecorder;

    // A copy of BPLCON0 (Denise has another copy)
    u16 bplcon0;

    /* Value of bplcon0 at the DDFSTRT trigger cycle.
     * This variable is set at the beginning of each rasterline and updated
     * on-the-fly when dmacon changes before the trigger conditions has been
     * reached.
     */
    u16 bplcon0AtDDFStrt;

    // The DMA control register
    u16 dmacon;

    /* Value of dmacon at the DDFSTRT trigger cycle.
     * This variable is set at the beginning of each rasterline and updated
     * on-the-fly when dmacon changes before the trigger conditions has been
     * reached.
     */
    u16 dmaconAtDDFStrt;

    // This value is updated in the hsync handler with the lowest 6 bits of
    // dmacon if the master enable bit is 1 or set to 0 if the master enable
    // bit is 0. It is used as an offset into the DAS lookup tables.
    u16 dmaDAS;

    // The disk DMA pointer
    u32 dskpt;

    // The audio DMA pointers
    u32 audpt[4];

    // The bitplane DMA pointers
    u32 bplpt[6];

    // Audio DMA request from Paula
    // This signal is set to true by Paula when a new audio DMA word is needed.
    bool audxDR[4];

    
    // The bitplane modulo registers for odd bitplanes
    i16 bpl1mod;

    // The bitplane modulo registers for even bitplanes
    i16 bpl2mod;

    // The sprite DMA pointers
    u32 sprpt[8];
    
    
    //
    // Bookkeeping
    //

    // Recorded DMA values for all cycles in the current rasterline
    u16 busValue[HPOS_CNT];

    // Recorded DMA usage for all cycles in the current rasterline
    BusOwner busOwner[HPOS_CNT];

    // Unsed in the hsyncHandler to remember the result of inBplDmaLine
    bool oldBplDmaLine;


    //
    // Bus access control
    //

private:

    /* Blitter slow down
     * The BLS signal indicates that the CPU's request to access the bus has
     * been denied for three or more consecutive cycles.
     */
    bool bls;


    //
    // Constructing and destructing
    //
    
public:
    
    Agnus(Amiga& ref);

    void initLookupTables();
    void initBplEventTableLores();
    void initBplEventTableHires();
    void initDasEventTable();

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.revision;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & slot
        & nextTrigger

        & bplEvent
        & dasEvent
        & nextBplEvent
        & nextDasEvent

        & hsyncActions
        & clock
        & frame
        & pos
        & frameInfo.nr
        & frameInfo.interlaced
        & frameInfo.numLines
        & lof
        & sprVStrt
        & sprVStop
        & sprDmaState

        & diwstrt
        & diwstop
        & diwHstrt
        & diwHstop
        & diwVstrt
        & diwVstop
        & diwVFlop
        & diwHFlop
        & diwHFlopOn
        & diwHFlopOff

        & ddfstrt
        & ddfstop
        & ddfstrtReached
        & ddfstopReached
        & ddfVFlop
        & ddfStrtLores
        & ddfStrtHires
        & ddfStopLores
        & ddfStopHires
        & ddfState
        & ocsEarlyAccessLine

        & chngRecorder
        & bplcon0
        & bplcon0AtDDFStrt
        & dmacon
        & dmaconAtDDFStrt
        & dmaDAS
        & dskpt
        & audpt
        & audxDR
        & bplpt
        & bpl1mod
        & bpl2mod
        & sprpt

        & busValue
        & busOwner
        & oldBplDmaLine

        & bls;
    }


    //
    // Configuring
    //

    AgnusConfig getConfig() { return config; }

    AgnusRevision getRevision() { return config.revision; }
    void setRevision(AgnusRevision type);

    bool isOCS() { return config.revision == AGNUS_8367; }
    bool isECS() { return config.revision != AGNUS_8367; }

    // Returns the maximum amout of Chip Ram in KB this Agnus can handle
    long chipRamLimit();

    // Returns a big mask for the memory locations this Agnus can address
    u32 chipRamMask();

    // Returns the line in which the VERTB interrupt gets triggered
    int vStrobeLine() { return isOCS() ? 1 : 0; }

    // Returns the connected bits in DDFSTRT / DDFSTOP
    u16 ddfMask() { return isOCS() ? 0xFC : 0xFE; }


    //
    // Methods from HardwareComponent
    //
    
private:

    void _powerOn() override;
    void _reset() override;
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    void inspectEvents();
    void inspectEventSlot(EventSlot nr);

public:

    void dumpEvents();

    // Returns the result of the most recent call to inspect()
    AgnusInfo getInfo();
    EventInfo getEventInfo();
    EventSlotInfo getEventSlotInfo(int nr);

    // Returns statistical information about the current activiy
    AgnusStats getStats() { return stats; }

    // Resets the collected statistical information
    void clearStats() { memset(&stats, 0, sizeof(stats)); }


    //
    // Examining the current frame
    //

public:

    // Indicates if the current frame is a long or a short frame
    bool isLongFrame() { return frameInfo.numLines == 313; }
    bool isShortFrame() { return frameInfo.numLines == 312; }

    /* Returns the number of master cycles in the current frame.
     * The result depends on the number of lines that are drawn. This values
     * varies between long and short frames.
     */
    Cycle cyclesInFrame();

    /* Returns the master cycle belonging to beam position (0,0)
     * The first function treats (0,0) as the upper left position of the
     * current frame. The second function referes to the next frame.
     */
    Cycle startOfFrame();
    Cycle startOfNextFrame();

    // Indicates if the provided master cycle belongs to a specific frame.
    bool belongsToPreviousFrame(Cycle cycle);
    bool belongsToCurrentFrame(Cycle cycle);
    bool belongsToNextFrame(Cycle cycle);


    //
    // Examining the current rasterline
    //

public:

    // Indicates if the electron beam is inside the VBLANK area
    bool inVBlank() { return pos.v < 26; }

    // Indicates if the electron beam is in the last rasterline
    bool inLastRasterline() { return pos.v == frameInfo.numLines - 1; }

    // Indicates if the electron beam is in a line where bitplane DMA is enabled
    bool inBplDmaLine() { return inBplDmaLine(dmacon, bplcon0); }
    bool inBplDmaLine(u16 dmacon, u16 bplcon0);

    // Returns the pixel position for the current horizontal position
    i16 ppos() { return (pos.h * 4) + 6; }

    
    //
    // Working with DMA slots
    //

public:

    // Returns the firsr DMA cycle belonging to the next fetch unit
    // EXPERIMENTAL
    i16 nextFetchUnit(i16 hpos) { return hpos + (4 - (hpos & 3)); }

    // Returns the next aligned DMA cycle greater or equal to the provided position
    // EXPERIMENTAL
    i16 alignToFetchUnit(i16 hpos) { return nextFetchUnit(hpos - 1); }


    //
    // Working with beam positions
    //

public:

    /* Translates a beam position to a master cycle.
     * 'beam' is a position inside the current frame.
     */
    Cycle beamToCycle(Beam beam);

    /* Translates a master cycle to a beam position.
     * 'cycle' must belong to the current frame for the function to work.
     */
    Beam cycleToBeam(Cycle cycle);

    /* Advances a beam position by a given number of cycles.
     * Note: Only the horizontal component is wrapped over.
     */
    Beam addToBeam(Beam beam, Cycle cycles);


    //
    // Managing DMA access
    //

    // Called by Paula's audio engine to requests a DMA word
    template <int channel> void setAudxDR() { audxDR[channel] = true; }

    // Getter and setter for the BLS signal (Blitter slow down)
    bool getBLS() { return bls; }
    void setBLS(bool value) { bls = value; }

    // Indicates if bitplane DMA is blocked by a hardware stop
    bool bplHwStop() { return pos.h < 0x18 || pos.h >= 0xE0; }

    /* Returns true if Copper execution is blocked.
     * The first function is called in Copper states that do not perform
     * DMA and the second function in those states that do.
     */
    bool copperCanRun();
    bool copperCanDoDMA();

    /* Checks if the bus is currently available for the specified resource.
     */
    template <BusOwner owner> bool busIsFree();

    /* Attempts to allocate the bus for the specified resource.
     * Returns true if the bus was successfully allocated.
     * On success, the bus owner is recorded in the busOwner array.
     */
    template <BusOwner owner> bool allocateBus();


    //
    // Performing DMAs
    //

    u16 doDiskDMA();

    template <int channel> u16 doSpriteDMA();
    template <int channel> u16 doAudioDMA();

    // OLD
    void doDiskDMA(u16 value);
    u16 doSpriteDMA(int channel);
    template <int channel> u16 doBitplaneDMA();

    u16 copperRead(u32 addr);
    void copperWrite(u32 addr, u16 value);

    u16 blitterRead(u32 addr);
    void blitterWrite(u32 addr, u16 value);

    //
    // Managing the DMA allocation table
    //
    
public:

    // Removes all events from the BPL event table except BPL_EOL
    void clearBplEventTable();

    // Removes all events from the DAS event table except BUS_REFRESH
    void clearDasEventTable();

    // Allocates the bitplane DMA slots
    void allocateBplSlots(u16 dmacon, u16 bplcon0, int first, int last = HPOS_MAX-1);
    void allocateBplSlots(int first, int last = HPOS_MAX-1);

    // Adds or removes bitplane DMA events to the DMA event table
    void switchBplDmaOn();
    void switchBplDmaOff();
    void updateBplDma();

    // Adds or removes disk, audio, sprites event to the DAS event table
    void updateDasDma(u16 dmacon);
    void updateDasDma() { updateDasDma(dmacon & 0x3F); }

    // Updates the jump table for a given event table
    void updateJumpTable(EventID *eventTable, u8 *jumpTable, int end);

    // Updates the jump table for the bplEvent table
    void updateBplJumpTable(i16 end = HPOS_MAX);

    // Updates the jump table for the dasEvent table
    void updateDasJumpTable(i16 end = HPOS_MAX);

    // Returns true if the event in the specified slot is the Lx event.
    bool isLastLx(i16 dmaCycle);

    // Returns true if the event in the specified slot is the Hx event.
    bool isLastHx(i16 dmaCycle);

    // Returns true if the specified position belongs to the last fetch unit.
    bool inLastFetchUnit(i16 dmaCycle);

    // Dumps an event table for debugging
    void dumpEventTable(EventID *table, char str[256][2], int from, int to);

    // Dumps the BPL or DAS event table for debugging
    void dumpBplEventTable(int from, int to);
    void dumpBplEventTable();
    void dumpDasEventTable(int from, int to);
    void dumpDasEventTable();

    
    //
    // Accessing registers
    //
    
public:

    // DMACONR
    u16 peekDMACONR();

    // DMACON
    void pokeDMACON(u16 value);
    void setDMACON(u16 oldValue, u16 newValue);

    // Returns true if the Blitter has priority over the CPU
    static bool bltpri(u16 v) { return GET_BIT(v, 10); }
    inline bool bltpri() { return bltpri(dmacon); }

    // Returns true if a certain DMA channel is enabled
    static bool bpldma(u16 v) { return (v & (DMAEN|BPLEN)) == (DMAEN|BPLEN); }
    static bool copdma(u16 v) { return (v & (DMAEN|COPEN)) == (DMAEN|COPEN); }
    static bool bltdma(u16 v) { return (v & (DMAEN|BLTEN)) == (DMAEN|BLTEN); }
    static bool sprdma(u16 v) { return (v & (DMAEN|SPREN)) == (DMAEN|SPREN); }
    static bool dskdma(u16 v) { return (v & (DMAEN|DSKEN)) == (DMAEN|DSKEN); }
    inline bool bpldma() { return bpldma(dmacon); }
    inline bool copdma() { return copdma(dmacon); }
    inline bool bltdma() { return bltdma(dmacon); }
    inline bool sprdma() { return sprdma(dmacon); }
    inline bool dskdma() { return dskdma(dmacon); }
    template <int x> static bool auddma(u16 v) {
        return (v & (DMAEN | (AUD0EN << x))) == (DMAEN | (AUD0EN << x)); }
    template <int x> inline bool auddma() { return auddma<x>(dmacon); }

    // DSKPTH, DSKPTL
    void pokeDSKPTH(u16 value);
    void pokeDSKPTL(u16 value);
    
    // VHPOSR, VHPOS, VPOSR, VPOS
    u16 peekVHPOSR();
    void pokeVHPOS(u16 value);
    u16 peekVPOSR();
    void pokeVPOS(u16 value);
    
    // DIWSTRT, DIWSTOP
    template <PokeSource s> void pokeDIWSTRT(u16 value);
    template <PokeSource s> void pokeDIWSTOP(u16 value);
    void setDIWSTRT(u16 value);
    void setDIWSTOP(u16 value);

    // DDFSTRT, DDFSTOP
    void pokeDDFSTRT(u16 value);
    void pokeDDFSTOP(u16 value);
    void setDDFSTRT(u16 old, u16 value);
    void setDDFSTOP(u16 old, u16 value);

    // Sets up the likely DDF values for the next rasterline
    void predictDDF();

    void computeDDFWindow();

    // Called by computeDDFWindow()
    void computeDDFWindowOCS();
    void computeDDFWindowECS();

    // Called by computeDDFWindowOCS() and computeDDFWindowECS()
    void computeStandardDDFWindow(i16 strt, i16 stop);

    // BPLCON0
    void pokeBPLCON0(u16 value);
    void setBPLCON0(u16 oldValue, u16 newValue);
    void setBPLCON0(u16 newValue) { setBPLCON0(bplcon0, newValue); }

    /* Returns the Agnus view of the BPU bits.
     * The value determines the number of enabled DMA channels. It is computed
     * out of the three BPU bits stored in BPLCON0, but not identical with them.
     * The value differs if the BPU bits reflect an invalid bit pattern.
     * Compare with Denise::bpu() which returns the Denise view of the BPU bits.
     */
    static int bpu(u16 v);
    int bpu() { return bpu(bplcon0); }

    // BPLxPTL, BPLxPTH
    template <int x> void pokeBPLxPTH(u16 value);
    template <int x> void pokeBPLxPTL(u16 value);
    bool skipBPLxPT(int x);
    template <int x> void setBPLxPTH(u16 value);
    template <int x> void setBPLxPTL(u16 value);

    // BPL1MOD, BPL2MOD
    void pokeBPL1MOD(u16 value);
    void setBPL1MOD(u16 value);
    void pokeBPL2MOD(u16 value);
    void setBPL2MOD(u16 value);

    // Sprite registers
    template <int x> void pokeSPRxPTH(u16 value);
    template <int x> void pokeSPRxPTL(u16 value);
    template <int x> void pokeSPRxPOS(u16 value);
    template <int x> void pokeSPRxCTL(u16 value);

    // Adds the modulo register to a bitplane pointer
    template <int x> void addBPLMOD() {
        bplpt[x] += (x % 2) ? bpl2mod : bpl1mod;
    }


    //
    // Operating the device
    //
    
public:

    // Executes the device for a single cycle
    void execute();

    // Executes the device until the target clock is reached
    void executeUntil(Cycle targetClock);

    // Executes the device until the CPU can acquire the bus
    void executeUntilBusIsFree();

    // Schedules a register to change
    void recordRegisterChange(Cycle delay, u32 addr, u16 value);

private:

    // Performs all pending register changes
    void updateRegisters();

    // Executes the first sprite DMA cycle
    template <int nr> void executeFirstSpriteCycle();

    // Executes the second sprite DMA cycle
    template <int nr> void executeSecondSpriteCycle();

    // Updates the sprite DMA status in cycle 0xDF
    void updateSpriteDMA();

    // Finishes up the current rasterline
    void hsyncHandler();

    // Finishes up the current frame
    void vsyncHandler();


    //
    // Serving events
    //

public:

    // Triggers the vertical blank interrupt
    void serviceVblEvent();


    //
    // Class extensions
    //

#include "EventHandler.h"

};

#endif
