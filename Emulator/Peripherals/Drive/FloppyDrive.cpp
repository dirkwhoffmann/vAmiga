// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FloppyDrive.h"
#include "Amiga.h"
#include "BootBlockImage.h"
#include "DiskController.h"
#include "FloppyFile.h"
#include "MutableFileSystem.h"
#include "MsgQueue.h"
#include "OSDescriptors.h"

namespace vamiga {

FloppyDrive::FloppyDrive(Amiga& ref, isize nr) : Drive(ref, nr)
{
    string path;
    
    if (nr == 0) path = INITIAL_DF0;
    if (nr == 1) path = INITIAL_DF1;
    if (nr == 2) path = INITIAL_DF2;
    if (nr == 3) path = INITIAL_DF3;
    
    if (path != "") {
        
        try {
            
            auto adf = ADFFile(path);
            disk = std::make_unique<FloppyDisk>(adf);
            
        } catch (...) {
            
            warn("Cannot open ADF file %s\n", path.c_str());
        }
    }
}

const char *
FloppyDrive::getDescription() const
{
    assert(usize(nr) < 4);
    return nr == 0 ? "Df0" : nr == 1 ? "Df1" : nr == 2 ? "Df2" : "Df3";
}

void
FloppyDrive::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    if (hard) assert(diskToInsert == nullptr);
}

void
FloppyDrive::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    std::vector <Option> options = {
        
        OPT_DRIVE_TYPE,
        OPT_DRIVE_MECHANICS,
        OPT_DRIVE_RPM,
        OPT_DISK_SWAP_DELAY,
        OPT_DRIVE_PAN,
        OPT_STEP_VOLUME,
        OPT_POLL_VOLUME,
        OPT_INSERT_VOLUME,
        OPT_EJECT_VOLUME
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option, nr));
    }
}

i64
FloppyDrive::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_DRIVE_TYPE:          return (long)config.type;
        case OPT_DRIVE_MECHANICS:     return (long)config.mechanics;
        case OPT_DRIVE_RPM:           return (long)config.rpm;
        case OPT_DISK_SWAP_DELAY:     return (long)config.diskSwapDelay;
        case OPT_DRIVE_PAN:           return (long)config.pan;
        case OPT_STEP_VOLUME:         return (long)config.stepVolume;
        case OPT_POLL_VOLUME:         return (long)config.pollVolume;
        case OPT_INSERT_VOLUME:       return (long)config.insertVolume;
        case OPT_EJECT_VOLUME:        return (long)config.ejectVolume;

        default:
            fatalError;
    }
}

void
FloppyDrive::setConfigItem(Option option, i64 value)
{
    switch (option) {

        case OPT_DRIVE_TYPE:
            
            if (!FloppyDriveTypeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, FloppyDriveTypeEnum::keyList());
            }
            if (value != DRIVE_DD_35 && value != DRIVE_HD_35) {
                throw VAError(ERROR_OPT_UNSUPPORTED);
            }
            
            config.type = (FloppyDriveType)value;
            return;

        case OPT_DRIVE_MECHANICS:

            if (!DriveMechanicsEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, DriveMechanicsEnum::keyList());
            }

            config.mechanics = (DriveMechanics)value;
            return;

        case OPT_DRIVE_RPM:

            config.rpm = value;
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

        default:
            fatalError;
    }
}

void
FloppyDrive::_inspect() const
{
    {   SYNCHRONIZED
        
        info.head = head;
        info.hasDisk = hasDisk();
        info.motor = getMotor();
    }
}

void
FloppyDrive::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
        os << tab("Type");
        os << FloppyDriveTypeEnum::key(config.type) << std::endl;
        os << tab("Mechanics");
        os << DriveMechanicsEnum::key(config.mechanics) << std::endl;
        os << tab("Revolutions per minute");
        os << dec(config.rpm) << std::endl;
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
        os << tab("Search path");
        os << "\"" << searchPath << "\"" << std::endl;

        os << std::endl;
        os << tab("Start delay");
        os << dec(AS_MSEC(getStartDelay())) << " msec" << std::endl;
        os << tab("Stop delay");
        os << dec(AS_MSEC(getStopDelay())) << " msec" << std::endl;
        os << tab("Step pulse delay");
        os << dec(AS_USEC(getStepPulseDelay())) << " usec" << std::endl;
        os << tab("Reverse step pulse delay");
        os << dec(AS_USEC(getRevStepPulseDelay())) << " usec" << std::endl;
        os << tab("Track to track delay");
        os << dec(AS_MSEC(getTrackToTrackDelay())) << " msec" << std::endl;
        os << tab("Head settle time");
        os << dec(AS_MSEC(getHeadSettleTime())) << " msec" << std::endl;
    }
    
    if (category == Category::State) {
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
        os << tab("dskchange");
        os << dec(dskchange) << std::endl;
        os << tab("dsklen");
        os << dec(dsklen) << std::endl;
        os << tab("prb");
        os << hex(prb) << std::endl;
        os << tab("Drive head");
        os << dec(head.cylinder) << ":";
        os << dec(head.head) << ":";
        os << dec(head.offset) << std::endl;
        os << tab("Disk");
        os << bol(disk != nullptr) << std::endl;
        os << tab("Modified");
        os << bol(hasModifiedDisk()) << std::endl;
        os << tab("Id count");
        os << dec(idCount) << std::endl;
        os << tab("Id bit");
        os << dec(idBit) << std::endl;

        os << std::endl;
        os << tab("latestStepUp");
        os << dec(latestStepUp) << std::endl;
        os << tab("latestStepDown");
        os << dec(latestStepDown) << std::endl;
        os << tab("latestStep");
        os << dec(latestStep) << std::endl;
        os << tab("latestStepCompleted");
        os << dec(latestStepCompleted) << std::endl;
        os << tab("cylinderHistory");
        os << hex(cylinderHistory) << std::endl;

        os << std::endl;
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
        os << tab("readyToStepUp()");
        os << bol(readyToStepUp()) << std::endl;
        os << tab("readyToStepDown()");
        os << bol(readyToStepDown()) << std::endl;
    }

    if (category == Category::Disk) {

        if (hasDisk()) {

            disk->_dump(Category::State, os);

        } else {

            os << "No disk in drive" << std::endl;
        }
    }
}

isize
FloppyDrive::_size()
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
FloppyDrive::_load(const u8 *buffer) 
{
    util::SerReader reader(buffer);
    isize result;
    
    // Read own state
    applyToPersistentItems(reader);
    applyToResetItems(reader);

    // Check if the snapshot includes a disk
    bool diskInSnapshot; reader << diskInSnapshot;
    
    if (diskInSnapshot) {
        
        Diameter type;
        Density density;
        reader << type << density;
        disk = std::make_unique<FloppyDisk>(reader, type, density);

    } else {
        
        disk = nullptr;
    }

    result = (isize)(reader.ptr - buffer);
    trace(SNP_DEBUG, "Recreated from %ld bytes\n", result);
    return result;
}

isize
FloppyDrive::_save(u8 *buffer)
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
FloppyDrive::isConnected() const
{
    return diskController.getConfigItem(OPT_DRIVE_CONNECT, nr);
}

bool
FloppyDrive::hasDisk() const
{
    return disk != nullptr;
}

bool
FloppyDrive::hasModifiedDisk() const
{
    return hasDisk() ? disk->isModified() : false;
}

bool
FloppyDrive::hasProtectedDisk() const
{
    return hasDisk() ? disk->isWriteProtected() : false;
}

void
FloppyDrive::setModificationFlag(bool value)
{
    if (disk) disk->setModified(value);
}

void
FloppyDrive::setProtectionFlag(bool value)
{
    if (disk) {
        
        if (value && !disk->isWriteProtected()) {
            
            disk->setWriteProtection(true);
            msgQueue.put(MSG_DISK_PROTECTED, true);
        }
        if (!value && disk->isWriteProtected()) {
            
            disk->setWriteProtection(false);
            msgQueue.put(MSG_DISK_PROTECTED, false);
        }
    }
}

u32
FloppyDrive::getDriveId() const
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
                if (disk && disk->getDensity() == DENSITY_HD) return 0xAAAAAAAA;
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

bool
FloppyDrive::idMode() const
{
    return motorStopped() || motorSpeedingUp();
}

bool
FloppyDrive::isWriting() const
{
    return motor && isSelected() && diskController.getState() == DRIVE_DMA_WRITE;
}

Cycle
FloppyDrive::getStartDelay() const
{
    switch (config.mechanics) {

        case MECHANICS_NONE:    return 0;
        case MECHANICS_A1010:   return MSEC(380);

        default:
            fatalError;
    }
}

Cycle
FloppyDrive::getStopDelay() const
{
    switch (config.mechanics) {

        case MECHANICS_NONE:    return 0;
        case MECHANICS_A1010:   return MSEC(80);

        default:
            fatalError;
    }
}

Cycle
FloppyDrive::getStepPulseDelay() const
{
    switch (config.mechanics) {

        case MECHANICS_NONE:    return 0;
        case MECHANICS_A1010:   return USEC(40);

        default:
            fatalError;
    }
}

Cycle
FloppyDrive::getRevStepPulseDelay() const
{
    switch (config.mechanics) {

        case MECHANICS_NONE:    return 0;
        case MECHANICS_A1010:   return USEC(40);

        default:
            fatalError;
    }
}

Cycle
FloppyDrive::getTrackToTrackDelay() const
{
    switch (config.mechanics) {

        case MECHANICS_NONE:    return 0;
        case MECHANICS_A1010:   return MSEC(3);

        default:
            fatalError;
    }
}

Cycle
FloppyDrive::getHeadSettleTime() const
{
    switch (config.mechanics) {

        case MECHANICS_NONE:    return 0;
        case MECHANICS_A1010:   return MSEC(9);

        default:
            fatalError;
    }
}

u8
FloppyDrive::driveStatusFlags() const
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
        if (head.cylinder == 0) { result &= 0b11101111; }
        
        // PA3: /DSKPROT
        if (!hasUnprotectedDisk()) { result &= 0b11110111; }
        
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
FloppyDrive::motorSpeed() const
{
    if (motor) {

        // Case 1: Motor speeds up

        auto startDelay = getStartDelay();
        if (startDelay == 0) return 100.0;

        Cycle elapsed = agnus.clock - switchCycle;
        return std::min(switchSpeed + 100.0 * (elapsed / startDelay), 100.0);

    } else {

        // Case 2: Motor slows down

        auto stopDelay = getStopDelay();
        if (stopDelay == 0) return 0.0;

        Cycle elapsed = agnus.clock - switchCycle;
        return std::max(switchSpeed - 100.0 * (elapsed / stopDelay), 0.0);
    }
}

void
FloppyDrive::setMotor(bool value)
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
    msgQueue.put(MSG_DRIVE_LED, DriveMsg { i16(nr), value, 0, 0 });
    msgQueue.put(MSG_DRIVE_MOTOR, DriveMsg { i16(nr), value, 0, 0 });

    // Enable or disable warp mode if applicable
    amiga.updateWarpState();

    debug(DSK_DEBUG, "Motor %s [%d]\n", motor ? "on" : "off", idCount);
}

bool
FloppyDrive::motorSpeedingUp() const
{
    return motor && motorSpeed() < 100.0;
}

bool
FloppyDrive::motorAtFullSpeed() const
{
    return motorSpeed() == 100.0;
}

bool
FloppyDrive::motorSlowingDown() const
{
    return !motor && motorSpeed() > 0.0;
}

bool
FloppyDrive::motorStopped() const
{
    return motorSpeed() == 0.0;
}

void
FloppyDrive::selectSide(Head h)
{
    assert(h == 0 || h == 1);
    head.head = h;
}

u8
FloppyDrive::readByte() const
{
    // Case 1: No disk is inserted
    if (!disk) return 0xFF;

    // Case 2: A step operation is in progress
    if (agnus.clock < latestStepCompleted) return u8(rand() & 0x55);
    
    // Case 3: Normal operation
    return disk->readByte(head.cylinder, head.head, head.offset);
}

u8
FloppyDrive::readByteAndRotate()
{
    u8 result = readByte();
    if (motor) rotate();
    return result;
}

u16
FloppyDrive::readWordAndRotate()
{
    u8 byte1 = readByteAndRotate();
    u8 byte2 = readByteAndRotate();
    
    return HI_LO(byte1, byte2);
}

void
FloppyDrive::writeByte(u8 value)
{
    if (disk) {
        disk->writeByte(head.cylinder, head.head, head.offset, value);
    }
}

void
FloppyDrive::writeByteAndRotate(u8 value)
{
    writeByte(value);
    if (motor) rotate();
}

void
FloppyDrive::writeWordAndRotate(u16 value)
{
    writeByteAndRotate(HI_BYTE(value));
    writeByteAndRotate(LO_BYTE(value));
}

void
FloppyDrive::rotate()
{
    long last = disk ? disk->length.cylinder[head.cylinder][head.head] : 12668;
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
FloppyDrive::findSyncMark()
{
    long length = disk->length.cylinder[head.cylinder][head.head];
    for (isize i = 0; i < length; i++) {
        
        if (readByteAndRotate() != 0x44) continue;
        if (readByteAndRotate() != 0x89) continue;
        break;
    }

    trace(DSK_DEBUG, "Moving to SYNC mark at offset %ld\n", head.offset);
}

bool
FloppyDrive::readyToStepUp() const
{
    // Check step delay
    if (agnus.clock - latestStep < getStepPulseDelay()) {

        debug(DSK_CHECKSUM, "Ignoring head step\n");
        return false;
    }

    // If the step direction reverses, some extra-time is needed (?)
    if (agnus.clock - latestStepDown < getRevStepPulseDelay()) {

        debug(DSK_CHECKSUM, "Ignoring reverse head step\n");
        return false;
    }

    return true;
}

bool
FloppyDrive::readyToStepDown() const
{
    // Check step delay
    if (agnus.clock - latestStep < getStepPulseDelay()) {

        debug(DSK_CHECKSUM, "Ignoring head step\n");
        return false;
    }

    // If the step direction reverses, some extra-time is needed (?)
    if (agnus.clock - latestStepUp < getRevStepPulseDelay()) {

        debug(DSK_CHECKSUM, "Ignoring reverse head step\n");
        return false;
    }

    return true;
}

void
FloppyDrive::step(isize dir)
{    
    // Update the disk change signal
    if (hasDisk()) dskchange = true;

    if (dir) {

        // Only proceed if the last head step was a while ago
        if (!readyToStepDown()) return;

        // Move drive head outwards (towards the lower tracks)
        if (head.cylinder > 0) {

            head.cylinder--;
            recordCylinder(head.cylinder);

            // Determine when the step will be completed
            latestStepCompleted = agnus.clock + getTrackToTrackDelay() + getHeadSettleTime();

            // Remember the step cycle
            latestStep = latestStepDown = agnus.clock;
        }

        debug(DSK_CHECKSUM, "Stepping down to cylinder %ld\n", head.cylinder);

    } else {

        // Only proceed if the last head step was a while ago
        if (!readyToStepUp()) return;

        // Move drive head inwards (towards the upper tracks)
        if (head.cylinder < 83) {

            head.cylinder++;
            recordCylinder(head.cylinder);

            // Determine when the step will be completed
            latestStepCompleted = agnus.clock + getTrackToTrackDelay() + getHeadSettleTime();

            // Remember the step cycle
            latestStep = latestStepUp = agnus.clock;
        }

        debug(DSK_CHECKSUM, "Stepping up to cylinder %ld\n", head.cylinder);
    }
    
    // Push drive head forward
    if (ALIGN_HEAD) head.offset = 0;
    
    // Notify the GUI
    if (pollsForDisk()) {
        
        msgQueue.put(MSG_DRIVE_POLL, DriveMsg {
            i16(nr), i16(head.cylinder), config.pollVolume, config.pan
        });
        
    } else {

        msgQueue.put(MSG_DRIVE_STEP, DriveMsg {
            i16(nr), i16(head.cylinder), config.stepVolume, config.pan
        });
    }
}

void
FloppyDrive::recordCylinder(Cylinder cylinder)
{
    cylinderHistory = (cylinderHistory & 0x00FF'FFFF'FFFF'FFFF) << 8 | cylinder;
}

bool
FloppyDrive::pollsForDisk() const
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
FloppyDrive::isInsertable(Diameter t, Density d) const
{
    debug(DSK_DEBUG,
          "isInsertable(%s, %s)\n", DiameterEnum::key(t), DensityEnum::key(d));
    
    switch (config.type) {
            
        case DRIVE_DD_35:
            return t == INCH_35 && d == DENSITY_DD;
            
        case DRIVE_HD_35:
            return t == INCH_35;
            
        case DRIVE_DD_525:
            return t == INCH_525 && d == DENSITY_DD;

        default:
            fatalError;
    }
}

bool
FloppyDrive::isInsertable(const FloppyFile &file) const
{
    return isInsertable(file.getDiameter(), file.getDensity());
}

bool
FloppyDrive::isInsertable(const FloppyDisk &disk) const
{
    return isInsertable(disk.diameter, disk.density);
}

template <EventSlot s> void
FloppyDrive::ejectDisk(Cycle delay)
{
    debug(DSK_DEBUG, "ejectDisk <%ld> (%lld)\n", s, delay);
    
    {   SUSPENDED

        // Schedule an ejection event
        agnus.scheduleRel <s> (delay, DCH_EJECT);

        // If there is no delay, service the event immediately
        if (delay == 0) serviceDiskChangeEvent <s> ();
    }
}

void
FloppyDrive::ejectDisk(Cycle delay)
{
    debug(DSK_DEBUG, "ejectDisk(%lld)\n", delay);
    
    if (nr == 0) ejectDisk <SLOT_DC0> (delay);
    if (nr == 1) ejectDisk <SLOT_DC1> (delay);
    if (nr == 2) ejectDisk <SLOT_DC2> (delay);
    if (nr == 3) ejectDisk <SLOT_DC3> (delay);
}

template <EventSlot s> void
FloppyDrive::insertDisk(std::unique_ptr<FloppyDisk> disk, Cycle delay)
{
    assert(disk != nullptr);
    
    debug(DSK_DEBUG, "insertDisk <%ld> (%lld)\n", s, delay);

    // Only proceed if the provided disk is compatible with this drive
    if (!isInsertable(*disk)) throw VAError(ERROR_DISK_INCOMPATIBLE);

    {   SUSPENDED
        
        // Get ownership of the disk
        diskToInsert = std::move(disk);

        // Schedule an insertion event
        agnus.scheduleRel <s> (delay, DCH_INSERT);

        // If there is no delay, service the event immediately
        if (delay == 0) serviceDiskChangeEvent <s> ();
    }
}

void
FloppyDrive::catchFile(const string &path)
{
    {   SUSPENDED
        
        // Extract the file system
        auto fs = MutableFileSystem(*this);
        
        // Seek file
        auto file = fs.seekFile(path);
        if (file == nullptr) throw VAError(ERROR_FILE_NOT_FOUND);
        
        // Extract file
        Buffer<u8> buffer;
        file->writeData(buffer);
        
        // Parse hunks
        auto descr = ProgramUnitDescriptor(buffer);
        
        // Seek the code section and read the first instruction word
        auto offset = descr.seek(HUNK_CODE);
        if (!offset) throw VAError(ERROR_HUNK_CORRUPTED);
        u16 instr = HI_LO(buffer[*offset + 8], buffer[*offset + 9]);
        
        // Replace the first instruction word by a software trap
        auto trap = cpu.debugger.swTraps.create(instr);
        buffer[*offset + 8] = HI_BYTE(trap);
        buffer[*offset + 9] = LO_BYTE(trap);
        
        // Write the modification back to the file system
        file->overwriteData(buffer);
        
        // Convert the modified file system back to a disk
        auto adf = ADFFile(fs);
        
        // Replace the old disk
        swapDisk(std::make_unique<FloppyDisk>(adf));
    }
}

void
FloppyDrive::insertDisk(std::unique_ptr<FloppyDisk> disk, Cycle delay)
{
    debug(DSK_DEBUG, "insertDisk(%lld)\n", delay);
    
    if (nr == 0) insertDisk <SLOT_DC0> (std::move(disk), delay);
    if (nr == 1) insertDisk <SLOT_DC1> (std::move(disk), delay);
    if (nr == 2) insertDisk <SLOT_DC2> (std::move(disk), delay);
    if (nr == 3) insertDisk <SLOT_DC3> (std::move(disk), delay);
}

void
FloppyDrive::insertNew(FSVolumeType fs, BootBlockId bb, string name)
{
    debug(DSK_DEBUG,
          "insertNew(%s,%s, %s)\n",
          FSVolumeTypeEnum::key(fs), BootBlockIdEnum::key(bb), name.c_str());
    
    ADFFile adf;
    
    // Create a suitable ADF for this drive
    switch (config.type) {
            
        case DRIVE_DD_35: adf.init(INCH_35, DENSITY_DD); break;
        case DRIVE_HD_35: adf.init(INCH_35, DENSITY_HD); break;
        case DRIVE_DD_525: adf.init(INCH_525, DENSITY_SD); break;
    }
    
    // Add a file system
    adf.formatDisk(fs, bb, name);
    
    // Replace the current disk with the new one
    swapDisk(adf);
}

void
FloppyDrive::swapDisk(std::unique_ptr<FloppyDisk> disk)
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
FloppyDrive::swapDisk(class FloppyFile &file)
{
    swapDisk(std::make_unique<FloppyDisk>(file));
}

void
FloppyDrive::swapDisk(const string &name)
{
    bool append = !util::isAbsolutePath(name) && searchPath != "";
    string path = append ? searchPath + "/" + name : name;
    
    std::unique_ptr<FloppyFile> file(FloppyFile::make(path));
    swapDisk(*file);
}

template <EventSlot s> void
FloppyDrive::serviceDiskChangeEvent()
{
    // Check if we need to eject the current disk
    if (agnus.id[s] == DCH_EJECT || agnus.id[s] == DCH_INSERT) {
        
        if (disk) {
            
            // Flag disk change in CIAA::PA
            dskchange = false;
            
            // Get rid of the disk
            disk = nullptr;
            
            // Notify the GUI
            msgQueue.put(MSG_DISK_EJECT,
                         DriveMsg { i16(nr), 0, config.ejectVolume, config.pan });
            /*
            msgQueue.put(MSG_DISK_EJECT,
                         i16(nr), 0, config.ejectVolume, config.pan);
             */
        }
    }
    
    // Check if we need to insert a new disk
    if (agnus.id[s] == DCH_INSERT) {
        
        if (diskToInsert) {
            
            // Insert the new disk
            disk = std::move(diskToInsert);
            
            // Remove indeterminism by repositioning the drive head
            head.offset = 0;
            
            // Notify the GUI
            msgQueue.put(MSG_DISK_INSERT, DriveMsg {
                i16(nr), 0, config.insertVolume, config.pan
            });
        }
    }

    // Remove the event
    agnus.cancel <s> ();
}

void
FloppyDrive::PRBdidChange(u8 oldValue, u8 newValue)
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

}
