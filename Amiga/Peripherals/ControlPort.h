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
    
 
    //
    // Constructing and destructing
    //
    
public:
    
    ControlPort(int nr);
    

    //
    // Delivering data for peek methods
    //
    
    public:
    
    // Returns the control port bits showing up in the POTGOR register.
    uint16_t potgor();

    // Returns the control port bits showing up in the JOYxDAT register.
    uint16_t joydat();

    // Returns the control port bits showing up in the CIAA PA register.
    uint8_t ciapa();

    
    //
    // Connecting peripherals
    //
    
public:
    
    // Connects or disconnects a device
    void connectDevice(ControlPortDevice device);
};

#endif

