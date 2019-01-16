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
    // FAKE METHODS FOR THE VISUAL PROTOTYPE
    //
    
public:
    
    void fakeSomething();
    
};


#endif
