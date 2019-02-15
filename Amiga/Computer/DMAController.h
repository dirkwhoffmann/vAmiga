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

//
// THIS CLASS IS A STUB TO GET THE VISUAL PROTOTYPE WORKING
//

class DMAController : public HardwareComponent {
    
public:
    
    // The DMA controller has been executed up to this clock cycle.
    uint64_t clock = 0;
    
    //  Horizontal Beam Position Counter
    uint16_t vhpos = 0;

    //  Vertical Beam Position Counter
    uint16_t vpos = 0;

    // The DMA control register
    uint16_t dmacon = 0;
    


    
    
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
    // Register access
    //
    
public:
    
    uint16_t peekDMACON();
    void pokeDMACON(uint16_t value);
    
    
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
