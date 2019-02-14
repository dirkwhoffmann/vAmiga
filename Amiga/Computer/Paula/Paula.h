// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _PAULA_INC
#define _PAULA_INC

#include "AudioUnit.h"

//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class Paula : public HardwareComponent {
    
public:
    
    // Paula has been executed up to this clock cycle.
    Cycle clock = 0;
    
    // Audio unit
    AudioUnit audioUnit;
    
    // The interrupt request register
    uint16_t intreq;
    
    // The interrupt enable register
    uint16_t intena;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Paula();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    /*
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    */
    void _setWarp(bool value) override;
    
public:
    
    // Collects the data shown in the GUI's debug panel.
    PaulaInfo getInfo();
    
    
    //
    // Register access
    //
    
public:
    
    uint16_t peekINTREQ();
    void pokeINTREQ(uint16_t value);
 
    uint16_t peekINTENA();
    void pokeINTENA(uint16_t value);
    
    
    //
    // Interrupts
    //
    
private:
    
    // Computes the interrupt level of a pending interrupt.
    int interruptLevel();
    
    //
    // FAKE METHODS FOR THE VISUAL PROTOTYPE (TEMPORARY)
    //
    
public:
    
    /*
    uint32_t getVolume() { return 42; }
    long bufferUnderflows() { return 42; }
    long bufferOverflows() { return 42; }
    double fillLevel() { return .5; }
     */
};

#endif
