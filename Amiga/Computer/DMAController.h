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
    
    //  Horizontal Beam Position Counter
    uint16_t vhpos = 0;

    //  Vertical Beam Position Counter
    uint16_t vpos = 0;
    
    // The DMA control register
    uint16_t dmacon = 0;
    
    // Display Window and Display Data Fetch registers
    uint16_t diwstrt = 0;
    uint16_t diwstop = 0;
    uint16_t ddfstrt = 0;
    uint16_t ddfstop = 0;

    // The 6 bitplane address pointers
    uint32_t bplpt[6];
    
    // The bitplane modulo registers
    uint16_t bpl1mod = 0; // odd planes
    uint16_t bpl2mod = 0; // even planes

    // Sprite pointer registers
    uint32_t sprptr[8]; 
    
    
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
    
    uint16_t peekDMACON();
    void pokeDMACON(uint16_t value);
    void pokeDIWSTRT(uint16_t value);
    void pokeDIWSTOP(uint16_t value);
    void pokeDDFSTRT(uint16_t value);
    void pokeDDFSTOP(uint16_t value);
    void pokeBPLxPTL(int x, uint16_t value);
    void pokeBPLxPTH(int x, uint16_t value);
    void pokeBPL1MOD(uint16_t value);
    void pokeBPL2MOD(uint16_t value);
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
