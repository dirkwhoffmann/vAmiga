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

        // Configuration items
        { &type,             sizeof(type),             PERSISTANT },
        { &id,               sizeof(id),               PERSISTANT },

        // Internal state items
        { &idMode,           sizeof(idMode),           0 },
        { &idCount,          sizeof(idCount),          0 },
        { &motor,            sizeof(motor),            0 },
        { &dskchange,        sizeof(dskchange),        0 },
        { &dsklen,           sizeof(dsklen),           0 },
        { &prb,              sizeof(prb),              0 },
        { &head.side,        sizeof(head.side),        0 },
        { &head.cylinder,    sizeof(head.cylinder),    0 },
        { &head.offset,      sizeof(head.offset),      0 },
        { &cylinderHistory,  sizeof(cylinderHistory),  0 },

    });
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
Drive::setType(DriveType t)
{
    switch ((type = t)) {
        
        case A1010_ORIG: id = DRIVE_ID_35DD; acceleration =  1; break;
        case A1010_2X:   id = DRIVE_ID_35DD; acceleration =  2; break;
        case A1010_4X:   id = DRIVE_ID_35DD; acceleration =  4; break;
        case A1010_8X:   id = DRIVE_ID_35DD; acceleration =  8; break;
        case A1010_WARP: id = DRIVE_ID_35DD; acceleration = -1; break;
        
        default: assert(false);
    }
    debug("Setting drive type to %s (speedup: %d)\n", driveTypeName(type), acceleration);
}

bool
Drive::isDataSource()
{
    /* I'm not exactly sure abot the conditions to make the drive writes onto
     * the data lines. I assume that the drive must be selected *and* the motor
     * needs to be switched on. 
     */
    return isSelected() && motor; 
}

uint8_t
Drive::driveStatusFlags()
{
    uint8_t result = 0xFF;
    
    if (isSelected()) {
        
        // PA5: /DSKRDY
        if (idMode) {
            // debug("ID mode is on\n");
            if (id & (1 << idCount)) result &= 0b11011111;
        } else {
            // debug("ID mode is off\n");
            if (motor && hasDisk()) result &= 0b11011111;
        }
        
        // PA4: /DSKTRACK0
        // debug("Head is at cyclinder %d\n", head.cylinder);
        if (head.cylinder == 0) { result &= 0b11101111; }
        
        // PA3: /DSKPROT
        if (!hasWriteEnabledDisk()) { result &= 0b11110111; }
        
        /* PA2: /DSKCHANGE
         * "Disk has been removed from the drive. The signal goes low whenever
         *  a disk is removed. It remains low until a disk is inserted AND a
         *  step pulse is received." [HRM]
         */
        if (!dskchange) result &= 0b11111011;
    }
    
    return result;
}

void
Drive::setMotor(bool value)
{
    if (!motor && value) {
        
        debug("*** Motor on\n");
        
        // Turn motor on
        motor = true;
        
        // Quit identification mode
        // debug("Quitting identification mode\n");
        idMode = false;
        idCount = 0;
    }
    
    else if (motor && !value) {
        
        debug("*** Motor off\n");

        // Turn motor off
        motor = false;
        
        // Enter identification mode
        // debug("Entering identification mode/n");
        idMode = true;
        idCount = 32;
    }
}

void
Drive::selectSide(int side)
{
    assert(side < 2);
    if (head.side != side) debug("*** Select side %d\n", side);

    head.side = side;
}

uint8_t
Drive::readHead()
{
    if (disk) {
        return disk->readHead(head.cylinder, head.side, head.offset);
    } else {
        return 0xFF;
    }
}

void
Drive::rotate()
{
    // debug("head = %d\n", head.offset);
    
    if (++head.offset == Disk::mfmBytesPerTrack) {
        
        // Start over at the beginning of the current cyclinder
        head.offset = 0;
        
        /* If this drive is currently selected, we emulate a falling edge on
         * the flag pin of CIA A. This causes the CIA to trigger the INDEX
         * interrupt if the corresponding enable bit is set.
         */
        if (isSelected()) amiga->ciaA.emulateFallingEdgeOnFlagPin();
    }
    assert(head.offset < Disk::mfmBytesPerTrack);
}

void
Drive::moveHead(int dir)
{
    // Update disk change signal
    if (hasDisk()) dskchange = true;
    
    if (dir) {
        
        // Move drive head outwards (towards the lower tracks)
        if (head.cylinder > 0) head.cylinder--;
    
    } else {
        
        // Move drive head inwards (towards the upper tracks)
        if (head.cylinder < 79) head.cylinder++;
    }
    
    /* Inform the GUI
     * We send a MSG_DRIVE_HEAD_POLL, if a disk change polling signature is
     * detected. If no pattern is detected, MSG_DRIVE_HEAD is sent.
     */
    cylinderHistory = ((cylinderHistory << 8) | head.cylinder) & 0xFFFFFFFF;
    if (cylinderHistory == 0x00010001 || cylinderHistory == 0x01000100) {
        amiga->putMessage(MSG_DRIVE_HEAD_POLL);
    } else {
        amiga->putMessage(MSG_DRIVE_HEAD);
    }
}

bool
Drive::hasWriteEnabledDisk()
{
    return hasDisk() ? !disk->isWriteProtected() : false;
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
        
        // Flag disk change in the CIAA::PA
        dskchange = false;
        
        // Get rid of the disk
        delete disk;
        disk = NULL;
        
        // Notify the GUI
        amiga->putMessage(MSG_DRIVE_DISK_EJECT, nr);
    }
}

void
Drive::insertDisk(Disk *disk)
{
    if (disk) {
        
        ejectDisk();
        this->disk = disk;
        amiga->putMessage(MSG_DRIVE_DISK_INSERT, nr);
    }
}

void
Drive::insertDisk(ADFFile *file)
{
    insertDisk(Disk::makeWithFile(file));
}

void
Drive::latchMTR(bool value)
{
    // debug("Latching MTR bit %d\n", value);
 
    if (value == 0 && !motor) {
        
        // Switch motor on
        setMotor(true);
        
        // Switch drive LED on
        amiga->putMessage(MSG_DRIVE_LED_ON, nr);
    }
    else if (value != 0 && motor) {
        
        // Switch motor off
       setMotor(false);
        
        // Switch drive LED off
        amiga->putMessage(MSG_DRIVE_LED_OFF, nr);
    }
}

// TODO: MOVE THIS TO PAULA ?!
void
Drive::pokeDSKLEN(uint16_t value)
{
 
}

void
Drive::PRBdidChange(uint8_t oldValue, uint8_t newValue)
{
    // -----------------------------------------------------------------
    // | /MTR  | /SEL3 | /SEL2 | /SEL1 | /SEL0 | /SIDE |  DIR  | STEP  |
    // -----------------------------------------------------------------

    bool oldSel  = oldValue & (0b1000 << nr);
    bool newSel  = newValue & (0b1000 << nr);
    bool oldStep = oldValue & 1;
    bool newStep = newValue & 1;
    bool dir     = newValue & 2;
    
    // Store a copy of the new PRB value
    prb = newValue;
    
    // Latch MTR on a falling edge of SELx
    if (FALLING_EDGE(oldSel, newSel)) latchMTR(newValue & 0x80);
    
    // In identification mode, select the next id bit on a raising edge of SELx
    if (idMode && idCount && RISING_EDGE(oldSel, newSel)) idCount--;
    
    // Only proceed if this drive is selected (selx is low)
    if (newSel) return;
        
    // Evaluate the side selection bit
    if (head.side != !(newValue & 0b100)) {
        debug("Switching to side %d\n", !(newValue & 0b100));
    }
    head.side = !(newValue & 0b100);
        
    // Move the drive head if there is a falling edge on the step line
    if (FALLING_EDGE(oldStep, newStep)) moveHead(dir);
}
