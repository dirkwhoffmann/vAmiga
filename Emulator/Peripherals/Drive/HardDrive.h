// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HardDriveTypes.h"
#include "Drive.h"
#include "AgnusTypes.h"
#include "HDFFile.h"
#include "MemUtils.h"

class HardDrive : public Drive {
    
    friend class HDFFile;
    friend class HdController;
    
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
    std::vector<PartitionDescriptor> ptable;
            
    // Disk data
    Buffer<u8> data;
    
    // Current position of the read/write head
    DriveHead head;

    // Current drive state
    HardDriveState state = HDR_IDLE;
    
    // Disk state flags
    bool modified = false;
    bool writeProtected = false;

    
    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
        
    // Creates a hard drive with a certain geometry
    void init(const GeometryDescriptor &geometry);

    // Creates a hard drive with a certain capacity in bytes
    void init(isize size);

    // Creates a hard drive with the contents of a file system
    void init(const MutableFileSystem &fs) throws;

    // Creates a hard drive with the contents of an HDF
    void init(const HDFFile &hdf) throws;

private:

    // Restors the initial state
    void init();

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override;
    void _dump(Category category, std::ostream& os) const override;
    
    
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
        << config.connected
        << config.pan
        << config.stepVolume
        << diskVendor
        << diskProduct
        << diskRevision
        << controllerVendor
        << controllerProduct
        << controllerRevision
        >> geometry
        >> ptable
        << data
        << modified
        << writeProtected;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << head.cylinder
            << head.head
            << head.offset
            << state;
        }
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
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

    u64 fnv() const override;
    bool hasDisk() const override;
    bool hasModifiedDisk() const override;
    bool hasProtectedDisk() const override;
    void setModificationFlag(bool value) override;
    void setProtectionFlag(bool value) override;
    
    
    //
    // Configuring
    //
    
public:
    
    static HardDriveConfig getDefaultConfig(isize nr);
    const HardDriveConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);

    
    //
    // Analyzing
    //

public:

    // Returns information about the disk or one of its partitions
    HardDriveInfo getInfo() const { return AmigaComponent::getInfo(info); }
    const PartitionDescriptor &getPartitionInfo(isize nr);
    
    // Returns the disk geometry
    const GeometryDescriptor &getGeometry() const { return geometry; }

    // Returns the number of partitions
    isize numPartitions() const { return isize(ptable.size()); }
        
    // Returns the current drive state
    HardDriveState getState() const { return state; }
    
    // Gets or sets the 'modification' flag
    bool isModified() const { return modified; }
    void setModified(bool value) { modified = value; }
        

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
    
private:
        
    // Checks the given argument list for consistency
    i8 verify(isize offset, isize length, u32 addr);

    // Moves the drive head to the specified block
    void moveHead(isize lba);
    void moveHead(isize c, isize h, isize s);
    
    
    //
    // Scheduling and serving events
    //
    
public:
    
    // Schedules an event to revert to idle state
    void scheduleIdleEvent();
    
    // Services a hard drive event
    template <EventSlot s> void serviceHdrEvent();
    
};
