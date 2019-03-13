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
    
    /* The drive type identification code
     * Each drive identifies itself by a 32 bit identification code that is
     * transmitted via the DRVRDY signal in a special identification mode. The
     * identification mode is activated by switching the drive motor on and
     * off. The following identification codes are possible:
     *
     *   $00000000 : no drive connected
     *   $55555555 : 5.25" drive
     *   $FFFFFFFF : 3.5" drive
     */
    uint32_t id;

    // Indicates if id should be reset
    bool resetId;
    
    // Indicates if the drive is connected to the Amiga
    bool connected = true;
    
    /* The latched MTR bit (motor control bit)
     * Each drive latches the motor signal at the time it is selected (i.e.,
     * when the SELx line is pulled down to 0). The disk drive motor stays in
     * this state until the drive is selected agail. This bit also controls the
     * activity light on the front of the disk drive.
     */
    // bool mtr;
    
    /* Indicates if the motor is running at full speed
     * On a real drive, it can take up to one half second (500ms) until the
     * drive runs at full speed. We don't emulate  accurate timing here and
     * set the variable to true once the drive motor is switched on.
     */
    bool motor;
    
    // The currently selected disk head
    // DEPRECATED
    // bool side;
    
    // A copy of the PRB register of CIA B
    uint8_t prb;
    
    // The current drive head location
    struct {
        uint8_t cylinder;
    } head;
    
    
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
    
    //
    // Handling the drive status register flags
    //
    
    // Returns true if this drive is currently selected
    bool isSelected() { return (prb & (0b1000 << nr)) == 0; }
    
    uint8_t driveStatusFlags();
    

    //
    // Handling the drive head
    //
    
    // Moves the drive head inwards
    void moveIn();
    
    // Moves the drive head outwards
    void moveOut();
    
    
    //
    // Handling disks
    //


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
