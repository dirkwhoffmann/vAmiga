// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_DRIVE_INC
#define _AMIGA_DRIVE_INC

#include "HardwareComponent.h"

class AmigaDisk;
class ADFFile;

//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class AmigaDrive : public HardwareComponent {
    
public:
    
    // Indicates if the drive is connected to the Amiga
    bool connected = true;
    
    // The currently inserted disk (if any)
    AmigaDisk *disk = NULL;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    AmigaDrive();
    
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
    
    bool isConnected() { return connected; }
    void setConnected(bool value) { connected = value; }

    bool hasDisk() { return disk != NULL; }
    void ejectDisk();
    void insertDisk(AmigaDisk *disk);
    void insertDisk(ADFFile *file);

};

#endif
