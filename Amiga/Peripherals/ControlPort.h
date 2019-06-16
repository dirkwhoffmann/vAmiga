// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CONTROL_PORT_INC
#define _CONTROL_PORT_INC

#include "HardwareComponent.h"

class ControlPort : public HardwareComponent {

private:
    
    // Represented control port (1 or 2)
    int nr;
    
    // Connected device
    ControlPortDevice device = CPD_NONE;
    
    // The two mouse position counters
    int64_t mouseCounterX = 0;
    int64_t mouseCounterY = 0;


    //
    // Constructing and destructing
    //
    
public:
    
    ControlPort(int nr);


    //
    // Methods from HardwareComponent
    //

private:

    void _inspect() override;
    void _dump() override;
    

    //
    // Managing registers
    //
    
    public:
    
    // Returns the control port bits showing up in the POTGOR register.
    uint16_t potgor();

    // Returns the control port bits showing up in the JOYxDAT register.
    uint16_t joydat();

    // Returns the control port bits showing up in the CIAA PA register.
    uint8_t ciapa();

    // Emulates a write access to JOYTEST
    void pokeJOYTEST(uint16_t value);

    
    //
    // Connecting peripherals
    //
    
public:
    
    // Connects or disconnects a device
    void connectDevice(ControlPortDevice device);
};

#endif

