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
        { &speed,            sizeof(speed),            PERSISTANT },

        // Internal state items
        { &idCount,          sizeof(idCount),          0 },
        { &idBit,            sizeof(idBit),            0 },
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
    assert(isDriveType(t));
    
    type = t;
 
    debug("Setting drive type to %s\n", driveTypeName(type));
}

void
Drive::setSpeed(uint16_t value)
{
    assert(speed == 0 ||
           speed == 1 ||
           speed == 2 ||
           speed == 4 ||
           speed == 8 );
    
    speed = value;
    
    debug("Setting acceleration factor to %d\n", value);

}

uint32_t
Drive::getDriveId()
{
    /* Floopy drives identify themselve with the following codes:
     *
     *   3.5" DD :  0xFFFFFFFF
     *   3.5" HD :  0xAAAAAAAA  if an HD disk is inserted
     *              0xFFFFFFFF  if no disk or a DD disk is inserted
     *   5.25"SD :  0x55555555
     *
     * An unconnected drive corresponds to ID 0x00000000.
     */
    
    assert(type == DRIVE_35_DD);
    return 0xFFFFFFFF;
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
        if (!motor) {
            // debug("ID mode is on\n");
            if (idBit) result &= 0b11011111;
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
        
        // printf("Motor on\n");
        
        amiga->putMessage(MSG_DRIVE_LED_ON, nr);
        amiga->putMessage(MSG_DRIVE_MOTOR_ON, nr);
    }
    
    else if (motor && !value) {
  
        // printf("Motor off\n");
        
        // Reset identification shift register counter
        idCount = 0;
        
        amiga->putMessage(MSG_DRIVE_LED_OFF, nr);
        amiga->putMessage(MSG_DRIVE_MOTOR_OFF, nr);
    }
    
    motor = value;
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
Drive::writeHead(uint8_t value)
{
    if (disk) {
        disk->writeHead(value, head.cylinder, head.side, head.offset);
    }
}

void
Drive::rotate()
{
    // debug("head = %d\n", head.offset);
    
    if (++head.offset == disk->trackLen) {
        
        // Start over at the beginning of the current cyclinder
        head.offset = 0;
        
        /* If this drive is currently selected, we emulate a falling edge on
         * the flag pin of CIA A. This causes the CIA to trigger the INDEX
         * interrupt if the corresponding enable bit is set.
         */
        if (isSelected()) amiga->ciaA.emulateFallingEdgeOnFlagPin();
    }
    assert(head.offset < disk->trackLen);
}

void
Drive::findSyncMark()
{
    for (unsigned i = 0; i < disk->trackLen; i++, rotate()) {
        
        if (readHead() != 0x44) continue;
        if (readHead() != 0x89) continue;
        break;
    }
}

void
Drive::moveHead(int dir)
{
    // Update disk change signal
    if (hasDisk()) dskchange = true;
    
    if (dir) {
        
        // Move drive head outwards (towards the lower tracks)
        if (head.cylinder > 0) head.cylinder--;
        // debug("[%lld] Moving down to cylinder %d\n", amiga->agnus.frame, head.cylinder);
        // if (nr == 0) plainmsg("Df%d cylinder %d\n", nr, head.cylinder);
    
    } else {
        
        // Move drive head inwards (towards the upper tracks)
        if (head.cylinder < 79) head.cylinder++;
        // debug("[%lld] Moving up to cylinder %d\n", amiga->agnus.frame, head.cylinder);
        // if (nr == 0) plainmsg("Df%d cylinder %d\n", nr, head.cylinder);
    }
    
    head.offset = 0; 
    // findSyncMark(); // REMOVE ASAP
    
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
Drive::setWriteProtection(bool value)
{
    if (disk) {
        
        if (value && !disk->isWriteProtected()) {
            
            disk->setWriteProtection(true);
            amiga->putMessage(MSG_DRIVE_DISK_PROTECTED);
        }
        if (!value && disk->isWriteProtected()) {
            
            disk->setWriteProtection(false);
            amiga->putMessage(MSG_DRIVE_DISK_UNPROTECTED);
        }
    }
}

void
Drive::toggleWriteProtection()
{
    if (hasWriteProtectedDisk()) setWriteProtection(false);
    if (hasWriteEnabledDisk())   setWriteProtection(true);
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
Drive::PRBdidChange(uint8_t oldValue, uint8_t newValue)
{
    // -----------------------------------------------------------------
    // | /MTR  | /SEL3 | /SEL2 | /SEL1 | /SEL0 | /SIDE |  DIR  | STEP  |
    // -----------------------------------------------------------------

    bool oldMtr = oldValue & 0x80;
    bool oldSel = oldValue & (0b1000 << nr);
    bool oldStep = oldValue & 0x01;

    bool newMtr = newValue & 0x80;
    bool newSel = newValue & (0b1000 << nr);
    bool newStep = newValue & 0x01;
    
    bool newDir = newValue & 0x02;
    
    // Store a copy of the new PRB value
    prb = newValue;
    
    //
    // Drive motor
    //

    // The motor state can only change on a falling edge on the select line
    if (FALLING_EDGE(oldSel, newSel)) {
        
        // Emulate the indentification shift register
        idCount = (idCount + 1) % 32;
        idBit = !!GET_BIT(getDriveId(), 31 - idCount);
        
        // Drive motor logic from SAE / UAE
        if (!oldMtr || !newMtr) {
            switchMotorOn();
        } else if (oldMtr) {
            switchMotorOff();
        }
        
        // plaindebug("disk.select() sel df%d ($%08X) [$%08x, bit #%02d: %d]\n",
        //       nr, getDriveId(), getDriveId() << idCount, 31 - idCount, idBit);
    }
    
    //
    // Drive head
    //
    
    // Move head if STEP goes high and drive was already selected
    if (RISING_EDGE(oldStep, newStep) && !oldSel) moveHead(newDir);
    
    // Evaluate the side selection bit
    if (head.side != !(newValue & 0b100)) {
        // debug("Switching to side %d\n", !(newValue & 0b100));
    }
    head.side = !(newValue & 0b100);
        
 
}
