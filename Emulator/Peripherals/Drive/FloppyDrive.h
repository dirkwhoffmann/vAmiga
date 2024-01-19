// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FloppyDriveTypes.h"
#include "AgnusTypes.h"
#include "BootBlockImageTypes.h"
#include "FSTypes.h"
#include "Drive.h"
#include "FloppyDisk.h"
#include "DiskController.h"
#include "Thread.h"

namespace vamiga {

class FloppyDrive : public Drive {
    
    friend class DiskController;

    // Current configuration
    FloppyDriveConfig config = {};

    // Result of the latest inspection
    mutable FloppyDriveInfo info = {};

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

    // Time stamps of the most recent head step
    Cycle latestStepUp;
    Cycle latestStepDown;
    Cycle latestStep;
    Cycle latestStepCompleted;

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
    std::unique_ptr<FloppyDisk> disk;

private:

    // A disk waiting to be inserted (if any)
    std::unique_ptr<FloppyDisk> diskToInsert;
    
    // Search path for disk files, one for each drive
    string searchPath;
    
    
    //
    // Initializing
    //

public:

    FloppyDrive(Amiga& ref, isize nr);
    
    
    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override;
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _initialize() override;
    void _reset(bool hard) override;
    void _inspect() const override;
    
    template <class T>
    void serialize(T& worker)
    {
        if (util::isSoftResetter(worker)) return;

        worker

        << head.cylinder
        << head.head
        << head.offset
        << motor
        << switchCycle
        << switchSpeed
        << idCount
        << idBit
        << latestStepUp
        << latestStepDown
        << latestStep
        << dskchange
        << dsklen
        << prb
        << cylinderHistory;

        if (util::isResetter(worker)) return;

        worker

        << config.type
        << config.mechanics
        << config.rpm;
    }

    isize _size() override;
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override;
    isize _save(u8 *buffer) override;

    
    //
    // Methods from Drive
    //
    
public:
    
    bool isConnected() const override;
    
    Cylinder currentCyl() const override { return head.cylinder; }
    Head currentHead() const override { return head.head; }
    isize currentOffset() const override { return head.offset; }

    bool hasDisk() const override;
    bool hasModifiedDisk() const override;
    bool hasProtectedDisk() const override;
    void setModificationFlag(bool value) override;
    void setProtectionFlag(bool value) override;

    
    //
    // Configuring
    //
    
public:
    
    const FloppyDriveConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    
    const string &getSearchPath() const { return searchPath; }
    void setSearchPath(const string &path) { searchPath = path; }


    //
    // Analyzing
    //

public:
    
    // Returns the result of the latest inspection
    FloppyDriveInfo getInfo() const { return CoreComponent::getInfo(info); }

    // Return the identification pattern of this drive
    u32 getDriveId() const;

    // Checks whether the drive is in identification mode
    bool idMode() const;

    // Checks whether a write operation is in progress
    bool isWriting() const;


    //
    // Querying mechanical delays
    //

    // Returns the ramp-up time of the drive motor
    Cycle getStartDelay() const;

    // Returns the ramp-down time of the drive motor
    Cycle getStopDelay() const;

    // Returns the minimum delay required between two step pulses
    Cycle getStepPulseDelay() const;
    Cycle getRevStepPulseDelay() const;

    // Returns the duration of a head step
    Cycle getTrackToTrackDelay() const;

    // Returns how long garbabe is read after a head step
    Cycle getHeadSettleTime() const;


    //
    // Handling the drive status register flags
    //
    
    // Returns true if the drive is currently selected
    bool isSelected() const { return (prb & (0b1000 << nr)) == 0; }
    
    u8 driveStatusFlags() const;
    

    //
    // Operating the drive motor
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

    
    //
    // Accessing data
    //

    // Selects the active drive head (0 = lower, 1 = upper)
    void selectSide(Head h);

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
    bool readyToStepUp() const;
    bool readyToStepDown() const;

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

    bool isInsertable(Diameter t, Density d) const;
    bool isInsertable(const FloppyFile &file) const;
    bool isInsertable(const FloppyDisk &disk) const;

    // Ejects the current disk with an optional delay
    void ejectDisk(Cycle delay = 0);
    
    // Inserts a new disk with an optional delay
    void insertDisk(std::unique_ptr<FloppyDisk> disk, Cycle delay = 0) throws;
    
    // Replaces the current disk (recommended way to insert disks)
    void swapDisk(std::unique_ptr<FloppyDisk> disk) throws;
    void swapDisk(class FloppyFile &file) throws;
    void swapDisk(const string &name) throws;

    // Replaces the current disk with a factory-fresh disk
    void insertNew(FSVolumeType fs, BootBlockId bb, string name) throws;

private:
    
    template <EventSlot s> void ejectDisk(Cycle delay);
    template <EventSlot s> void insertDisk(std::unique_ptr<FloppyDisk> disk, Cycle delay) throws;

    
    //
    // Handling files
    //

public:
    
    // Sets a catchpoint on the specified file
    void catchFile(const string &path) throws;
    
    
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

}
