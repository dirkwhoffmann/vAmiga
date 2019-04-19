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
#include "EventHandler.h"
#include "Copper.h"
#include "Blitter.h"

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

// Increments a DMA pointer register by 2
#define INC_DMAPTR(x) (x) = ((x) + 2) & 0x7FFFE;

// Maximum vertical and horizontal beam positions
#define VPOS_MAX 312
#define HPOS_MAX 226

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
    private:
    
    // Information shown in the GUI inspector panel
    DMAInfo info;
    
    
    //
    // Sub components
    //
    
    public:
    
    // Every Amiga fan knows what the Copper is.
    Copper copper;
    
    // Every Amiga fan knows what the Blitter is.
    Blitter blitter;
    
    // The event sheduler, a key component of this emulator.
    EventHandler eventHandler;
    
    
    //
    // Counters
    //
    
    /* The DMA controller has been executed up to this clock cycle.
     * Measured in master clock units.
     */
    Cycle clock;
    
    /* The frame counter.
     * The value is increased on every VSYNC action.
     */
    int64_t frame;
    
    /* Value of clock at the beginning of the current frame.
     * The value is latched on every VSYNC action and used for implementing
     * the beamToCycle conversion functions.
     */
    Cycle latchedClock;
    
    // The current vertical beam position (0 .. VPOS_MAX)
    int16_t vpos;
    
    // The current horizontal beam position (0 .. HPOS_MAX)
    int16_t hpos;
    
    /* The display window coordinates.
     * These values are calculated out of diwstrt and diwstop and updated
     * automatically inside pokeDIWSTRT() and pokeDIWSTOP().
     * DEPRECATED (?!)
     */
    int16_t hstrt;
    int16_t hstop;
    int16_t vstrt;
    int16_t vstop;
    
    /* The vertical trigger positions of all 8 sprites.
     * Note: The horizontal trigger positions are stored inside Denise. Agnus
     * knows nothing about them.
     */
    int16_t sprvstrt[8];
    int16_t sprvstop[8];

    // The current DMA states of all 8 sprites.
    SprDMAState sprDmaState[8];

    
    //
    // Registers
    //
    
    // The DMA control register
    uint16_t dmacon;
    
    // The disk DMA pointer
    uint32_t dskpt;
    
    // The display window registers
    uint16_t diwstrt;
    uint16_t diwstop;
    
    // The display data fetch registers
    uint16_t ddfstrt;
    uint16_t ddfstop;
    
    // The audio DMA pointers
    uint32_t audlc[4];
    
    // The bitplane DMA pointers
    uint32_t bplpt[6];
    
    // The bitplane modulo registers for odd bitplanes
    uint16_t bpl1mod;
    
    // The bitplane modulo registers for even bitplanes
    uint16_t bpl2mod;
    
    // The sprite DMA pointers
    uint32_t sprpt[8];
    
    
    //
    // DMA bookkeeping
    //
    
    // The current owner of the bus, updated in every DMA cycle.
    // TODO: THIS VARIABLE IS NOT USED YET
    uint16_t busOwner;
    
    
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
    EventID dmaEvent[HPOS_MAX + 1];
    
    /* Jump table for quick handling the DMA time slot allocation table.
     * For a given horizontal position hpos, nextDmaEvent[hpos] points to the
     * next horizontal position where a DMA event happens. The array element
     * equals 0, if no further DMA access happens after hpos.
     */
    uint8_t nextDmaEvent[HPOS_MAX + 1];
    
    
    //
    // Bitplane bookkeeping
    //
    
    // The number of currently active bitplanes
    int activeBitplanes;
    
    
    //
    // Constructing and destructing
    //
    
    public:
    
    Agnus();
    
    
    //
    // Methods from HardwareComponent
    //
    
    private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _inspect() override; 
    void _dump() override;
    
    
    //
    // Reading the internal state
    //
    
    public:
    
    // Returns the latest internal state recorded by inspect()
    DMAInfo getInfo();
    
    
    //
    // Working with cycles and beam positions
    //
    
    public:
    
    // Returns the current beam position as a 17 bit value
    uint32_t getBeam() { return BEAM(vpos, hpos); }
    
    /* Returns the number of DMA cycles per rasterline
     * The value is valid for PAL machines, only.
     */
    DMACycle cyclesPerLine() { return 227; /* cycles 0x00 ... 0xE2 */ }
    
    /* Returns the number of DMA cycles that make up the current frame
     * The value is valid for PAL machines, only.
     */
    DMACycle cyclesInCurrentFrame();
    
    /* Converts a cycle to a beam position.
     * This function returns the beam position for a provided master clock
     * cycle. The cycle is measured in master clock cycles and either an
     * absolute value (cycles sind power up) or a count relative to the
     * beginning of the current frame.
     */
    void cycleToBeamAbs(Cycle cycle, int64_t &frame, int16_t &vpos, int16_t &hpos);
    void cycleToBeamRel(Cycle cycle, int64_t &frame, int16_t &vpos, int16_t &hpos);
    
    /* Converts a beam position to a master clock cycle.
     * This function returns the cycle of the master clock that corresponds to
     * the provided beam position. The return value is either an absolute
     * cycle count (master cycles since power up) or a relative cycle count
     * (relative to position (0,0) in the current frame).
     */
    Cycle beamToCyclesAbs(int16_t vpos, int16_t hpos);
    Cycle beamToCyclesRel(int16_t vpos, int16_t hpos);
    Cycle beamToCyclesAbs(int32_t beam) { return beamToCyclesAbs(VPOS(beam), HPOS(beam)); }
    Cycle beamToCyclesRel(int32_t beam) { return beamToCyclesRel(VPOS(beam), HPOS(beam)); }
    
    /* Returns the difference of two beam position in master cycles
     * Returns NEVER if the start position is greater than the end position
     * or if the end position is unreachable.
     */
    Cycle beamDiff(int16_t vStart, int16_t hStart, int16_t vEnd, int16_t hEnd);
    Cycle beamDiff(int16_t vEnd, int16_t hEnd) { return beamDiff(vpos, hpos, vEnd, hEnd); }
    Cycle beamDiff(int32_t end) { return beamDiff(VPOS(end), HPOS(end)); }
    
    
    //
    // Managing the DMA allocation tables
    //
    
    public:
    
    // Builds the DMA time slot allocation table for the current line.
    // DEPRECATED
    // void buildDMAEventTable();
    
    // Removes all events from the DMA time slot allocation table.
    void clearDMAEventTable();
    
    // Adds or removes the disk DMA events to the DMA event table.
    void switchDiskDmaOn();
    void switchDiskDmaOff();

    // Adds or removes the audio DMA events to the DMA event table.
    void switchAudioDmaOn(int channel);
    void switchAudioDmaOff(int channel);

    // Adds or removes the sprite DMA events to the DMA event table.
    void switchSpriteDmaOn();
    void switchSpriteDmaOff();
    
    // Adds or removes the audio DMA events to the DMA event table.
    void switchBitplaneDmaOn();
    void switchBitplaneDmaOff();
    void updateBitplaneDma();
    
    // Updates the DMA time slot allocation's jump table.
    void updateJumpTable(int16_t to);
    void updateJumpTable() { updateJumpTable(HPOS_MAX - 1); }
    
    // Dumps the DMA time slot allocation table to the console for debugging.
    void dumpDMAEventTable(int from, int to);
 
    
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
    
    // DDFSTRT, DDFSTOP
    void pokeDDFSTRT(uint16_t value);
    void pokeDDFSTOP(uint16_t value);
    
    // AUDxLCL, AUDxLCL
    void pokeAUDxLCH(int x, uint16_t value);
    void pokeAUDxLCL(int x, uint16_t value);
    
    // BPLxPTL, BPLxPTH
    void pokeBPLxPTH(int x, uint16_t value);
    void pokeBPLxPTL(int x, uint16_t value);
    
    // BPL1MOD, BPL2MOD
    void pokeBPL1MOD(uint16_t value);
    void pokeBPL2MOD(uint16_t value);
    
    // SPRxPTL, SPRxPTH
    void pokeSPRxPTH(int x, uint16_t value);
    void pokeSPRxPTL(int x, uint16_t value);
    
    /* Adds BPLxMOD to the pointers of the active bitplanes
     * This method is called whenever the bitplane DMA restarts.
     */
    void addBPLxMOD();
    
    
    //
    // Stuff to cleanup
    //
    
    public:
    
    // Returns true if Copper is allowed to perform a DMA cycle
    bool copperCanHaveBus();
    
    
    //
    // Running the device
    //
    
    public:
    
    void executeUntil(Cycle targetClock);
    
        
    //
    // Handling events
    //
    
    public:
    
    // Services a high-priority DMA event (Disk, Audio, Sprites, Bitplanes)
    void serviceDMAEvent(EventID id);
    
    // Sub-handlers for sprite DMA events (called by serviceDMAEvent).
    void serviceS1Event(int nr);
    void serviceS2Event(int nr);

    // Services a raster event (Pixel drawing, HSYNC)
    void serviceRASEvent(EventID id);

    // Schedules the first RAS slot event
    void scheduleFirstRASEvent(int16_t vpos);

    // Schedules the next RAS slot event
    // void scheduleNextRASEvent(int16_t vpos, int16_t hpos);
    
    /* This functions serves the RAS_HSYNC event in the RAS slot.
     * The RAS_HSYNC event is triggered at the end of each rasterline.
     */
    void hsyncHandler();
    
    // This function is called when the end of a frame has been reached.
    void vsyncHandler();
};

#endif
