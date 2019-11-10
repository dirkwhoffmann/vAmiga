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

#include "HardwareComponent.h"
#include "Copper.h"
#include "Blitter.h"
#include "DmaDebugger.h"
#include "Beam.h"
#include "Event.h"

// Bit plane indices
#define PLANE1 0
#define PLANE2 1
#define PLANE3 2
#define PLANE4 3
#define PLANE5 4
#define PLANE6 5

// Bit masks as they appear in the DMACON register
#define DMAEN 0b1000000000
#define BPLEN 0b0100000000
#define COPEN 0b0010000000
#define BLTEN 0b0001000000
#define SPREN 0b0000100000
#define DSKEN 0b0000010000
#define AU3EN 0b0000001000
#define AU2EN 0b0000000100
#define AU1EN 0b0000000010
#define AU0EN 0b0000000001
#define AUDEN 0b0000001111

// Hsync action bits
#define HSYNC_COMPUTE_DDF_WINDOW 0b001
#define HSYNC_UPDATE_BPL_TABLE   0b010
#define HSYNC_UPDATE_DAS_TABLE   0b100


// DEPRECATED
// Casts a value into the pointer format used by the Original Chip Set (OCS)
/*
#define CHIP_PTR(x) ((x) & 0x7FFFE)

// Increases or decreases a pointer given in the OCS format by a certain value
#define INC_CHIP_PTR_BY(x,y) ((x) = ((x)+(y)) & 0x7FFFE)
#define SUB_FROM_CHIP_PTR(x,y) ((x) = ((x)-(y)) & 0x7FFFE)

// Increments a DMA pointer register by 2
#define INC_CHIP_PTR(x) (x) = ((x) + 2) & 0x7FFFE;
*/

// Casts an address into the Chip Ram range
#define CHIP_PTR(x) ((x) & mem.chipMask & ~1)

// Increments a Chip Ram pointer by a single word
#define INC_CHIP_PTR(x) ((x) = ((x) + 2) & mem.chipMask & ~1);

// Increases or decreases a Chip Ram pointer by a certain amount
#define INC_CHIP_PTR_BY(x,y) ((x) = ((x)+(y)) & mem.chipMask & ~1)
// #define DEC_CHIP_PTR_BY(x,y) ((x) = ((x)-(y)) & mem.chipMask & ~1)


// Assembles a beam position out of two components
#define BEAM(y,x) (((y) << 8) | (x))

// Extracts the vertical or the horizontal component out of a beam position
#define VPOS(x) ((x) >> 8)
#define HPOS(x) ((x) & 0xFF)

class Agnus : public SubComponent {
    
    // The current configuration
    AgnusConfig config;

    // Information shown in the GUI inspector panel
    AgnusInfo info;
    EventInfo eventInfo;

    // Statistics shown in the GUI monitor panel
     AgnusStats stats;


    //
    // Sub components
    //
    
public:
    
    // Every Amiga fan knows what the Copper is.
    Copper copper = Copper(amiga);
    
    // Every Amiga fan knows what the Blitter is.
    Blitter blitter = Blitter(amiga);
    
    // A graphics engine for visualizing DMA accesses
    DmaDebugger dmaDebugger = DmaDebugger(amiga);


    //
    // Lookup tables
    //

    // DEPRECATED:
    /* DAS lookup tables (Disk, Audio, Sprite DMA)
     *
     * nextDASEvent[DAS Event ID][DMA enable bits]:
     * Used to lookup the next EventID to be scheduled in the DAS slot
     *
     * nextDASDelay[DAS Event ID][DMA enable bits]:
     * Used to lookup when the newly scheduled event should trigger
     *
     *             DMA enable bits : Lowest 6 bits of DMACON
     *                DAS Event ID : Any DAS slot EventID, e.g., DMA_A0
     *
     * Example: If all DMA channels are enabled, the DMA time slot allocation
     *          table looks as follows:
     *
     * 0000000000000000111111111111111100000000000000001111 ...
     * 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123 ...
     *        D D D A A A A S S S S S S S S S S S S S S S S
     *        0 1 2 0 1 2 3 0 0 1 1 2 2 3 3 4 4 5 5 6 6 7 7
     *                          1 2 1 2 1 2 1 2 1 2 1 2 1 2
     *
     * In this case: nextDASEvent[DMA_A0][0b111111] = DMA_A1
     *               nextDASDelay[DMA_A0][0b111111] = 2
     */
    // EventID nextDASEvent[DAS_EVENT_COUNT][64];
    // int16_t nextDASDelay[DAS_EVENT_COUNT][64];

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
    uint8_t fetchUnitNr[2][HPOS_CNT];

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
    uint8_t nextBplEvent[HPOS_CNT];
    uint8_t nextDasEvent[HPOS_CNT];


    //
    // Operation control
    //

    // Action flags checked in every cycle
    uint64_t actions;

    // Action flags checked in the HSYNC handler
    uint64_t hsyncActions;


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

    // The current vertical beam position (0 .. VPOS_MAX)
    // int16_t vpos;
    
    // The current horizontal beam position (0 .. HPOS_MAX)
    // int16_t hpos;

    // Information about the currently drawn frame
    struct {

        // Frame count (will eventually replace variable 'frame')
        Frame nr;

        // Indicates if this frame is drawn in interlace mode
        bool interlaced;

        // The number of rasterlines in the current frame
        int16_t numLines;

    } frameInfo;

    // The long frame flipflop
    bool lof;

    /* The vertical trigger positions of all 8 sprites.
     * Note: The horizontal trigger positions are stored inside Denise. Agnus
     * knows nothing about them.
     */
    int16_t sprVStrt[8];
    int16_t sprVStop[8];

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
    uint16_t diwstrt;
    uint16_t diwstop;

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
    int16_t diwHstrt;
    int16_t diwHstop;
    int16_t diwVstrt;
    int16_t diwVstop;

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
    int16_t diwHFlopOn;
    int16_t diwHFlopOff;


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
    uint16_t ddfstrt;
    uint16_t ddfstop;

    /* At the end of a rasterline, these variables conains the DMA cycles
     * where the hpos counter matched ddfstrt or ddfstop, respectively. A
     * value of -1 indicates that no matching event took place.
     */
    int16_t ddfstrtReached;
    int16_t ddfstopReached;

    /* DDF flipflops
     *
     * The vertical DDF flipflop needs to be set to enable bitplane DMA.
     */
    bool ddfVFlop;

    // The actual data fetch window
    int16_t dmaStrtLores;      // First lores bitplane DMA cycle
    int16_t dmaStrtHires;      // First hires bitplane DMA cycle
    int16_t dmaStopLores;      // Last lores bitplane DMA cycle + 1
    int16_t dmaStopHires;      // Last hires bitplane DMA cycle + 1

    // Difference between dmaStrt and the start of the corresponding fetch unit
    int16_t dmaStrtLoresShift;
    int16_t dmaStrtHiresShift;

    bool inLoresDmaArea(int16_t pos) { return pos >= dmaStrtLores && pos < dmaStopLores; }
    bool inHiresDmaArea(int16_t pos) { return pos >= dmaStrtHires && pos < dmaStopHires; }


    //
    // Registers
    //

    // Ringbuffer for managing register change delays
    ChangeRecorder<8> changeRecorder;

    // A copy of BPLCON0 (Denise has another copy)
    uint16_t bplcon0;

    /* Value of bplcon0 at the DDFSTRT trigger cycle.
     * This variable is set at the beginning of each rasterline and updated
     * on-the-fly when dmacon changes before the trigger conditions has been
     * reached.
     */
    uint16_t bplcon0AtDDFStrt;

    // The DMA control register
    uint16_t dmacon;

    /* Value of dmacon at the DDFSTRT trigger cycle.
     * This variable is set at the beginning of each rasterline and updated
     * on-the-fly when dmacon changes before the trigger conditions has been
     * reached.
     */
    uint16_t dmaconAtDDFStrt;

    // This value is updated in the hsync handler with the lowest 6 bits of
    // dmacon if the master enable bit is 1 or set to 0 if the master enable
    // bit is 0. It is used as an offset into the DAS lookup tables.
    uint16_t dmaDAS;

    // The disk DMA pointer
    uint32_t dskpt;

    // The audio DMA pointers
    uint32_t audlc[4];
    uint32_t audlcold[4];

    // The bitplane DMA pointers
    uint32_t bplpt[6];

    // The bitplane modulo registers for odd bitplanes
    int16_t bpl1mod;

    // The bitplane modulo registers for even bitplanes
    int16_t bpl2mod;

    // The sprite DMA pointers
    uint32_t sprpt[8];
    
    
    //
    // Bookkeeping
    //

    // Recorded DMA values for all cycles in the current rasterline
    uint16_t busValue[HPOS_CNT];

    // Recorded DMA usage for all cycles in the current rasterline
    BusOwner busOwner[HPOS_CNT];

    // Unsed in the hsyncHandler to remember the result of inBplDmaLine
    bool oldBplDmaLine;


    /*
     * Priority logic (CPU versus Blitter)
     *
     * To block the Blitter, three conditions must hold:
     *
     *     - The BLTPRI flag is false
     *     - The CPU must request the bus
     *     - The CPU request must have been denied for three consecutive cycles
     */

private:

    bool cpuRequestsBus;
    int cpuDenials;


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

        & actions
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
        & dmaStrtLores
        & dmaStrtHires
        & dmaStopLores
        & dmaStopHires
        & dmaStrtLoresShift
        & dmaStrtHiresShift

        & changeRecorder
        & bplcon0
        & bplcon0AtDDFStrt
        & dmacon
        & dmaconAtDDFStrt
        & dmaDAS
        & dskpt
        & audlc
        & audlcold
        & bplpt
        & bpl1mod
        & bpl2mod
        & sprpt

        & busValue
        & busOwner
        & oldBplDmaLine

        & cpuRequestsBus
        & cpuDenials;
    }


    //
    // Configuring
    //

    AgnusConfig getConfig() { return config; }

    AgnusRevision getRevision() { return config.revision; }
    void setRevision(AgnusRevision type);

    // Returns the maximum amout of Chip Ram in KB this Agnus can handle
    long chipRamLimit();


    //
    // Methods from HardwareComponent
    //
    
private:

    void _powerOn() override;
    void _reset() override;
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }

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
    bool inBplDmaLine(uint16_t dmacon, uint16_t bplcon0);


    //
    // Working with DMA slots
    //

public:

    // Returns the firsr DMA cycle belonging to the next fetch unit
    // EXPERIMENTAL
    int16_t nextFetchUnit(int16_t hpos) { return hpos + (4 - (hpos & 3)); }

    // Returns the next aligned DMA cycle greater or equal to the provided position
    // EXPERIMENTAL
    int16_t alignToFetchUnit(int16_t hpos) { return nextFetchUnit(hpos - 1); }


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
    // OLD
    //

    // Returns the current beam position as a 17 bit value
    // DEPRECATED
    uint32_t getBeam() { return BEAM(pos.v, pos.h); }
    
    /* Returns the difference of two beam position in master cycles
     * Returns NEVER if the start position is greater than the end position
     * or if the end position is unreachable.
     * DEPRECATED
     */
    Cycle beamDiff(int16_t vStart, int16_t hStart, int16_t vEnd, int16_t hEnd);
    Cycle beamDiff(int16_t vEnd, int16_t hEnd) { return beamDiff(pos.v, pos.h, vEnd, hEnd); }
    Cycle beamDiff(int32_t end) { return beamDiff(VPOS(end), HPOS(end)); }
    

    //
    // Managing DMA access
    //

    // Indicates if bitplane DMA is blocked by a hardware stops
    bool bplHwStop() { return pos.h < 0x18 || pos.h >= 0xE0; }

    /* Returns true if Copper execution is blocked.
     * The first function is called in Copper states that do not perform
     * DMA and the second function in those states that do.
     */
    bool copperCanRun();
    bool copperCanDoDMA();

    //Called by the CPU when it wants to use the bus or no longer needs it.
    void requestBus(bool value);

    /* Attempts to allocate the bus for the specified resource.
     * Returns true if the bus was successfully allocated.
     * On success, the bus owner is recorded in the busOwner array.
     */
    template <BusOwner owner> bool allocateBus();


    //
    // Performing DMAs
    //

    uint16_t doDiskDMA();

    template <int channel> uint16_t doSpriteDMA();

    // OLD
    void doDiskDMA(uint16_t value);
    uint16_t doAudioDMA(int channel);
    uint16_t doSpriteDMA(int channel);
    template <int channel> uint16_t doBitplaneDMA();

    uint16_t copperRead(uint32_t addr);
    void copperWrite(uint32_t addr, uint16_t value);

    uint16_t blitterRead(uint32_t addr);
    void blitterWrite(uint32_t addr, uint16_t value);

    //
    // Managing the DMA allocation table
    //
    
public:

    // Removes all events from the BPL event table except BPL_EOL
    void clearBplEventTable();

    // Removes all events from the DAS event table except BUS_REFRESH
    void clearDasEventTable();

    // Allocates the bitplane DMA slots
    void allocateBplSlots(uint16_t dmacon, uint16_t bplcon0, int first, int last = HPOS_MAX-1);
    void allocateBplSlots(int first, int last = HPOS_MAX-1);

    // Adds or removes bitplane DMA events to the DMA event table
    void switchBplDmaOn();
    void switchBplDmaOff();
    void updateBplDma();

    // Adds or removes disk, audio, sprites event to the DAS event table
    void updateDasDma(uint16_t dmacon);
    void updateDasDma() { updateDasDma(dmacon & 0x3F); }

    // Updates the jump table for a given event table
    void updateJumpTable(EventID *eventTable, uint8_t *jumpTable, int end);

    // Updates the jump table for the bplEvent table
    void updateBplJumpTable(int16_t end = HPOS_MAX);

    // Updates the jump table for the dasEvent table
    void updateDasJumpTable(int16_t end = HPOS_MAX);

    // Returns true if the event in the specified slot is the Lx event.
    bool isLastLx(int16_t dmaCycle);

    // Returns true if the event in the specified slot is the Hx event.
    bool isLastHx(int16_t dmaCycle);

    // Returns true if the specified position belongs to the last fetch unit.
    bool inLastFetchUnit(int16_t dmaCycle);

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
    uint16_t peekDMACONR();

    // DMACON
    void pokeDMACON(uint16_t value);
    void setDMACON(uint16_t oldValue, uint16_t newValue);

    // Returns true if DMA access of a certain type is enabled
    static bool bltpri(uint16_t v) { return GET_BIT(v, 10); }
    inline bool bltpri() { return bltpri(dmacon); }
    static bool doBplDMA(uint16_t v) { return (v & (DMAEN | BPLEN)) == (DMAEN | BPLEN); }
    inline bool doBplDMA() { return doBplDMA(dmacon); }
    static bool doCopDMA(uint16_t v) { return (v & (DMAEN | COPEN)) == (DMAEN | COPEN); }
    inline bool doCopDMA() { return doCopDMA(dmacon); }
    static bool doBltDMA(uint16_t v) { return (v & (DMAEN | BLTEN)) == (DMAEN | BLTEN); }
    inline bool doBltDMA() { return doBltDMA(dmacon); }
    static bool doSprDMA(uint16_t v) { return (v & (DMAEN | SPREN)) == (DMAEN | SPREN); }
    inline bool doSprDMA() { return doSprDMA(dmacon); }
    static bool doDskDMA(uint16_t v) { return (v & (DMAEN | DSKEN)) == (DMAEN | DSKEN); }
    inline bool doDskDMA() { return doDskDMA(dmacon); }
    template <int x> static bool doAudDMA(uint16_t v) {
        return (v & (DMAEN | (AU0EN << x))) == (DMAEN | (AU0EN << x)); }
    template <int x> inline bool doAudDMA() { return doAudDMA<x>(dmacon); }

    // DSKPTH, DSKPTL
    void pokeDSKPTH(uint16_t value);
    void pokeDSKPTL(uint16_t value);
    
    // VHPOSR, VHPOS, VPOSR, VPOS
    uint16_t peekVHPOSR();
    void pokeVHPOS(uint16_t value);
    uint16_t peekVPOSR();
    void pokeVPOS(uint16_t value);
    
    // DIWSTRT, DIWSTOP
    template <PokeSource s> void pokeDIWSTRT(uint16_t value);
    template <PokeSource s> void pokeDIWSTOP(uint16_t value);
    void setDIWSTRT(uint16_t value);
    void setDIWSTOP(uint16_t value);

    // DDFSTRT, DDFSTOP
    void pokeDDFSTRT(uint16_t value);
    void pokeDDFSTOP(uint16_t value);
    void setDDFSTRT(uint16_t old, uint16_t value);
    void setDDFSTOP(uint16_t old, uint16_t value);

    // Computes the data fetch window's start and stop position
    void computeDDFStrt();
    void computeDDFStop();
    void computeDDFWindow() { computeDDFStrt(); computeDDFStop(); }

    // BPLCON0
    void pokeBPLCON0(uint16_t value);
    void setBPLCON0(uint16_t oldValue, uint16_t newValue);

    /* Returns the Agnus view of the BPU bits.
     * The value determines the number of enabled DMA channels. It is computed
     * out of the three BPU bits stored in BPLCON0, but not identical with them.
     * The value differs if the BPU bits reflect an invalid bit pattern.
     * Compare with Denise::bpu() which returns the Denise view of the BPU bits.
     */
    static int bpu(uint16_t v);
    int bpu() { return bpu(bplcon0); }

    // BPLxPTL, BPLxPTH
    template <int x> void pokeBPLxPTH(uint16_t value);
    template <int x> void pokeBPLxPTL(uint16_t value);
    bool skipBPLxPT(int x);
    template <int x> void setBPLxPTH(uint16_t value);
    template <int x> void setBPLxPTL(uint16_t value);

    // BPL1MOD, BPL2MOD
    void pokeBPL1MOD(uint16_t value);
    void setBPL1MOD(uint16_t value);
    void pokeBPL2MOD(uint16_t value);
    void setBPL2MOD(uint16_t value);

    // Sprite registers
    template <int x> void pokeSPRxPTH(uint16_t value);
    template <int x> void pokeSPRxPTL(uint16_t value);
    template <int x> void pokeSPRxPOS(uint16_t value);
    template <int x> void pokeSPRxCTL(uint16_t value);

    // Adds the modulo register to a bitplane pointer
    template <int x> void addBPLMOD() {
        assert(x < 6);
        INC_CHIP_PTR_BY(bplpt[x], (x % 2) ? bpl2mod : bpl1mod);
        // debug("addBPLMOD%d +%d = %d ($%X)\n", x, (x % 2) ? bpl2mod : bpl1mod, bplpt[x], bplpt[x]);
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
    void recordRegisterChange(Cycle delay, uint32_t addr, uint16_t value);

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
    // Class extensions
    //

#include "EventHandler.h"

};

#endif
