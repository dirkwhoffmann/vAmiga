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
#include "Disk.h"

class ADFFile;

//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class Drive : public HardwareComponent {
    
private:
    
    // Drive number (0 = df0, 1 = df1, 2 = df2, 3 = df3)
    long nr = 0;
    
    // Indicates if the drive is connected to the Amiga
    bool connected = true;
    
    // The latched MTR bit (drive motor control bit)
    bool mtr;
    
    // The currently selected disk head
    bool side;
    
public:
    
    // The currently inserted disk (if any)
    Disk *disk = NULL;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    // AmigaDrive();
    Drive(unsigned nr);
    
    
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
    
    long getNr() { return nr; }
    
    bool isConnected() { return connected; }
    void setConnected(bool value);
    void toggleConnected();
    
    bool hasDisk() { return disk != NULL; }
    bool hasModifiedDisk() { return disk ? disk->isModified() : false; }
    void setModifiedDisk(bool value) { if (disk) disk->setModified(value); }
    
    bool hasWriteProtectedDisk();
    void toggleWriteProtection();
    
    void ejectDisk();
    void insertDisk(Disk *disk);
    void insertDisk(ADFFile *file);
    
    /* Latches the MTR bit
     * TODO: Quote from HRM?
     */
    void latchMTR(bool value);
    
    /* Delegation function for register CIAB::PRB
     * This function is called whenever CIA B's PRB register changes.
     */
    void PRBdidChange(uint8_t oldValue, uint8_t newValue);
};

#endif
