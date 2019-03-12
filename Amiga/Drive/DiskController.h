// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DISK_CONTROLLER_INC
#define _DISK_CONTROLLER_INC

#include "HardwareComponent.h"


//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class DiskController : public HardwareComponent {
    
public:
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    DiskController();
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    
public:
    
    bool doesDMA(unsigned nr) { return false; }
};

#endif
