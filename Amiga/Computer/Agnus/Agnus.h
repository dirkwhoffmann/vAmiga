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

// Hsync action bits
#define HSYNC_UPDATE_EVENT_TABLE 0x01
#define HSYNC_UPDATE_DAS_SLOT    0x02

// Increments a DMA pointer register by 2
#define INC_DMAPTR(x) (x) = ((x) + 2) & 0x7FFFE;

// Assembles a beam position out of two components
#define BEAM(y,x) (((y) << 8) | (x))

// Extracts the vertical or the horizontal component out of a beam position
#define VPOS(x) ((x) >> 8)
#define HPOS(x) ((x) & 0xFF)

// Verifies a VPOS / HPOS combination
static inline bool isVposHpos(int16_t vpos, int16_t hpos) {
    return vpos >= 0 && vpos <= VPOS_MAX && hpos >= 0 && hpos <= HPOS_MAX;
}

class Agnus : public HardwareComponent
{
    // Quick-access references
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
    
    // The event sheduler, a key component of this emulator.
    // EventHandler events;

    // A graphics engine for visualizing DMA accesses
    DmaDebugger dmaDebugger;


    //
    // Lookup tables
    //

    /* DAS lookup tables (Disk, Audio, Sprite DMA)
     *
     * firstDASEvent[DMA enable bits]:
     * Used to lookup the first DAS event in a rasterline
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
    EventID firstDASEvent[64];
    int16_t firstDASDelay[64];
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


    //
    // Events
    //

public:

    // The event table
    Event slot[SLOT_COUNT];

    // Next trigger cycle for an event in the primary event table
    Cycle nextTrigger = NEVER;


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

    // The current vertical beam position (0 .. VPOS_MAX)
    int16_t vpos;
    
    // The current horizontal beam position (0 .. HPOS_MAX)
    int16_t hpos;

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

    /* The bitplane DMA window.
     *     dmaStrtLores is the first lores bitplane DMA cycle
     *     dmaStrtHires is the first hires bitplane DMA cycle
     *     dmaStopLores is the last lores bitplane DMA cycle + 1
     *     dmaStopHires is the last hires bitplane DMA cycle + 1
     */
    int16_t dmaStrtLores;
    int16_t dmaStrtHires;
    int16_t dmaStopLores;
    int16_t dmaStopHires;

    /* The bitplane DMA cycle window.
     *     dmaStrt is the first cycle of the first fetch unit
     *     dmaStop is the last cycle of the last fetch unit
     * The values are updated in pokeDDFSTRT() and pokeDDFSTOP()
     */
    int16_t dmaStrt; // DEPRECATED
    int16_t dmaStop; // DEPRECATED

    /* The vertical trigger positions of all 8 sprites.
     * Note: The horizontal trigger positions are stored inside Denise. Agnus
     * knows nothing about them.
     */
    int16_t sprVStrt[8];
    int16_t sprVStop[8];

    // The current DMA states of all 8 sprites.
    SprDMAState sprDmaState[8];


    //
    // Display window (DIW)
    //

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

    /* The DIW flipflop
     *
     * The current implementation is based on the following assumptions:
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

    /* Value of the DIW flipflops
     * Variable vFlop stores the value of the vertical DIW flipflop. The value
     * is updated at the beginning of each rasterline and cannot change
     * thereafter. Variable hFlop stores the value of the horizontal DIW
     * flipflop as it was at the beginning of the rasterline. To find out
     * the value of the horizontal flipflop inside or at the end of a
     * rasterline, hFlopOn and hFlopOff need to be evaluated.
     */
    bool vFlop;
    bool hFlop;

    /* At the end of a rasterline, this variable conains the pixel coordinate
     * where the hpos counter matched diwHstrt or diwHstop, respectively. A
     * value of -1 indicates that no matching event took place.
     */
    int16_t hFlopOn;
    int16_t hFlopOff;


    //
    // Registers
    //
    
    // The DMA control register
    uint16_t dmacon;
    
    // The disk DMA pointer
    uint32_t dskpt;
    
    // The display window registers
    // uint16_t diwstrt;
    // uint16_t diwstop;
    
    // The display data fetch registers
    uint16_t ddfstrt;
    uint16_t ddfstop;
    uint16_t ddfstrtAligned; // DEPRECATED
    uint16_t ddfstopAligned; // DEPRECATED
    
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

    // The number of currently active bitplanes
    uint8_t activeBitplanes;

    // Recorded DMA usage for all cycles in the current rasterline
    BusOwner busOwner[HPOS_CNT];

    // Recorded DMA values for all cycles in the current rasterline
    uint16_t busValue[HPOS_CNT];

    // Unsed in the hsyncHandler to remember the result of inBplDmaArea
    bool oldBplDmaArea;

    
    //
    // DMA allocation tables
    //
    
    /* The DMA time slot allocation table for a complete horizontal line.
     * The array resembles Fig. 6-9 im the HRM (3rd rev.). It's semantics is
     * as follows:
     * If, e.g., audio DMA for channel 1 and 2 is activated, elements
     * dmaEvent[7] and dmaEvent[9] equal AUDEN. If no DMA event takes place at
     * a specific cycle, the array element is 0.
     */
    EventID dmaEvent[HPOS_CNT];
    
    /* Jump table for quick handling the DMA time slot allocation table.
     * For a given horizontal position hpos, nextDmaEvent[hpos] points to the
     * next horizontal position where a DMA event happens. The array element
     * equals 0, if no further DMA access happens after hpos.
     */
    uint8_t nextDmaEvent[HPOS_CNT];

    // Indicates if the dma event table needs an update
    // bool dmaTableIsDirty;

    // DMA cycle of the first and last DMA_H1 or DMA_L1 event
    uint8_t dmaFirstBpl1Event;
    uint8_t dmaLastBpl1Event;

    
    //
    // Operation control
    //

    /* Work items to be processed at the end of the current rasterline
     * This variable is 0 most of the time. If some special action needs to
     * be taken at the end of the current rasterline, certain bits are set
     * to 1.
     */
    uint64_t hsyncActions;

    
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
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _inspect() override;
    void _dump() override;

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
    // Investigating the current frame
    //

public:

    // Indicates if the current frame is a long or a short frame
    bool isLongFrame() { return frameInfo.numLines == 313; }
    bool isShortFrame() { return frameInfo.numLines == 312; }

    /* Returns the number of master cycles in the current frame.
     * The result depends on the number of lines that are drawn. This values
     * varies between long and short frames.
     */
    Cycle cyclesInCurrentFrame();

    /* Indicates if the provided master cycle belongs to a specific frame.
     */
    bool belongsToCurrentFrame(Cycle cycle);

    /* Returns the master cycle belonging to beam position (0,0)
     * The first function treats (0,0) as the upper left position of the
     * current frame. The second function referes to the next frame.
     */
    Cycle startOfCurrentFrame();
    Cycle startOfNextFrame();


    //
    // Investigating the current rasterline
    //

public:

    // Indicates if the current rasterline is a bitplane DMA line
    bool inBplDmaArea();


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

    // Returns the current beam position
    Beam beamPosition();

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
    uint32_t getBeam() { return BEAM(vpos, hpos); }
    
    /* Returns the number of DMA cycles per rasterline
     * The value is valid for PAL machines, only.
     * DEPRECATED
     */
    DMACycle DMACyclesPerLine() { return 227; /* cycles 0x00 ... 0xE2 */ }
    
    /* Returns the number of DMA cycles that make up the current frame
     * The value is valid for PAL machines, only.
     * DEPRECATED
     */
    DMACycle DMACyclesInCurrentFrame();

    /* Returns the difference of two beam position in master cycles
     * Returns NEVER if the start position is greater than the end position
     * or if the end position is unreachable.
     * DEPRECATED
     */
    Cycle beamDiff(int16_t vStart, int16_t hStart, int16_t vEnd, int16_t hEnd);
    Cycle beamDiff(int16_t vEnd, int16_t hEnd) { return beamDiff(vpos, hpos, vEnd, hEnd); }
    Cycle beamDiff(int32_t end) { return beamDiff(VPOS(end), HPOS(end)); }
    

    //
    // Managing DMA access
    //

    // Returns true if Copper is allowed to perform a DMA cycle
    bool copperCanHaveBus();


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


    //
    // Managing the DMA allocation table
    //
    
public:

    // Removes all events from the DMA time slot allocation table.
    void clearDMAEventTable();

    // Allocates the bitplane DMA slots
    void allocateBplSlots(int bpu, bool hires, int first, int last);
    void allocateBplSlots(int bpu, bool hires, int first);

    // Adds or removes the disk DMA events to the DMA event table.
    void switchDiskDmaOn();
    void switchDiskDmaOff();

    // Adds or removes the audio DMA events to the DMA event table.
    void switchAudioDmaOn(int channel);
    void switchAudioDmaOff(int channel);

    // Adds or removes the sprite DMA events to the DMA event table.
    void switchSpriteDmaOn();
    void switchSpriteDmaOff();
    
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

    // Dumps the DMA time slot allocation table to the console for debugging.
    void dumpDMAEventTable(int from, int to);
    void dumpDMAEventTable();

    
    //
    // Accessing registers
    //
    
public:
    
    // DMACON
    uint16_t peekDMACONR();
    void pokeDMACON(uint16_t value);
    
    // Returns true if DMA access of a certain type is currently enabled.
    inline bool bplDMA() { return (dmacon & (DMAEN | BPLEN)) == (DMAEN | BPLEN); }
    inline bool copDMA() { return (dmacon & (DMAEN | COPEN)) == (DMAEN | COPEN); }
    inline bool bltDMA() { return (dmacon & (DMAEN | BLTEN)) == (DMAEN | BLTEN); }
    inline bool sprDMA() { return (dmacon & (DMAEN | SPREN)) == (DMAEN | SPREN); }
    inline bool dskDMA() { return (dmacon & (DMAEN | DSKEN)) == (DMAEN | DSKEN); }
    inline bool au3DMA() { return (dmacon & (DMAEN | AU3EN)) == (DMAEN | AU3EN); }
    inline bool au2DMA() { return (dmacon & (DMAEN | AU2EN)) == (DMAEN | AU2EN); }
    inline bool au1DMA() { return (dmacon & (DMAEN | AU1EN)) == (DMAEN | AU1EN); }
    inline bool au0DMA() { return (dmacon & (DMAEN | AU0EN)) == (DMAEN | AU0EN); }
    inline bool audDMA(int x) {
        return (dmacon & (DMAEN | (AU0EN << x))) == (DMAEN | (AU0EN << x)); }

    // DSKPTH, DSKPTL
    void pokeDSKPTH(uint16_t value);
    void pokeDSKPTL(uint16_t value);
    
    // VHPOSR, VHPOS, VPOSR, VPOS
    uint16_t peekVHPOSR();
    void pokeVHPOS(uint16_t value);
    uint16_t peekVPOSR();
    void pokeVPOS(uint16_t value);
    
    // DIWSTRT, DIWSTOP
    void pokeDIWSTRT(uint16_t value);
    void pokeDIWSTOP(uint16_t value);
    void setDIWSTRT(uint16_t value);
    void setDIWSTOP(uint16_t value);

    // DDFSTRT, DDFSTOP
    void pokeDDFSTRT(uint16_t value);
    void pokeDDFSTOP(uint16_t value);
    void computeDDFWindow();

    // AUDxLCL, AUDxLCL
    // template <int x> void pokeAUDxLCH(uint16_t value);
    // template <int x> void pokeAUDxLCL(uint16_t value);

    // BPLxPTL, BPLxPTH
    template <int x> void pokeBPLxPTH(uint16_t value);
    template <int x> void pokeBPLxPTL(uint16_t value);
    
    // BPL1MOD, BPL2MOD
    void pokeBPL1MOD(uint16_t value);
    void pokeBPL2MOD(uint16_t value);
    void setBPL1MOD(uint16_t value);
    void setBPL2MOD(uint16_t value);

    // SPRxPTL, SPRxPTH
    template <int x> void pokeSPRxPTH(uint16_t value);
    template <int x> void pokeSPRxPTL(uint16_t value);

    // Adds the modulo register to a bitplane pointer
    template <int x> void addBPLMOD() {
        assert(x < 6);
        INC_OCS_PTR(bplpt[x], (x % 2) ? bpl2mod : bpl1mod);
    }

    /* Adds BPLxMOD to the pointers of the active bitplanes
     * This method is called whenever the bitplane DMA restarts.
     */
    void addBPLxMOD();


    //
    // Running the device
    //
    
public:

    // Executes the device until the target clock is reached
    void executeUntil(Cycle targetClock);

private:

    // Executes the first sprite DMA cycle
    template <int nr> void executeFirstSpriteCycle();

    // Executes the second sprite DMA cycle
    template <int nr> void executeSecondSpriteCycle();

    /* Concludes a rasterline
     * Called when servicing a SYNC_H event in the SYNC slot.
     */
    void hsyncHandler();

    /* Concludes a rasterline
     * Called by hsyncHandler() when the last rasterline has been concluded.
     */
    void vsyncHandler();


    //
    // Class extensions
    //

#include "EventHandler.h"

};

#endif
