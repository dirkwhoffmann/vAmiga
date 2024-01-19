// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HardDriveTypes.h"
#include "Drive.h"
#include "AgnusTypes.h"
#include "HdControllerTypes.h"
#include "HDFFile.h"
#include "MemUtils.h"

namespace vamiga {

class HardDrive : public Drive {
    
    friend class HDFFile;
    friend class HdController;

    // Write-through storage files
    static std::fstream wtStream[4];
    
    // Current configuration
    HardDriveConfig config = {};
    
    // Result of the latest inspection
    mutable HardDriveInfo info = {};

    // Product information
    string diskVendor;
    string diskProduct;
    string diskRevision;
    string controllerVendor;
    string controllerProduct;
    string controllerRevision;

    // Hard disk geometry
    GeometryDescriptor geometry;
    
    // Partition table
    std::vector <PartitionDescriptor> ptable;

    // Loadable file system drivers
    std::vector <DriverDescriptor> drivers;

    // Disk data
    Buffer<u8> data;
    
    // Current position of the read/write head
    DriveHead head;

    // Current drive state
    HardDriveState state = HDR_IDLE;
    
    // Disk state flags
    bool modified = false;
    bool writeProtected = false;
    optional <bool> bootable;

    // Indicates if write-through mode is enabled
    bool writeThrough = false;
    
    
    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
    ~HardDrive();
    
    // Creates a hard drive with a certain geometry
    void init(const GeometryDescriptor &geometry);

    // Creates a hard drive with a certain capacity in bytes
    void init(isize size);

    // Creates a hard drive with the contents of a file system
    void init(const MutableFileSystem &fs) throws;

    // Creates a hard drive with the contents of an HDF
    void init(const HDFFile &hdf) throws;

    // Creates a hard drive with the contents of an HDF file
    void init(const string &path) throws;

private:

    // Restors the initial state
    void init();

    
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
        << state;

        if (util::isResetter(worker)) return;

        worker

        << config.type
        << config.pan
        << config.stepVolume
        << diskVendor
        << diskProduct
        << diskRevision
        << controllerVendor
        << controllerProduct
        << controllerRevision
        << geometry
        << ptable
        << drivers
        << data
        << modified
        << writeProtected
        << bootable;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    
    //
    // Methods from Drive
    //
    
public:
    
    string getDiskVendor() const override { return diskVendor; }
    string getDiskProduct() const override { return diskProduct; }
    string getDiskRevision() const override { return diskRevision; }
    string getControllerVendor() const override { return controllerVendor; }
    string getControllerProduct() const override { return controllerProduct; }
    string getControllerRevision() const override { return controllerRevision; }

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
    
    const HardDriveConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    
private:
    
    void connect();
    void disconnect();

    
    //
    // Analyzing
    //

public:

    // Returns information about the disk or one of its partitions
    HardDriveInfo getInfo() const { return CoreComponent::getInfo(info); }
    const PartitionDescriptor &getPartitionInfo(isize nr);
    
    // Returns the disk geometry
    const GeometryDescriptor &getGeometry() const { return geometry; }

    // Returns the number of partitions
    isize numPartitions() const { return isize(ptable.size()); }

    // Returns the number of loadable file system drivers
    isize numDrivers() const { return isize(drivers.size()); }

    // Returns the current drive state
    HardDriveState getState() const { return state; }
    
    // Gets or sets the 'modification' flag
    bool isModified() const { return modified; }
    void setModified(bool value) { modified = value; }

    // Returns the current controller state
    HdcState getHdcState();

    // Checks whether the drive will work with the currently installed Rom
    bool isCompatible();
    
    
    //
    // Formatting
    //
    
    // Returns a default volume name
    string defaultName(isize partition = 0);

    // Formats the disk
    void format(FSVolumeType fs, string name) throws;

    // Change the drive geometry
    void changeGeometry(isize c, isize h, isize s, isize b = 512) throws;
    void changeGeometry(const GeometryDescriptor &geometry) throws;
    
    
    //
    // Reading and writing
    //
    
public:
    
    // Reads a data block from the hard drive and copies it into RAM
    i8 read(isize offset, isize length, u32 addr);
    
    // Reads a data block from RAM and writes it onto the hard drive
    i8 write(isize offset, isize length, u32 addr);
    
    // Reads a loadable file system
    void readDriver(isize nr, Buffer<u8> &driver);
    
private:

    // Checks the given argument list for consistency
    i8 verify(isize offset, isize length, u32 addr);

    // Moves the drive head to the specified block
    void moveHead(isize lba);
    void moveHead(isize c, isize h, isize s);
    
    
    //
    // Importing and exporting
    //
    
public:
    
    // Restores a disk (called on connect)
    bool restoreDisk() throws;

    // Exports the disk in HDF format
    void writeToFile(const string &path) throws;

    
    //
    // Managing write-through mode
    //
    
    bool writeThroughEnabled() const { return writeThrough; }
    void enableWriteThrough() throws;
    void disableWriteThrough();

private:
    
    // Return the path to the write-through storage file
    string writeThroughPath();
    
    // Creates or updates the write-through storage file
    void saveWriteThroughImage() throws;
    
    
    //
    // Scheduling and serving events
    //
    
public:
    
    // Schedules an event to revert to idle state
    void scheduleIdleEvent();
    
    // Services a hard drive event
    template <EventSlot s> void serviceHdrEvent();
    
};

}
