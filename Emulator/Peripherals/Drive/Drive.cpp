// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Drive.h"
#include "Agnus.h"
#include "BootBlockImage.h"
#include "CIA.h"
#include "DiskFile.h"
#include "FSDevice.h"
#include "MsgQueue.h"

Drive::Drive(Amiga& ref, isize n) : SubComponent(ref), nr(n)
{
    assert(nr < 4);
}

const char *
Drive::getDescription() const
{
    assert(nr <= 3);
    return nr == 0 ? "Df0" : nr == 1 ? "Df1" : nr == 2 ? "Df2" : "Df3";
}

void
Drive::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    if (hard) assert(diskToInsert == nullptr);
}

DriveConfig
Drive::getDefaultConfig(isize nr)
{
    DriveConfig defaults;
    
    defaults.type = DRIVE_DD_35;
    defaults.mechanicalDelays = true;
    defaults.startDelay = MSEC(380);
    defaults.stopDelay = MSEC(80);
    defaults.stepDelay = USEC(8000);
    defaults.diskSwapDelay = SEC(1.8);
    defaults.pan = IS_EVEN(nr) ? 100 : -100;
    defaults.stepVolume = 128;
    defaults.pollVolume = 128;
    defaults.insertVolume = 128;
    defaults.ejectVolume = 128;
    defaults.defaultFileSystem = FS_OFS;
    defaults.defaultBootBlock = BB_NONE;

    return defaults;
}

void
Drive::resetConfig()
{
    auto defaults = getDefaultConfig(nr);
    
    setConfigItem(OPT_DRIVE_TYPE, defaults.type);
    setConfigItem(OPT_EMULATE_MECHANICS, defaults.mechanicalDelays);
    setConfigItem(OPT_START_DELAY, defaults.startDelay);
    setConfigItem(OPT_STOP_DELAY, defaults.stopDelay);
    setConfigItem(OPT_STEP_DELAY, defaults.stepDelay);
    setConfigItem(OPT_DISK_SWAP_DELAY, defaults.diskSwapDelay);
    setConfigItem(OPT_DRIVE_PAN, defaults.pan);
    setConfigItem(OPT_STEP_VOLUME, defaults.stepVolume);
    setConfigItem(OPT_POLL_VOLUME, defaults.pollVolume);
    setConfigItem(OPT_INSERT_VOLUME, defaults.insertVolume);
    setConfigItem(OPT_EJECT_VOLUME, defaults.ejectVolume);
    setConfigItem(OPT_DEFAULT_FILESYSTEM, defaults.defaultFileSystem);
    setConfigItem(OPT_DEFAULT_BOOTBLOCK, defaults.defaultBootBlock);
}

i64
Drive::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_DRIVE_TYPE:          return (long)config.type;
        case OPT_EMULATE_MECHANICS:   return (long)config.mechanicalDelays;
        case OPT_START_DELAY:         return (long)config.startDelay;
        case OPT_STOP_DELAY:          return (long)config.stopDelay;
        case OPT_STEP_DELAY:          return (long)config.stepDelay;
        case OPT_DISK_SWAP_DELAY:     return (long)config.diskSwapDelay;
        case OPT_DRIVE_PAN:           return (long)config.pan;
        case OPT_STEP_VOLUME:         return (long)config.stepVolume;
        case OPT_POLL_VOLUME:         return (long)config.pollVolume;
        case OPT_INSERT_VOLUME:       return (long)config.insertVolume;
        case OPT_EJECT_VOLUME:        return (long)config.ejectVolume;
        case OPT_DEFAULT_FILESYSTEM:  return (long)config.defaultFileSystem;
        case OPT_DEFAULT_BOOTBLOCK:   return (long)config.defaultBootBlock;

        default:
            fatalError;
    }
}

void
Drive::setConfigItem(Option option, i64 value)
{
    switch (option) {
                            
        case OPT_DRIVE_TYPE:
            
            if (!DriveTypeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, DriveTypeEnum::keyList());
            }
            if (value != DRIVE_DD_35 && value != DRIVE_HD_35) {
                throw VAError(ERROR_OPT_UNSUPPORTED);
            }
            
            config.type = (DriveType)value;
            return;

        case OPT_EMULATE_MECHANICS:
        
            config.mechanicalDelays = value;
            return;

        case OPT_START_DELAY:
        
            config.startDelay = value;
            return;

        case OPT_STOP_DELAY:
        
            config.stopDelay = value;
            return;

        case OPT_STEP_DELAY:
        
            config.stepDelay = value;
            return;

        case OPT_DISK_SWAP_DELAY:
        
            config.diskSwapDelay = value;
            return;

        case OPT_DRIVE_PAN:

            config.pan = (i16)value;
            return;

        case OPT_STEP_VOLUME:

            config.stepVolume = (u8)value;
            return;

        case OPT_POLL_VOLUME:

            config.pollVolume = (u8)value;
            return;

        case OPT_EJECT_VOLUME:

            config.ejectVolume = (u8)value;
            return;

        case OPT_INSERT_VOLUME:

            config.insertVolume = (u8)value;
            return;

        case OPT_DEFAULT_FILESYSTEM:

            if (!FSVolumeTypeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, FSVolumeTypeEnum::keyList());
            }
            
            config.defaultFileSystem = (FSVolumeType)value;
            return;

        case OPT_DEFAULT_BOOTBLOCK:

            if (!BootBlockIdEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, BootBlockIdEnum::keyList());
            }
            
            config.defaultBootBlock = (BootBlockId)value;
            return;

        default:
            fatalError;
    }
}

void
Drive::_inspect() const
{
    synchronized {
        
        info.head = head;
        info.hasDisk = hasDisk();
        info.motor = getMotor();
    }
}

void
Drive::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
        os << tab("Type");
        os << DriveTypeEnum::key(config.type) << std::endl;
        os << tab("Emulate mechanics");
        os << bol(config.mechanicalDelays) << std::endl;
        os << tab("Start delay");
        os << dec(config.startDelay) << std::endl;
        os << tab("Stop delay");
        os << dec(config.stopDelay) << std::endl;
        os << tab("Step delay");
        os << dec(config.stepDelay) << std::endl;
        os << tab("Disk swap delay");
        os << dec(config.diskSwapDelay) << std::endl;
        os << tab("Insert volume");
        os << dec(config.insertVolume) << std::endl;
        os << tab("Eject volume");
        os << dec(config.ejectVolume) << std::endl;
        os << tab("Step volume");
        os << dec(config.stepVolume) << std::endl;
        os << tab("Poll volume");
        os << dec(config.pollVolume) << std::endl;
        os << tab("Pan");
        os << dec(config.pan) << std::endl;
        os << tab("Default file system");
        os << FSVolumeTypeEnum::key(config.defaultFileSystem) << std::endl;
        os << tab("Default boot block");
        os << BootBlockIdEnum::key(config.defaultBootBlock) << std::endl;
        os << tab("Search path");
        os << "\"" << searchPath << "\"" << std::endl;
    }
    
    if (category & dump::State) {
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
        os << tab("Id count");
        os << dec(idCount) << std::endl;
        os << tab("Id bit");
        os << dec(idBit) << std::endl;
        os << tab("motorSpeed()");
        os << motorSpeed() << std::endl;
        os << tab("getMotor()");
        os << bol(getMotor()) << std::endl;
        os << tab("motorSpeedingUp()");
        os << bol(motorSpeedingUp()) << std::endl;
        os << tab("motorAtFullSpeed()");
        os << bol(motorAtFullSpeed()) << std::endl;
        os << tab("motorSlowingDown()");
        os << bol(motorSlowingDown()) << std::endl;
        os << tab("motorStopped()");
        os << bol(motorStopped()) << std::endl;
        os << tab("dskchange");
        os << dec(dskchange) << std::endl;
        os << tab("dsklen");
        os << dec(dsklen) << std::endl;
        os << tab("prb");
        os << hex(prb) << std::endl;
        os << tab("Side");
        os << dec(head.side) << std::endl;
        os << tab("Cylinder");
        os << dec(head.cylinder) << std::endl;
        os << tab("Offset");
        os << dec(head.offset) << std::endl;
        os << tab("cylinderHistory");
        os << hex(cylinderHistory) << std::endl;
        os << tab("Disk");
        os << bol(disk != nullptr) << std::endl;
    }
}

isize
Drive::_size()
{
    util::SerCounter counter;

    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    // Add the size of the boolean indicating whether a disk is inserted
    counter.count += sizeof(bool);

    if (hasDisk()) {

        // Add the disk type and disk state
        counter << disk->getDiameter() << disk->getDensity();
        disk->applyToPersistentItems(counter);
    }

    return counter.count;
}

isize
Drive::_load(const u8 *buffer) 
{
    util::SerReader reader(buffer);
    isize result;
    
    // Read own state
    applyToPersistentItems(reader);
    applyToResetItems(reader);

    // Check if the snapshot includes a disk
    bool diskInSnapshot; reader << diskInSnapshot;
    
    if (diskInSnapshot) {
        
        DiskDiameter type;
        DiskDensity density;
        reader << type << density;
        disk = std::make_unique<Disk>(reader, type, density);

    } else {
        
        disk = nullptr;
    }

    result = (isize)(reader.ptr - buffer);
    trace(SNP_DEBUG, "Recreated from %ld bytes\n", result);
    return result;
}

isize
Drive::_save(u8 *buffer)
{
    util::SerWriter writer(buffer);
    isize result;
    
    // Write own state
    applyToPersistentItems(writer);
    applyToResetItems(writer);

    // Indicate whether this drive has a disk is inserted
    writer << hasDisk();

    if (hasDisk()) {

        // Write the disk type
        writer << disk->getDiameter() << disk->getDensity();

        // Write the disk's state
        disk->applyToPersistentItems(writer);
    }
    
    result = (isize)(writer.ptr - buffer);
    trace(SNP_DEBUG, "Serialized to %ld bytes\n", result);
    return result;
}

bool
Drive::idMode() const
{
    return motorStopped() || motorSpeedingUp();
}

u32
Drive::getDriveId() const
{
    if (nr > 0) {
        
        // External floopy drives identify themselve as follows:
        //
        //     3.5" DD: 0xFFFFFFFF
        //     3.5" HD: 0xAAAAAAAA if an HD disk is inserted
        //              0xFFFFFFFF if no disk or a DD disk is inserted
        //     5.25"SD: 0x55555555
        
        switch(config.type) {
                
            case DRIVE_DD_35:
                return 0xFFFFFFFF;
                
            case DRIVE_HD_35:
                if (disk && disk->getDensity() == DISK_HD) return 0xAAAAAAAA;
                return 0xFFFFFFFF;
                
            case DRIVE_DD_525:
                return 0x55555555;
                
            default:
                fatalError;
        }
    }
    
    // The internal floppy drive identifies itself as 0x00000000
    return 0x00000000;
}

u8
Drive::driveStatusFlags() const
{
    u8 result = 0xFF;
    
    if (isSelected()) {
        
        // PA5: /DSKRDY
        if (idMode()) {
            if (idBit) result &= 0b11011111;
        } else if (hasDisk()) {
            if (motorAtFullSpeed() || motorSlowingDown()) result &= 0b11011111;
        }
        
        // PA4: /DSKTRACK0
        // debug("Head is at cylinder %d\n", head.cylinder);
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

double
Drive::motorSpeed()const
{
    // Quick exit if mechanics is not emulated
    if (!config.mechanicalDelays) return motor ? 100.0 : 0.0;
    
    // Determine the elapsed cycles since the last motor change
    Cycle elapsed = agnus.clock - switchCycle;
    assert(elapsed >= 0);
    
    // Compute the current speed
    if (motor) {
        if (config.startDelay == 0) return 100.0;
        return std::min(switchSpeed + 100.0 * (elapsed / config.startDelay), 100.0);
    } else {
        if (config.stopDelay == 0) return 0.0;
        return std::max(switchSpeed - 100.0 * (elapsed / config.stopDelay), 0.0);
    }
}

void
Drive::setMotor(bool value)
{
    // Only proceed if motor state will change
    if (motor == value) return;
    
    // Switch motor state
    switchSpeed = motorSpeed();
    switchCycle = agnus.clock;
    motor = value;
    
    // Reset the identification bit counter if motor has been turned off
    idCount = 0;
    
    // Inform the GUI
    msgQueue.put(value ? MSG_DRIVE_LED_ON : MSG_DRIVE_LED_OFF, nr);
    msgQueue.put(value ? MSG_DRIVE_MOTOR_ON : MSG_DRIVE_MOTOR_OFF, nr);
    
    debug(DSK_DEBUG, "Motor %s [%d]\n", motor ? "on" : "off", idCount);
}

bool
Drive::motorSpeedingUp() const
{
    return motor && motorSpeed() < 100.0;
}

bool
Drive::motorAtFullSpeed() const
{
    return motorSpeed() == 100.0;
}

bool
Drive::motorSlowingDown() const
{
    return !motor && motorSpeed() > 0.0;
}

bool
Drive::motorStopped() const
{
    return motorSpeed() == 0.0;
}

void
Drive::selectSide(Side side)
{
    assert(side < 2);
    head.side = side;
}

u8
Drive::readByte() const
{
    // Case 1: No disk is inserted
    if (!disk) {
        return 0xFF;
    }

    // Case 2: A step operation is in progress
    if (config.mechanicalDelays && (agnus.clock - stepCycle) < config.stepDelay) {
        return (u8)rand(); // 0xFF;
    }
    
    // Case 3: Normal operation
    return disk->readByte(head.cylinder, head.side, head.offset);
}

u8
Drive::readByteAndRotate()
{
    u8 result = readByte();
    if (motor) rotate();
    return result;
}

u16
Drive::readWordAndRotate()
{
    u8 byte1 = readByteAndRotate();
    u8 byte2 = readByteAndRotate();
    
    return HI_LO(byte1, byte2);
}

void
Drive::writeByte(u8 value)
{
    if (disk) {
        disk->writeByte(value, head.cylinder, head.side, head.offset);
    }
}

void
Drive::writeByteAndRotate(u8 value)
{
    writeByte(value);
    if (motor) rotate();
}

void
Drive::writeWordAndRotate(u16 value)
{
    writeByteAndRotate(HI_BYTE(value));
    writeByteAndRotate(LO_BYTE(value));
}

void
Drive::rotate()
{
    long last = disk ? disk->length.cylinder[head.cylinder][head.side] : 12668;
    if (++head.offset >= last) {
        
        // Start over at the beginning of the current cylinder
        head.offset = 0;

        // If this drive is selected, we emulate a falling edge on the flag pin
        // of CIA B. This causes the CIA to trigger the INDEX interrupt if the
        // corresponding enable bit is set.
        if (isSelected()) ciab.emulateFallingEdgeOnFlagPin();
    }
}

void
Drive::findSyncMark()
{
    long length = disk->length.cylinder[head.cylinder][head.side];
    for (isize i = 0; i < length; i++) {
        
        if (readByteAndRotate() != 0x44) continue;
        if (readByteAndRotate() != 0x89) continue;
        break;
    }

    trace(DSK_DEBUG, "Moving to SYNC mark at offset %ld\n", head.offset);
}

bool
Drive::readyToStep() const
{
    if (config.mechanicalDelays) {
        return agnus.clock - stepCycle > 1060;
    } else {
        return true;
    }
}

void
Drive::step(isize dir)
{    
    // Update the disk change signal
    if (hasDisk()) dskchange = true;
 
    // Only proceed if the last head step was a while ago
    if (!readyToStep()) return;
    
    if (dir) {
        
        // Move drive head outwards (towards the lower tracks)
        if (head.cylinder > 0) {
            head.cylinder--;
            recordCylinder(head.cylinder);
        }
        debug(DSK_CHECKSUM, "Stepping down to cylinder %ld\n", head.cylinder);

    } else {
        
        // Move drive head inwards (towards the upper tracks)
        if (head.cylinder < 83) {
            head.cylinder++;
            recordCylinder(head.cylinder);
        }
        debug(DSK_CHECKSUM, "Stepping up to cylinder %ld\n", head.cylinder);
    }
    
    // Push drive head forward
    if (ALIGN_HEAD) head.offset = 0;
    
    // Notify the GUI
    if (pollsForDisk()) {
        msgQueue.put(MSG_DRIVE_POLL,
                     config.pan << 24 | config.pollVolume << 16 | head.cylinder << 8 | nr);
    } else {
        msgQueue.put(MSG_DRIVE_STEP,
                     config.pan << 24 | config.stepVolume << 16 | head.cylinder << 8 | nr);
    }
        
    // Remember when we've performed the step
    stepCycle = agnus.clock;
}

void
Drive::recordCylinder(Cylinder cylinder)
{
    cylinderHistory = (cylinderHistory << 8) | cylinder;
}

bool
Drive::pollsForDisk() const
{
    /* Disk polling mode is detected by analyzing the movement history that
     * has been recorded by recordCylinder()
     */
    
    // Disk polling is only performed if no disk is inserted
    if (hasDisk()) return false;

    /* Head polling sequences of different Kickstart versions:
     *
     * Kickstart 1.2 and 1.3: 0-1-0-1-0-1-...
     * Kickstart 2.0:         0-1-2-3-2-3-...
     */
    static constexpr u64 signature[4] = {

        // Kickstart 1.2 and 1.3
        0x010001000100,
        0x000100010001,

        // Kickstart 2.0
        0x020302030203,
        0x030203020302
    };

    u64 mask = 0xFFFFFFFF;
    for (isize i = 0; i < 4; i++) {
        if ((cylinderHistory & mask) == (signature[i] & mask)) return true;
    }

    return false;
}

bool
Drive::hasWriteEnabledDisk() const
{
    return hasDisk() ? !disk->isWriteProtected() : false;
}

bool
Drive::hasWriteProtectedDisk() const
{
    return hasDisk() ? disk->isWriteProtected() : false;
}

void
Drive::setWriteProtection(bool value)
{
    if (disk) {
        
        if (value && !disk->isWriteProtected()) {
            
            disk->setWriteProtection(true);
            msgQueue.put(MSG_DISK_PROTECT);
        }
        if (!value && disk->isWriteProtected()) {
            
            disk->setWriteProtection(false);
            msgQueue.put(MSG_DISK_UNPROTECT);
        }
    }
}

void
Drive::toggleWriteProtection()
{
    if (hasDisk()) {
        disk->setWriteProtection(!disk->isWriteProtected());
    }
}

bool
Drive::isInsertable(DiskDiameter t, DiskDensity d) const
{
    debug(DSK_DEBUG,
          "isInsertable(%s, %s)\n", DiskDiameterEnum::key(t), DiskDensityEnum::key(d));
    
    switch (config.type) {
            
        case DRIVE_DD_35:
            return t == INCH_35 && d == DISK_DD;
            
        case DRIVE_HD_35:
            return t == INCH_35;
            
        case DRIVE_DD_525:
            return t == INCH_525 && d == DISK_DD;
                        
        default:
            fatalError;
    }
}

bool
Drive::isInsertable(const DiskFile &file) const
{
    return isInsertable(file.getDiskDiameter(), file.getDiskDensity());
}

bool
Drive::isInsertable(const Disk &disk) const
{
    return isInsertable(disk.diameter, disk.density);
}

template <EventSlot s> void
Drive::ejectDisk(Cycle delay)
{
    debug(DSK_DEBUG, "ejectDisk <%d> (%lld)\n", s, delay);
    
    {   SUSPENDED

        // Schedule an ejection event
        agnus.scheduleRel <s> (delay, DCH_EJECT);

        // If there is no delay, service the event immediately
        if (delay == 0) serviceDiskChangeEvent <s> ();
    }
}

void
Drive::ejectDisk(Cycle delay)
{
    debug(DSK_DEBUG, "ejectDisk(%lld)\n", delay);
    
    if (nr == 0) ejectDisk <SLOT_DC0> (delay);
    if (nr == 1) ejectDisk <SLOT_DC1> (delay);
    if (nr == 2) ejectDisk <SLOT_DC2> (delay);
    if (nr == 3) ejectDisk <SLOT_DC3> (delay);
}

template <EventSlot s> void
Drive::insertDisk(std::unique_ptr<Disk> disk, Cycle delay)
{
    assert(disk != nullptr);
    
    debug(DSK_DEBUG, "insertDisk <%d> (%lld)\n", s, delay);

    // Only proceed if the provided disk is compatible with this drive
    if (!isInsertable(*disk)) throw VAError(ERROR_DISK_INCOMPATIBLE);

    {   SUSPENDED
        
        // Get ownership of the disk
        diskToInsert = std::move(disk);

        // Schedule an ejection event
        agnus.scheduleRel <s> (delay, DCH_INSERT);

        // If there is no delay, service the event immediately
        if (delay == 0) serviceDiskChangeEvent <s> ();
    }
}

void
Drive::insertDisk(std::unique_ptr<Disk> disk, Cycle delay)
{
    debug(DSK_DEBUG, "insertDisk(%lld)\n", delay);
    
    if (nr == 0) insertDisk <SLOT_DC0> (std::move(disk), delay);
    if (nr == 1) insertDisk <SLOT_DC1> (std::move(disk), delay);
    if (nr == 2) insertDisk <SLOT_DC2> (std::move(disk), delay);
    if (nr == 3) insertDisk <SLOT_DC3> (std::move(disk), delay);
}

void
Drive::insertNew()
{
    ADFFile adf;
    
    // Create a suitable ADF for this drive
    switch (config.type) {
            
        case DRIVE_DD_35: adf.init(INCH_35, DISK_DD); break;
        case DRIVE_HD_35: adf.init(INCH_35, DISK_HD); break;
        case DRIVE_DD_525: adf.init(INCH_525, DISK_SD); break;
    }
    
    // Add a file system
    adf.formatDisk(config.defaultFileSystem, config.defaultBootBlock);
    
    // Replace the current disk with the new one
    swapDisk(adf);
}

void
Drive::swapDisk(std::unique_ptr<Disk> disk)
{
    debug(DSK_DEBUG, "swapDisk()\n");
    
    // Only proceed if the provided disk is compatible with this drive
    if (!isInsertable(*disk)) throw VAError(ERROR_DISK_INCOMPATIBLE);

    // Determine delay (in pause mode, we insert immediately)
    auto delay = isRunning() ? config.diskSwapDelay : 0;
        
    {   SUSPENDED

        if (hasDisk()) {

            // Eject the old disk first
            ejectDisk();

        } else {

            // Insert the new disk immediately
            delay = 0;
        }
                
        // Insert the new disk with a delay
        insertDisk(std::move(disk), delay);
    }
}

void
Drive::swapDisk(class DiskFile &file)
{
    swapDisk(std::make_unique<Disk>(file));
}

void
Drive::swapDisk(const string &name)
{
    bool append = !util::isAbsolutePath(name) && searchPath != "";
    string path = append ? searchPath + "/" + name : name;
    
    std::unique_ptr<DiskFile> file(DiskFile::make(path));
    swapDisk(*file);
}

u64
Drive::fnv() const
{
    return disk ? disk->getFnv() : 0;
}

template <EventSlot s> void
Drive::serviceDiskChangeEvent()
{
    // Check if we need to eject the current disk
    if (scheduler.id[s] == DCH_EJECT || scheduler.id[s] == DCH_INSERT) {
        
        if (disk) {
            
            // Flag disk change in CIAA::PA
            dskchange = false;
            
            // Get rid of the disk
            disk = nullptr;
            
            // Notify the GUI
            msgQueue.put(MSG_DISK_EJECT,
                         config.pan << 24 | config.ejectVolume << 16 | nr);
        }
    }
    
    // Check if we need to insert a new disk
    if (scheduler.id[s] == DCH_INSERT) {
        
        if (diskToInsert) {
            
            // Insert the new disk
            disk = std::move(diskToInsert);
            
            // Remove indeterminism by repositioning the drive head
            head.offset = 0;
            
            // Notify the GUI
            msgQueue.put(MSG_DISK_INSERT,
                         config.pan << 24 | config.insertVolume << 16 | nr);
        }
    }

    // Remove the event
    scheduler.cancel <s> ();
}

void
Drive::PRBdidChange(u8 oldValue, u8 newValue)
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
        
        // Emulate the identification shift register
        idCount = (idCount + 1) % 32;
        idBit = GET_BIT(getDriveId(), 31 - idCount);
        
        // Drive motor logic from SAE / UAE
        if (!oldMtr || !newMtr) {
            switchMotorOn();
        } else if (oldMtr) {
            switchMotorOff();
        }
    }
    
    //
    // Drive head
    //
    
    // Move head if STEP goes high and drive was selected
    if (RISING_EDGE(oldStep, newStep) && !oldSel) step(newDir);
    
    // Evaluate the side selection bit
    selectSide((newValue & 0b100) ? 0 : 1);
}
