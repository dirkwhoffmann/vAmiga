// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DriveTypes.h"
#include "SubComponent.h"
#include "Disk.h"
#include "DiskController.h"
#include "SchedulerTypes.h"
#include "SuspendableThread.h"

class Drive : public SubComponent {
    
    friend class DiskController;
        
    // Number of the emulated drive (0 = df0, 1 = df1, etc.)
    const isize nr;

    // Current configuration
    DriveConfig config = {};

    // Result of the latest inspection
    mutable DriveInfo info = {};

    // The current head location
    DriveHead head;

    // Drive motor status (on or off)
    bool motor;
    
    // Time stamp indicating the latest change of the motor status
    Cycle switchCycle;
    
    // Recorded motor speed at 'switchCycle' in percent
    double switchSpeed;
    
    // Position of the currently transmitted identification bit
    u8 idCount;

    // Value of the currently transmitted identification bit
    bool idBit;

    // Time stamp indicating when the head started to step to another cylinder
    Cycle stepCycle;
    
    /* Disk change status. This variable controls the /CHNG bit in the CIA A
     * PRA register. Note that the variable only changes it's value under
     * certain conditions. If a head movement pulse is send and no disk is
     * inserted, the variable is set to false (which is also the reset value).
     * It becomes true when a disk is ejected.
     */
    bool dskchange;
    
    // A copy of the DSKLEN register
    u8 dsklen;
    
    // A copy of the PRB register of CIA B
    u8 prb;
        
    /* History buffer storing the most recently visited tracks. The buffer is
     * used to detect the polling head movements that are issued by track disc
     * device to detect a newly inserted disk.
     */
    u64 cylinderHistory;

public:
    
    // The currently inserted disk (if any)
    std::unique_ptr<Disk> disk;

private:

    // A disk waiting to be inserted (if any)
    std::unique_ptr<Disk> diskToInsert;
    
    // Search path for disk files, one for each drive
    string searchPath;
    
    
    //
    // Initializing
    //

public:

    Drive(Amiga& ref, isize nr);
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override;
    void _dump(dump::Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;
    void _inspect() const override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.type
        << config.startDelay
        << config.stopDelay
        << config.stepDelay;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << head.side
            << head.cylinder
            << head.offset
            << motor
            << switchCycle
            << switchSpeed
            << idCount
            << idBit
            << stepCycle
            << dskchange
            << dsklen
            << prb
            << cylinderHistory;
        }
    }

    isize _size() override;
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override;
    isize _save(u8 *buffer) override;

    
    //
    // Configuring
    //
    
public:
    
    static DriveConfig getDefaultConfig(isize nr);
    const DriveConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    
    const string &getSearchPath() const { return searchPath; }
    void setSearchPath(const string &path) { searchPath = path; }


    //
    // Analyzing
    //

public:
    
    isize getNr() { return nr; }
    DriveInfo getInfo() const { return AmigaComponent::getInfo(info); }
 

    //
    // Accessing
    //

public:

    // Returns true iff the drive is in identification mode
    bool idMode() const;
    
    // Return the identification pattern of this drive
    u32 getDriveId() const;

    // Operation
    isize getCylinder() const { return head.cylinder; }
    
    
    //
    // Handling the drive status register flags
    //
    
    // Returns true if the drive is currently selected
    bool isSelected() const { return (prb & (0b1000 << nr)) == 0; }
    
    u8 driveStatusFlags() const;
    

    //
    // Operating the drive
    //
        
public:
    
    // Returns the current motor speed in percent
    double motorSpeed() const;

    // Turns the drive motor on or off
    bool getMotor() const { return motor; }
    void setMotor(bool value);
    void switchMotorOn() { setMotor(true); }
    void switchMotorOff() { setMotor(false); }

    // Informs about the current drive motor state
    bool motorSpeedingUp() const;
    bool motorAtFullSpeed() const;
    bool motorSlowingDown() const;
    bool motorStopped() const;

    // Selects the active drive head (0 = lower, 1 = upper)
    void selectSide(Side side);

    // Reads a value from the drive head and optionally rotates the disk
    u8 readByte() const;
    u8 readByteAndRotate();
    u16 readWordAndRotate();

    // Writes a value to the drive head and optionally rotates the disk
    void writeByte(u8 value);
    void writeByteAndRotate(u8 value);
    void writeWordAndRotate(u16 value);

    // Emulate a disk rotation (moves head to the next byte)
    void rotate();

    // Rotates the disk to the next sync mark
    void findSyncMark();

    
    //
    // Moving the drive head
    //

public:
    
    // Returns wheather the drive is ready to accept a stepping pulse
    bool readyToStep() const;
    
    // Moves the drive head (0 = inwards, 1 = outwards).
    void step(isize dir);

    // Records a cylinder change (needed for diskPollingMode() to work)
    void recordCylinder(Cylinder cylinder);

    // Returns true if the drive is in disk polling mode
    bool pollsForDisk() const;

    
    //
    // Handling disks
    //

public:
    
    bool hasDisk() const { return disk != nullptr; }
    bool hasDDDisk() const { return disk ? disk->density == DISK_DD : false; }
    bool hasHDDisk() const { return disk ? disk->density == DISK_HD : false; }
    bool hasModifiedDisk() const { return disk ? disk->isModified() : false; }
    void setModifiedDisk(bool value) { if (disk) disk->setModified(value); }
    
    bool hasWriteEnabledDisk() const;
    bool hasWriteProtectedDisk() const;
    void setWriteProtection(bool value); 
    void toggleWriteProtection();
    
    u64 fnv() const;

    bool isInsertable(DiskDiameter t, DiskDensity d) const;
    bool isInsertable(const DiskFile &file) const;
    bool isInsertable(const Disk &disk) const;

    // Ejects the current disk with an optional delay
    void ejectDisk(Cycle delay = 0);
    
    // Inserts a new disk with an optional delay
    void insertDisk(std::unique_ptr<Disk> disk, Cycle delay = 0) throws;
    
    // Replaces the current disk (recommended way to insert disks)
    void swapDisk(std::unique_ptr<Disk> disk) throws;
    void swapDisk(class DiskFile &file) throws;
    void swapDisk(const string &name) throws;

    // Replaces the current disk with a factory-fresh disk
    void insertNew() throws;

    
private:
    
    template <EventSlot s> void ejectDisk(Cycle delay);
    template <EventSlot s> void insertDisk(std::unique_ptr<Disk> disk, Cycle delay) throws;

    
    //
    // Serving events
    //
    
public:
    
    // Services an event in the disk change slot
    template <EventSlot s> void serviceDiskChangeEvent();
    
    
    //
    // Delegation methods
    //
    
public:
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(u8 oldValue, u8 newValue);
};
