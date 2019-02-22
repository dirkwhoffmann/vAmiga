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


class DMAController : public HardwareComponent {
    
public:
    
    /* The event handler
     * This handler is used to schedule events such as executing or waking up
     * the CIAs.
     */
    EventHandler eventHandler;
    
    // The DMA controller has been executed up to this clock cycle.
    Cycle clock = 0;
    
    // The beam position counter (17 bit)
    // V8 V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1
    uint32_t beam;
    
    //  Horizontal Beam Position Counter
    // uint16_t vhpos = 0;

    //  Vertical Beam Position Counter
    // uint16_t vpos = 0;
    
    // The DMA control register
    uint16_t dmacon = 0;
    
    // Display Window and Display Data Fetch registers
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

    // Copper DMA
    uint32_t coplc[2];
    
    // Audio DMA
    uint32_t audlc[4];
    
    // Bitplane DMA
    uint32_t bplpt[6];
    
    // Sprite DMA
    uint32_t sprptr[8];
    
    
    //
    // Copper
    //
    
    // Comparison value to determine if Copper should run
    // uint16_t copperTrigger;
    
    // The Copper Danger Bit (CDANG)
    bool cdang;
    
    // The Copper instruction register
    uint16_t copins = 0;
    
    // The Copper program counter
    uint32_t coppc = 0;
    
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
    // Accessing registers
    //
    
public:
    
    inline uint16_t hpos() { return beam & 0xFF; }
    inline void sethpos(uint8_t value) { beam = (beam & ~0xFF) | value; }
    inline void inchpos() { beam += 1; }
    inline uint16_t vpos() { return beam >> 8; }
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
    void pokeCOPCON(uint16_t value);
    
    void pokeCOPJMP(int x);
    void pokeCOPINS(uint16_t value); 

    void pokeBPL1MOD(uint16_t value);
    void pokeBPL2MOD(uint16_t value);
    
    // DMA pointer registers
    
    void pokeDSKPTH(uint16_t value);
    void pokeDSKPTL(uint16_t value);

    void pokeBLTxPTL(int x, uint16_t value);
    void pokeBLTxPTH(int x, uint16_t value);

    void pokeCOPxLCH(int x, uint16_t value);
    void pokeCOPxLCL(int x, uint16_t value);
  
    void pokeAUDxLCH(int x, uint16_t value);
    void pokeAUDxLCL(int x, uint16_t value);
    
    void pokeBPLxPTL(int x, uint16_t value);
    void pokeBPLxPTH(int x, uint16_t value);
 
    void pokeSPRxPTL(int x, uint16_t value);
    void pokeSPRxPTH(int x, uint16_t value);

    
    // This function is called when the end of a rasterline has been reached.
    void hsyncAction();

    // This function is called when the end of a frame has been reached.
    void vsyncAction();

    
    //
    // FAKE METHODS FOR THE VISUAL PROTOTYPE
    //
    
public:
    
    void executeUntil(Cycle targetClock);
};


#endif
