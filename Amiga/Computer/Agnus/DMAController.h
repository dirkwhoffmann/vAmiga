// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DMA_CONTROLLER_INC
#define _DMA_CONTROLLER_INC

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

// Assembles a beam position out of two components
#define BEAM(y,x) (((y) << 8) | (x))

// Extracts a component out of a beam position
#define VPOS(x) ((x) >> 8)
#define HPOS(x) ((x) & 0xFF)

// Maximum possible VPOS and HPOS
#define VPOS_MAX 312
#define HPOS_MAX 226

class DMAController : public HardwareComponent {
    
public:
    
    //
    // Sub components
    //
    
    // Every Amiga fan knows what the Copper is.
    Copper copper;

    // Every Amiga fan knows what the Blitter is.
    Blitter blitter;

    // The event sheduler, a key component of this emulator
    EventHandler eventHandler;
    
    
    //
    // Internal counters
    //

    /* The DMA controller has been executed up to this clock cycle.
     * Measured in master clock units.
     */
    Cycle clock = 0;

    /* A simple frame counter.
     * The value is increased on every VSYNC action.
     */
    int64_t frame = 0;

    /* Value of clock at the beginning of the current frame.
     * The value is latched on every VSYNC action.
     */
    Cycle latchedClock = 0;
    
    // The current vertical beam position (0 .. 312)
    int16_t vpos;

    // The current horizontal beam position (0 .. 226)
    int16_t hpos;
    
    /* The current beam position (17 bit)
     * Format: V8 V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1
     * Use HPOS and VPOS macros to extract the values.
     */
    // int32_t beam = 0;
    

    //
    // DMA book keeping
    //
    
    /* The current owner of the bus
     * This value is updaten in every DMA cycle.
     */
    uint16_t busOwner = 0;
    
    
    //
    // DMA allocation tables
    //
    
    /* The DMA time slot allocation table for a complete horizontal line.
     * The array resembles Fig. 6-9 im the HRM and has the following meaning:
     * If, e.g., Audio DMA for channel 1 and 2 is activated, elements
     * dmaEvent[7] and dmaEvent[9] equal AUDEN. If no DMA event
     * takes place at a specific cycle, the array element is 0.
     */
    EventID dmaEvent[HPOS_MAX + 1];
    
    /* Jump table for quickly accessing the DMA time slot allocation table.
     * For a given horizontal position hpos, nextDmaEvent[hpos] points to the
     * next horizontal position where a DMA event happens. If no further
     * DMA access happens, a 0 is stored.
     */
    uint8_t nextDmaEvent[HPOS_MAX + 1];
    
    
    //
    // Bitplane book keeping
    //
    
    /* Resolution flag
     * This flag is set in every line when the bitplane DMA starts.
     * DEPRECATED. WILL BE REPLACED BY BPLEvent states
     */
    bool lores; 
    
    // The number of currently active bitplanes
    int activeBitplanes = 0;
    
    
    //
    // Sprite book keeping
    //
    
    uint8_t sprOnOff; 
    
    
    //
    // Registers
    //
    
    // The DMA control register
    uint16_t dmacon = 0;
    
    // The display window and display data fetch registers
    uint16_t diwstrt = 0;
    uint16_t diwstop = 0;
    uint16_t ddfstrt = 0;
    uint16_t ddfstop = 0;

    
    //
    // Pointer registers (one for each DMA channel)
    //
    
    /* "The Amiga personal computer system provides a total of 25 DMA channels.
     *  [...] The RAM address generator contains a set of 25 pointer registers.
     *  Each of the pointer registers contains an 18-bit address, which points
     *  to the location in memory of data to be fetched next for the particular
     *  DMA channel. The pointer registers are loaded with data (an address)
     *  from data bus by the processor or coprocessor under program control."
     *  [Patent US 5103499]
     */
    
    // Disk DMA
    uint32_t dskpt;
    
    // Audio DMA
    uint32_t audlc[4];
    
    // Bitplane DMA
    uint32_t bplpt[6];
    
    // Sprite DMA
    uint32_t sprptr[8];
    
    // The bitplane modulo registers
    uint16_t bpl1mod = 0; // odd planes
    uint16_t bpl2mod = 0; // even planes
    
    /* Display window coordinates
     * These values are calculated out of diwstrt and diwstop and updated
     * automatically inside pokeDIWSTRT() and pokeDIWSTOP().
     */
    uint16_t hstrt = 0;
    uint16_t hstop = 0;
    uint16_t vstrt = 0;
    uint16_t vstop = 0;



    
    
    // FAKE COUNTERS FOR tHE VISUAL PROTOTYPE
    unsigned xCounter = 0;
    unsigned yCounter = 0;
    
    //
    // Constructing and destructing
    //
    
public:
    
    DMAController();
    ~DMAController();
 
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    
    //
    // Collecting information
    //
    
public:
    
 
    
    // Collects the data shown in the GUI's debug panel.
    DMAInfo getInfo();
    
    
    //
    // Working with cycles and beam positions
    //
    
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
    
    // Converts a master cycle into a frame position
    FramePosition cycle2FramePosition(Cycle cycle);

    // Converts a frame position into a master cycle
    Cycle framePosition2Cyce(FramePosition framePos);

    
    /* Converts a beam position to a CPU cycle
     * The result is the cycle when we reach that position. It can be smaller,
     * equal or bigger than he current clock value, depending on the current
     * beam position.
     */
    Cycle beam2cycles(int16_t vpos, int16_t hpos);
    Cycle beam2cycles(int32_t beam) { return beam2cycles(beam >> 8, beam & 0xFF); }

    // Builds the DMA time slot allocation table for the current line
    void buildDMAEventTable();

    // Removes all events from the DMA time slot allocation table
    void clearDMAEventTable();

    // Dumps the DMA time slot allocation table to the console for debugging
    void dumpDMAEventTable(int from, int to);
    
    /* Computes the beam coordinate where the next bitplane DMA can happen.
     * The value is dependent on the current values of ddfstrt and ddfstop.
     * Returns -1 of there won't be any more bitplane DMA in the current frame.
     */
    int32_t nextBPLDMABeam(int32_t currentBeam);
    
    /* Computes the CPU cycle when the next bitplane DMA can happen.
     * The function determines the beam position where the next DMA happens.
     * Then, it converts it to the corresponding cylce.
     */
    Cycle nextBpldmaCycle(int32_t currentBeam);
    
    /* Returns the bitplane number for a given beam position.
     * The beam positon is supposed to be a position where a BPLDMA happens.
     */
    // int beam2plane(int32_t beam);
    
private:
    
    
    
    //
    // Accessing registers
    //
    
public:
    
    // DMACON
    uint16_t peekDMACON();
    void pokeDMACON(uint16_t value);
    
    inline bool bplDMA() { return (dmacon & (DMAEN | BPLEN)) == (DMAEN | BPLEN); }
    inline bool copDMA() { return (dmacon & (DMAEN | COPEN)) == (DMAEN | COPEN); }
    inline bool bltDMA() { return (dmacon & (DMAEN | BLTEN)) == (DMAEN | BLTEN); }
    inline bool sprDMA() { return (dmacon & (DMAEN | SPREN)) == (DMAEN | SPREN); }
    inline bool dskDMA() { return (dmacon & (DMAEN | DSKEN)) == (DMAEN | DSKEN); }
    inline bool au3DMA() { return (dmacon & (DMAEN | AU3EN)) == (DMAEN | AU3EN); }
    inline bool au2DMA() { return (dmacon & (DMAEN | AU2EN)) == (DMAEN | AU2EN); }
    inline bool au1DMA() { return (dmacon & (DMAEN | AU1EN)) == (DMAEN | AU1EN); }
    inline bool au0DMA() { return (dmacon & (DMAEN | AU0EN)) == (DMAEN | AU0EN); }
    
    uint16_t peekVHPOS();
    void pokeVHPOS(uint16_t value);
    
    uint16_t peekVPOS();
    void pokeVPOS(uint16_t value);

    void pokeDIWSTRT(uint16_t value);
    void pokeDIWSTOP(uint16_t value);
    void pokeDDFSTRT(uint16_t value);
    void pokeDDFSTOP(uint16_t value);

    void pokeBPL1MOD(uint16_t value);
    void pokeBPL2MOD(uint16_t value);
    
    // DMA pointer registers
    
    void pokeDSKPTH(uint16_t value);
    void pokeDSKPTL(uint16_t value);
  
    void pokeAUDxLCH(int x, uint16_t value);
    void pokeAUDxLCL(int x, uint16_t value);
    
    void pokeBPLxPTL(int x, uint16_t value);
    void pokeBPLxPTH(int x, uint16_t value);
 
    void pokeSPRxPTL(int x, uint16_t value);
    void pokeSPRxPTH(int x, uint16_t value);

    /* Returns the difference of two beam position in master cycles
     * Returns NEVER if the start position is greater than the end position
     * or if the end position is unreachable.
     */
    Cycle beamDiff(int16_t vStart, int16_t hStart, int16_t vEnd, int16_t hEnd);
    Cycle beamDiff(int16_t vEnd, int16_t hEnd) { return beamDiff(vpos, hpos, vEnd, hEnd); }
    Cycle beamDiff(int32_t end) { return beamDiff(VPOS(end), HPOS(end)); }

    /* This functions serves the RAS_HSYNC event in the RAS slot.
     * The RAS_HSYNC event is triggered at the end of each rasterline.
     */
    void hsyncHandler();

    // This function is called when the end of a frame has been reached.
    void vsyncHandler();

    
    //
    // DMA
    //
    
    inline bool spriteIsOn(int i) { return sprOnOff & (1 << i); }
    inline void turnSpriteOn(int i) { sprOnOff |= (1 << i); }
    inline void turnSpriteOff(int i) { sprOnOff &= ~(1 << i); }

    // Returns true if Copper is allowed to perform a DMA cycle
    bool copperCanHaveBus();

    
    //
    // Handling events
    //
    
    // Processes a high-priority DMA event (Disk, Audio, Sprites, Bitplanes)
    void serviceDMAEvent(EventID id, int64_t data);

    // Processes a raster event (Pixel drawing, HSYNC)
    void serviceRASEvent(EventID id);

    // Schedules the next RAS slot event
    void scheduleNextRASEvent(int16_t vpos, int16_t hpos);
 
    /* Adds BPLxMOD to the pointers of the active bitplanes
     * This method is called whenever the bitplane DMA restarts.
     */
    void addBPLxMOD();
    

public:
    
    void executeUntil(Cycle targetClock);
};


#endif
