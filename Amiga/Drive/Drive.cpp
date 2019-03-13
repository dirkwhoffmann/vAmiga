// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"


Drive::Drive(unsigned nr)
{
    assert(nr < 4); // df0 - df3
    
    this->nr = nr;
    setDescription(nr == 0 ? "Df0" :
                   nr == 1 ? "Df1" :
                   nr == 2 ? "Df2" : "Df3");
    
    registerSnapshotItems(vector<SnapshotItem> {

        { &id,             sizeof(id),             0 },
        { &connected,      sizeof(connected),      0 },
        { &motor,          sizeof(motor),          0 },
        { &prb,            sizeof(prb),            0 },
        { &head.cylinder,  sizeof(head.cylinder),  0 },

    });

    // REMOVE AFTER TESTING
    Disk d; 
}

void
Drive::_powerOn()
{
    id  = 0xFFFFFFFF;
    prb = 0xFF;
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

uint8_t
Drive::driveStatusFlags()
{
    uint8_t result = 0xFF;
    
    if (isSelected()) {
        
        // PA5: /DSKRDY
        if (motor) {
            if (hasDisk()) { result &= 0b11011111; }
        } else {
            if (id & 0x8000) { result &= 0b11011111; }
        }
        
        // PA4: /DSKTRACK0
        if (head.cylinder == 0) { result &= 0b11101111; }
        
        // PA3: /DSKPROT
        if (hasWriteProtectedDisk()) { result &= 0b11110111; }
        
        // PA2: /DSKCHANGE
        // TODO
    }
    
    return result;
}

void
Drive::moveIn()
{
    if (head.cylinder < 79)
        head.cylinder++;
    
    debug("moveOIn: new cyclinder = %d\n", head.cylinder);
}

void
Drive::moveOut()
{
    if (head.cylinder > 0)
        head.cylinder--;

    debug("moveOut: new cyclinder = %d\n", head.cylinder);
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
 
    if (value == 0 && !motor) {
        
        // Switch motor on
        motor = true;
        
        // Switch drive LED on
        amiga->putMessage(MSG_DRIVE_LED_ON, nr);
    }
    
    else if (value != 0 && motor) {
        
        // Switch motor off
        motor = false;
        
        // Switch drive LED off
        amiga->putMessage(MSG_DRIVE_LED_OFF, nr);
        
        // Initialize identification mode
        id = 0xFFFFFFFF;
    }
}

void
Drive::PRBdidChange(uint8_t oldValue, uint8_t newValue)
{
    // Ignore this function call if the drive is not connected to the Amiga
    if (!connected) return;

    // -----------------------------------------------------------------
    // | /MTR  | /SEL3 | /SEL2 | /SEL1 | /SEL0 | /SIDE |  DIR  | STEP  |
    // -----------------------------------------------------------------

    bool oldSel = oldValue & (1 << (nr + 3));
    bool newSel = newValue & (1 << (nr + 3));
    bool oldStep = oldValue & 1;
    bool newStep = newValue & 1;
    
    prb = newValue;
    
    // Latch MTR on a falling edge of SELx
    if (FALLING_EDGE(oldSel, newSel)) {
        latchMTR(newValue & 0x80);
    }
    
    // Shift the id code bits on a raising edge of SELx (identification mode)
    if (RISING_EDGE(oldSel, newSel)) {
        id <<= 1;
    }
    
    // Only proceed if this drive is selected (selx is low)
    if (newSel) return;
        
    // Evaluate the side selection bit
    // side = !(newValue & 0b100);
        
    // Move the drive head on a positive edge of the step line
    if (!oldStep && newStep) {
        if (newValue & 2) {
            moveOut();
        } else {
            moveIn();
        }
    }
}
