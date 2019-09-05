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
#define HSYNC_COMPUTE_DDF_WINDOW 0x01
#define HSYNC_UPDATE_EVENT_TABLE 0x02

// Increments a DMA pointer register by 2
#define INC_DMAPTR(x) (x) = ((x) + 2) & 0x7FFFE;

// Assembles a beam position out of two components
#define BEAM(y,x) (((y) << 8) | (x))

// Extracts the vertical or the horizontal component out of a beam position
#define VPOS(x) ((x) >> 8)
#define HPOS(x) ((x) & 0xFF)


class Agnus : public HardwareComponent
{
    // Quick-access references
    class CPU *cpu;
    class CIAA *ciaA;
    class CIAB *ciaB;
    class Memory *mem;
    class Denise *denise;
    class Paula *paula;

    // Information shown in the GUI inspector panel
    DMAInfo info;
    EventInfo eventInfo;

    
    //
    // Sub components
    //
    
public:
    
    // Every Amiga fan knows what the Copper is.
    Copper copper;
    
    // Every Amiga fan knows what the Blitter is.
    Blitter blitter;
    
    // A graphics engine for visualizing DMA accesses
    DmaDebugger dmaDebugger;


    //
    // Lookup tables
    //

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
    EventID nextDASEvent[DAS_EVENT_COUNT][64];
    int16_t nextDASDelay[DAS_EVENT_COUNT][64];

    /* Bitplane DMA events as they appear in a single rasterline.
     *
     * Parameters: bitplaneDMA[Resolution][Bitplanes][Cycle]
     *
     *             Resolution : 0 or 1        (0 = LORES / 1 = HIRES)
     *              Bitplanes : 0 .. 6        (Bitplanes in use, BPU)
     *                  Cycle : 0 .. HPOS_MAX (DMA cycle)
     *
     * The lookup table is used to quickly update the DMA event table.
     * Depending on the current resoution and BPU value, a segment of this
     * lookup table is copied into the DMA event table.
     */
    EventID bitplaneDMA[2][7][HPOS_CNT];

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


    //
    // Events
    //

public:

    // The event table
    Event slot[SLOT_COUNT];

    // Next trigger cycle for an event in the primary event table
    Cycle nextTrigger = NEVER;


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

    /* The DMA controller has been executed up to this clock cycle.
     * Measured in master clock units.
     */
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
    uint16_t diwstrtNew;
    uint16_t diwstop;
    uint16_t diwstopNew;

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

    // Difference between dmaStrtLores and ddfstrt (either 0 or 4)
    int16_t dmaStrtLoresShift;

    bool inLoresDmaArea(int16_t pos) { return pos >= dmaStrtLores && pos < dmaStopLores; }
    bool inHiresDmaArea(int16_t pos) { return pos >= dmaStrtHires && pos < dmaStopHires; }


    //
    // Registers
    //

    // A copy of BPLCON0 (Denise has another copy)
    uint16_t bplcon0;
    uint16_t bplcon0New;

    /* Value of bplcon0 at the DDFSTRT trigger cycle.
     * This variable is set at the beginning of each rasterline and updated
     * on-the-fly when dmacon changes before the trigger conditions has been
     * reached.
     */
    uint16_t bplcon0AtDDFStrt;

    // The DMA control register
    uint16_t dmacon;
    uint16_t dmaconNew;

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
    uint32_t bplptlNew[6];
    uint32_t bplpthNew[6];

    // The bitplane modulo registers for odd bitplanes
    int16_t bpl1mod;
    int16_t bpl1modNew;
    
    // The bitplane modulo registers for even bitplanes
    int16_t bpl2mod;
    int16_t bpl2modNew;
    
    // The sprite DMA pointers
    uint32_t sprpt[8];
    
    
    //
    // Bookkeeping
    //

    // The current value of the data bus
    uint16_t dataBus;

    // Recorded DMA values for all cycles in the current rasterline
    uint16_t busValue[HPOS_CNT];

    // Recorded DMA usage for all cycles in the current rasterline
    BusOwner busOwner[HPOS_CNT];

    // Unsed in the hsyncHandler to remember the result of inBplDmaLine
    bool oldBplDmaLine;

    
    //
    // DMA
    //
    
    /* The DMA time slot allocation table for a complete horizontal line.
     * The array resembles Fig. 6-9 im the HRM (3rd rev.). It's semantics is
     * as follows:
     * If, e.g., audio DMA for channel 1 and 2 is activated, elements
     * dmaEvent[7] and dmaEvent[9] equal AUDEN. If no DMA event takes place at
     * a specific cycle, the array element is 0.
     * The event at position HPOS_CNT is outside the DMA cycle range and
     * triggers the HSYNC handler.
     *
     */
    EventID dmaEvent[HPOS_CNT];
    
    /* Jump table for quick handling the DMA time slot allocation table.
     * For a given horizontal position hpos, nextDmaEvent[hpos] points to the
     * next horizontal position where a DMA event happens.
     */
    uint8_t nextDmaEvent[HPOS_CNT];

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
    
    Agnus();

    // Initializes the lookup tables
    void initLookupTables();
    void initLoresBplEventTable();
    void initHiresBplEventTable();
    void initDASTables();


    //
    // Iterating over snapshot items
    //

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

        & actions
        & hsyncActions
        & clock
        & frame
        & pos.v
        & pos.h
        & frameInfo.nr
        & frameInfo.interlaced
        & frameInfo.numLines
        & lof
        & sprVStrt
        & sprVStop
        & sprDmaState

        & diwstrt
        & diwstrtNew
        & diwstop
        & diwstopNew
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

        & bplcon0
        & bplcon0New
        & bplcon0AtDDFStrt
        & dmacon
        & dmaconNew
        & dmaconAtDDFStrt
        & dmaDAS
        & dskpt
        & audlc
        & audlcold
        & bplpt
        & bplptlNew
        & bplpthNew
        & bpl1mod
        & bpl1modNew
        & bpl2mod
        & bpl2modNew
        & sprpt
        
        & dataBus
        & busValue
        & busOwner
        & oldBplDmaLine

        & dmaEvent
        & nextDmaEvent
        & cpuRequestsBus
        & cpuDenials;
    }
    

    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _reset() override;
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }

private:
    
    void inspectEvents();
    void inspectEventSlot(EventSlot nr);

public:

    void dumpEvents();


    //
    // Reading the internal state
    //
    
public:
    
    // Returns the latest recorded internal state
    DMAInfo getInfo();
    EventInfo getEventInfo();
    EventSlotInfo getEventSlotInfo(int nr);


    //
    // Examining the current frame
    //

public:

    // Returns the current position of the video beam
    // int16_t vpos() { return pos.v; }
    // int16_t hpos() { return pos.h; }

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

    // Removes all events from the DMA time slot allocation table.
    void clearBplEventTable();

    // Allocates the bitplane DMA slots
    void allocateBplSlots(uint16_t dmacon, uint16_t bplcon0, int first, int last = HPOS_MAX-1);
    void allocateBplSlots(int first, int last = HPOS_MAX-1);

    // Adds or removes the bitplane DMA events to the DMA event table.
    void switchBitplaneDmaOn();
    void switchBitplaneDmaOff();
    void updateBitplaneDma();

    // Updates the DMA time slot allocation's jump table.
    void updateJumpTable(int16_t to);
    void updateJumpTable() { updateJumpTable(HPOS_MAX); }

    // Returns true if the event in the specified slot is the Lx event.
    bool isLastLx(int16_t dmaCycle);

    // Returns true if the event in the specified slot is the Hx event.
    bool isLastHx(int16_t dmaCycle);

    // Returns true if the specified position belongs to the last fetch unit.
    bool inLastFetchUnit(int16_t dmaCycle);

    // Dumps the DMA time slot allocation table to the console for debugging.
    void dumpBplEventTable(int from, int to);
    void dumpBplEventTable();

    
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
    static bool bplDMA(uint16_t v) { return (v & (DMAEN | BPLEN)) == (DMAEN | BPLEN); }
    inline bool bplDMA() { return bplDMA(dmacon); }
    static bool copDMA(uint16_t v) { return (v & (DMAEN | COPEN)) == (DMAEN | COPEN); }
    inline bool copDMA() { return copDMA(dmacon); }
    static bool bltDMA(uint16_t v) { return (v & (DMAEN | BLTEN)) == (DMAEN | BLTEN); }
    inline bool bltDMA() { return bltDMA(dmacon); }
    static bool sprDMA(uint16_t v) { return (v & (DMAEN | SPREN)) == (DMAEN | SPREN); }
    inline bool sprDMA() { return sprDMA(dmacon); }
    static bool dskDMA(uint16_t v) { return (v & (DMAEN | DSKEN)) == (DMAEN | DSKEN); }
    inline bool dskDMA() { return dskDMA(dmacon); }
    template <int x> static bool audDMA(uint16_t v) {
        return (v & (DMAEN | (AU0EN << x))) == (DMAEN | (AU0EN << x)); }
    template <int x> inline bool audDMA() { return audDMA<x>(dmacon); }

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

    // Computes the data fetch window's start and stop position
    void computeDDFStrt();
    void computeDDFStop();

    // BPLCON0
    void pokeBPLCON0(uint16_t value);
    void setBPLCON0(uint16_t oldValue, uint16_t newValue);

    // BPLxPTL, BPLxPTH
    template <int x, PokeSource s> void pokeBPLxPTH(uint16_t value);
    template <int x, PokeSource s> void pokeBPLxPTL(uint16_t value);
    bool skipBPLxPT(int x);
    void setBPLxPTH(int x, uint16_t value);
    void setBPLxPTL(int x, uint16_t value);

    // BPL1MOD, BPL2MOD
    void pokeBPL1MOD(uint16_t value);
    void setBPL1MOD(uint16_t value);
    void pokeBPL2MOD(uint16_t value);
    void setBPL2MOD(uint16_t value);

    // SPRxPTL, SPRxPTH
    template <int x> void pokeSPRxPTH(uint16_t value);
    template <int x> void pokeSPRxPTL(uint16_t value);

    // Adds the modulo register to a bitplane pointer
    template <int x> void addBPLMOD() {
        assert(x < 6);
        INC_OCS_PTR(bplpt[x], (x % 2) ? bpl2mod : bpl1mod);
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

    // Sets an action flag
    void setActionFlag(uint64_t flag);

private:

    // Performs all pending register changes
    void updateRegisters();

    // Executes the first sprite DMA cycle
    template <int nr> void executeFirstSpriteCycle();

    // Executes the second sprite DMA cycle
    template <int nr> void executeSecondSpriteCycle();

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
