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
    plainmsg("             Nr: %d\n", nr);
    plainmsg("           Type: %s\n", driveTypeName(type));
    plainmsg("          Speed: %d\n", speed);
    plainmsg("       Id count: %d\n", idCount);
    plainmsg("         Id bit: %d\n", idBit);
    plainmsg("          Motor: %s\n", motor ? "on" : "off");
    plainmsg("      dskchange: %d\n", dskchange);
    plainmsg("         dsklen: %X\n", dsklen);
    plainmsg("            prb: %X\n", prb);
    plainmsg("           Side: %d\n", head.side);
    plainmsg("      Cyclinder: %d\n", head.cylinder);
    plainmsg("         Offset: %d\n", head.offset);
    plainmsg("cylinderHistory: %X\n", cylinderHistory);
    plainmsg("           Disk: %s\n", disk ? "yes" : "no");
}

size_t
Drive::_size()
{
    SerCounter counter;

    applyToPersistentItems(counter);
    applyToResetItems(counter);

    // Add the size of the boolean indicating whether a disk is inserted
    counter.count += sizeof(bool);

    if (hasDisk()) {

        // Add the disk type and disk state
        counter & disk->getType();
        disk->applyToPersistentItems(counter);
    }

    return counter.count;
}

size_t
Drive::_load(uint8_t *buffer)
{
    SerReader reader(buffer);
    bool diskInSnapshot;
    DiskType diskType;

    // Read own state
    applyToPersistentItems(reader);
    applyToResetItems(reader);

    // Check if a disk is attached to this snapshot
    reader & diskInSnapshot;

    // Create the disk
    if (diskInSnapshot) {

        // Delete the old disk if present
        if (disk) delete disk;

        reader & diskType;
        disk = Disk::makeWithReader(reader, diskType);
    }

    debug(SNAP_DEBUG, "Recreated from %d bytes\n", reader.ptr - buffer);
    return reader.ptr - buffer;
}

size_t
Drive::_save(uint8_t *buffer)
{
    SerWriter writer(buffer);

    // Write own state
    applyToPersistentItems(writer);
    applyToResetItems(writer);

    // Indicate whether this drive has a disk is inserted
    writer & hasDisk();

    if (hasDisk()) {

        // Write the disk type
        writer & disk->getType();

        // Write the disk's state
        disk->applyToPersistentItems(writer);
    }

    debug(SNAP_DEBUG, "Serialized to %d bytes\n", writer.ptr - buffer);
    return writer.ptr - buffer;
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
    assert(speed == 1 ||
           speed == 2 ||
           speed == 4 ||
           speed == 8 ||
           speed == 65535);
    
    speed = value;
    
    debug("Setting acceleration factor to %d\n", value);

}

uint32_t
Drive::getDriveId()
{
    /* External floopy drives identify themselve with the following codes:
     *
     *   3.5" DD :  0xFFFFFFFF
     *   3.5" HD :  0xAAAAAAAA  if an HD disk is inserted
     *              0xFFFFFFFF  if no disk or a DD disk is inserted
     *   5.25"SD :  0x55555555
     *
     * An unconnected drive corresponds to ID 0x00000000. The internal drive
     * does not identify itself. It's ID is also read as 0x00000000.
     */
    
    assert(type == DRIVE_35_DD);

    if (nr == 0) {
        return 0x00000000;
    } else {
        return 0xFFFFFFFF;
    }
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
        // debug("ID mode is %s\n", idMode() ? "on" : "off");
        if (idMode()) {
            if (idBit) result &= 0b11011111;
        } else {
            if (motorAtFullSpeed() && hasDisk()) result &= 0b11011111;
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
        
        motorOnCycle = amiga->getMasterClock();

        debug(DSK_DEBUG, "Motor on (Cycle: %d)\n", motorOnCycle);

        amiga->putMessage(MSG_DRIVE_LED_ON, nr);
        amiga->putMessage(MSG_DRIVE_MOTOR_ON, nr);
    }
    
    else if (motor && !value) {


        idCount = 0; // Reset identification shift register counter
        motorOffCycle = amiga->getMasterClock();

        debug(DSK_DEBUG, "Motor off (Cycle: %d)\n", motorOffCycle);

        amiga->putMessage(MSG_DRIVE_LED_OFF, nr);
        amiga->putMessage(MSG_DRIVE_MOTOR_OFF, nr);
    }
    
    motor = value;
}

Cycle
Drive::motorOnTime()
{
    return motor ? amiga->getMasterClock() - motorOnCycle : 0;
}

Cycle
Drive::motorOffTime()
{
    return motor ? 0 : (amiga->getMasterClock() - motorOffCycle);
}

bool
Drive::motorAtFullSpeed()
{
    Cycle delay = 380 * 28000; // 380 msec
    return isOriginalDrive() ? (motorOnTime() > delay) : motor;
}

bool
Drive::motorStopped()
{
    Cycle delay = 0;
    return isOriginalDrive() ? (motorOffTime() > delay) : !motor;
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
    uint8_t result = 0xFF;
    
    if (disk) {
        result = disk->readByte(head.cylinder, head.side, head.offset);
    }
    rotate();

    return result;
}

uint16_t
Drive::readHead16()
{
    uint8_t byte1 = readHead();
    uint8_t byte2 = readHead();
    
    return HI_LO(byte1, byte2);
}

void
Drive::writeHead(uint8_t value)
{
    if (disk) {
        disk->writeByte(value, head.cylinder, head.side, head.offset);
    }
    rotate();
}

void
Drive::writeHead16(uint16_t value)
{
    writeHead(HI_BYTE(value));
    writeHead(LO_BYTE(value));
}

void
Drive::rotate()
{
    if (++head.offset == disk->trackSize) {
        
        // Start over at the beginning of the current cyclinder
        head.offset = 0;

        /* If this drive is currently selected, we emulate a falling edge on
         * the flag pin of CIA B. This causes the CIA to trigger the INDEX
         * interrupt if the corresponding enable bit is set.
         */
        if (isSelected()) {
            // debug("emulateFallingEdgeOnFlagPin()\n"); 
            amiga->ciaB.emulateFallingEdgeOnFlagPin();
        }
    }

    assert(head.offset < Disk::trackSize);
}

void
Drive::findSyncMark()
{
    for (unsigned i = 0; i < disk->trackSize; i++) {
        
        if (readHead() != 0x44) continue;
        if (readHead() != 0x89) continue;
        break;
    }

    debug(DSK_DEBUG, "Moving to SYNC mark at offset %d\n", head.offset);
}

void
Drive::moveHead(int dir)
{
    // Update disk change signal
    if (hasDisk()) dskchange = true;
    
    if (dir) {
        
        // Move drive head outwards (towards the lower tracks)
        if (head.cylinder > 0) {
            head.cylinder--;
            recordCylinder(head.cylinder);
        }
        plaindebug(MIN(DSK_DEBUG, DSK_CHECKSUM), "Stepping down to cylinder %d\n", head.cylinder);

    } else {
        
        // Move drive head inwards (towards the upper tracks)
        if (head.cylinder < 79) {
            head.cylinder++;
            recordCylinder(head.cylinder);
        }
        plaindebug(MIN(DSK_DEBUG, DSK_CHECKSUM), "Stepping up to cylinder %d\n", head.cylinder);
    }

#ifdef ALIGN_DRIVE_HEAD
    head.offset = 0; 
#endif
    
    // Inform the GUI
    amiga->putMessage(pollsForDisk() ? MSG_DRIVE_HEAD_POLL : MSG_DRIVE_HEAD);
}

void
Drive::recordCylinder(uint8_t cylinder)
{
    cylinderHistory = (cylinderHistory << 8) | cylinder;
}

bool
Drive::pollsForDisk()
{
    // Disk polling is only performed if no disk is inserted
    if (hasDisk()) return false;

    /* Head polling sequences of different Kickstart versions:
     *
     * Kickstart 1.2 and 1.3: 0-1-0-1-0-1-...
     * Kickstart 2.0:         0-1-2-3-2-1-...
     */
    static const uint64_t signature[] = {

        // Kickstart 1.2 and 1.3
        0x010001000100,
        0x000100010001,

        // Kickstart 2.0
        0x020302030203,
        0x030203020302,
    };

    uint64_t mask = 0xFFFFFFFF;
    for (unsigned i = 0; i < sizeof(signature) / 8; i++) {
        if ((cylinderHistory & mask) == (signature[i] & mask)) return true;
    }

    return false;
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
    amiga->suspend();

    if (disk) {
        
        // Flag disk change in the CIAA::PA
        dskchange = false;
        
        // Get rid of the disk
        delete disk;
        disk = NULL;
        
        // Notify the GUI
        amiga->putMessage(MSG_DRIVE_DISK_EJECT, nr);
    }

    amiga->resume();
}

void
Drive::insertDisk(Disk *disk)
{
    amiga->suspend();

    if (disk) {
        
        ejectDisk();
        this->disk = disk;
        amiga->putMessage(MSG_DRIVE_DISK_INSERT, nr);
    }

    amiga->resume();
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
