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

// Assembles a beam position out of two components
#define BEAM(y,x) (((y) << 8) | (x))

// Extracts a component out of a beam position
#define VPOS(x) ((x) >> 8)
#define HPOS(x) ((x) & 0xFF)


class DMAController : public HardwareComponent {
    
public:
    
    //
    // Sub components
    //
    
    // The Amiga's famous Copper
    Copper copper;
    
    // The event sheduler. It's the key component of this emulator.
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
    
    /* The current beam position (17 bit)
     * Format: V8 V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1
     * Use HPOS and VPOS macros to extract the values.
     */
    int32_t beam = 0;
    

    //
    // DMA book keeping
    //
    
    /* The current owner of the bus
     * This value is updaten in every DMA cycle.
     */
    uint16_t busOwner = 0;
    
    
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
    
    // Blitter DMA
    uint32_t bltpt[4];
    
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
     * These values are calculated out of diwstrt and diwsstop and updated
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
    // Juggling cycles and beam positions
    //
    
    /* Returns the number of CPU cycles per rasterline
     * The value is valid for PAL machines, only.
     */
    Cycle cyclesPerLine() { return 227; /* cycles 0x00 ... 0xE2 */ }
 
    /* Returns the number of CPU cycles that make up the current frame
     * The value is valid for PAL machines, only.
     */
    Cycle cyclesInCurrentFrame();
    
    /* Converts a beam position to a CPU cycle
     * The result is the cycle when we reach that position. It can be smaller,
     * equal or bigger than he current clock value, depending on the current
     * beam position.
     */
    Cycle beam2cycles(int16_t vpos, int16_t hpos);
    Cycle beam2cycles(int32_t beam) { return beam2cycles(beam >> 8, beam & 0xFF); }

    /* Computes the beam coordinate where the next bitplane DMA can happen.
     * The value is dependent on the current values of ddfstrt and ddfstop.
     * Returns -1 of there won't be any more bitplane DMA in the current frame.
     */
    int32_t nextBPLDMABeam(uint32_t currentBeam);
    
    /* Computes the CPU cycle when the next bitplane DMA can happen.
     * Calls nextBPLDMAbeam() to compute the value
     */
    Cycle nextBpldmaCycle(uint32_t currentBeam);
    
    
private:
    
    
    
    //
    // Accessing registers
    //
    
public:
    
    inline void sethpos(uint8_t value) { beam = (beam & ~0xFF) | value; }
    inline void setvpos(uint16_t value) { beam = (beam & 0xFF) | (value << 8); }
    inline void incvpos() { beam += 256; }

    uint16_t peekDMACON();
    void pokeDMACON(uint16_t value);
    uint16_t peekVHPOS();
    uint16_t peekVPOS();
    void pokeDIWSTRT(uint16_t value);
    void pokeDIWSTOP(uint16_t value);
    void pokeDDFSTRT(uint16_t value);
    void pokeDDFSTOP(uint16_t value);

    void pokeBPL1MOD(uint16_t value);
    void pokeBPL2MOD(uint16_t value);
    
    // DMA pointer registers
    
    void pokeDSKPTH(uint16_t value);
    void pokeDSKPTL(uint16_t value);

    void pokeBLTxPTL(int x, uint16_t value);
    void pokeBLTxPTH(int x, uint16_t value);
  
    void pokeAUDxLCH(int x, uint16_t value);
    void pokeAUDxLCL(int x, uint16_t value);
    
    void pokeBPLxPTL(int x, uint16_t value);
    void pokeBPLxPTH(int x, uint16_t value);
 
    void pokeSPRxPTL(int x, uint16_t value);
    void pokeSPRxPTH(int x, uint16_t value);

    /* Returns the difference of two beam position in color clocks
     * Returns UINT32_MAX if the start position is greater than the end position
     * or if the end position is unreachable.
     */
    DMACycle beamDiff(uint32_t start, uint32_t end);
    DMACycle beamDiff(uint32_t end) { return beamDiff(beam, end); }
    
    // This function is called when the end of a rasterline has been reached.
    void hsyncHandler();

    // This function is called when the end of a frame has been reached.
    void vsyncAction();

    //
    // DMA
    //
    
    // Returns true if a certain DMA channel is activated
    inline bool bplDMA() { return (dmacon & (DMAEN | BPLEN)) == (DMAEN | BPLEN); }
    inline bool copDMA() { return (dmacon & (DMAEN | COPEN)) == (DMAEN | COPEN); }
    inline bool bltDMA() { return (dmacon & (DMAEN | BLTEN)) == (DMAEN | BLTEN); }
    inline bool sprDMA() { return (dmacon & (DMAEN | SPREN)) == (DMAEN | SPREN); }
    inline bool dskDMA() { return (dmacon & (DMAEN | DSKEN)) == (DMAEN | DSKEN); }
    
    // Returns true if Copper is allowed to perform a DMA cycle
    bool copperCanHaveBus();

    // Performs a bitplane DMA cycle for plane 1
    void doBplDMA1();
    
    // Performs a bitplane DMA cycle for plane 2 to 6
    void doBplDMA(int plane); 
 
    // Returns the number of active bitplanes
    // int activeBitplanes(); 
    
    /* Adds BPLxMOD to the pointers of the active bitplanes
     * This method is called whenever the bitplane DMA restarts.
     */
    void addBPLxMOD();
    

public:
    
    void executeUntil(Cycle targetClock);
};


#endif
