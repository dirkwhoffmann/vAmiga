// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

/*
AmigaDrive::AmigaDrive()
{
    setDescription("Drive");
}
*/

Drive::Drive(unsigned nr)
{
    assert(nr == 0 /* df0 */ || nr == 1 /* df1 */);
    
    this->nr = nr;
    setDescription(nr == 0 ? "Df0" : "Df1");
    
    // REMOVE AFTER TESTING
    Disk d; 
}

void
Drive::_powerOn()
{
    
}

void
Drive::_powerOff()
{
    
}

void
Drive::_reset()
{
    
}

void
Drive::_ping()
{
    amiga->putMessage(isConnected() ?
                      MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, nr);
    amiga->putMessage(hasDisk() ?
                      MSG_DRIVE_DISK_INSERT : MSG_DRIVE_DISK_EJECT, nr);
    amiga->putMessage(hasWriteProtectedDisk() ?
                      MSG_DRIVE_DISK_PROTECTED : MSG_DRIVE_DISK_UNPROTECTED, nr);
    amiga->putMessage(hasModifiedDisk() ?
                      MSG_DRIVE_DISK_UNSAVED : MSG_DRIVE_DISK_SAVED, nr);
}

void
Drive::_dump()
{
    msg("Has disk: %s\n", hasDisk() ? "yes" : "no");
}

void
Drive::setConnected(bool value)
{
    if (connected != value) {
        
        connected = value;
        amiga->putMessage(connected ? MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, nr);
    }
}

void
Drive::toggleConnected()
{
    setConnected(!isConnected());
}

bool
Drive::hasWriteProtectedDisk()
{
    return hasDisk() ? disk->isWriteProtected() : false;
}

void
Drive::toggleWriteProtection()
{
    if (disk) {
        
        if (disk->isWriteProtected()) {
            
            disk->setWriteProtection(false);
            amiga->putMessage(MSG_DRIVE_DISK_UNPROTECTED);
            
        } else {
            
            disk->setWriteProtection(true);
            amiga->putMessage(MSG_DRIVE_DISK_PROTECTED);
        }
    }
}

void
Drive::ejectDisk()
{
    if (disk) {
        delete disk;
        disk = NULL;
        amiga->putMessage(MSG_DRIVE_DISK_EJECT, nr);
    }
}

void
Drive::insertDisk(Disk *newDisk)
{
    if (newDisk) {
        
        debug("Drive::insertDisk(Disk *)\n");
        
        if (isConnected()) {
            ejectDisk();
            disk = newDisk;
            amiga->putMessage(MSG_DRIVE_DISK_INSERT, nr);
        }
    }
}

void
Drive::insertDisk(ADFFile *file)
{
    if (file) {
        
        // Convert ADF file into a disk
        // AmigaDisk = new AmigaDisk::makeWithFile(ADFFile *file)
        
        debug("Drive::insertDisk(ADFFile *)\n");
        
        Disk *newDisk = new Disk();
        newDisk->encodeDisk(file); 
        insertDisk(newDisk);
    }
}

void
Drive::latchMTR(bool value)
{
    debug("Latching MTR bit %d\n", value);
}

void
Drive::PRBdidChange(uint8_t oldValue, uint8_t newValue)
{
    // _MTR  _SEL3  _SEL2  _SEL1  _SEL0  _SIDE   DIR   STEP
    bool oldSel = oldValue & (1 << (nr + 3));
    bool newSel = newValue & (1 << (nr + 3));
    bool oldStep = oldValue & 1;
    bool newStep = newValue & 1;
    
    // Only proceed if the drive is connected to the Amiga
    if (!connected) return;
    
    // Latch MTR on a falling edge of SELx
    if (oldSel && !newSel) {
        latchMTR(!(newValue & 0x80));
    }
    
    // Only proceed if this drive is selected (selx is low)
    if (newSel) return;
        
    
    // Evaluate the side selection bit
    side = !(newValue & 0b100);
        
    // Move the drive head on a positive edge of the step line
    if (!oldStep && newStep) {
        if (newValue & 2) {
            debug("MOVING DRIVE HEAD OUT (IMPLEMENTATION MISSING)\n");
            //moveHeadOut();
        } else {
            debug("MOVING DRIVE HEAD IN (IMPLEMENTATION MISSING)\n");
            // moveHeadIn();
        }
    }
}
