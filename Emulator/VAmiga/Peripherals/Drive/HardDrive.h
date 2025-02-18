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
#include "Buffer.h"
#include "MemUtils.h"

namespace vamiga {

class HardDrive final : public Drive, public Inspectable<HardDriveInfo> {
    
    Descriptions descriptions = {
        {
            .type           = Class::HardDrive,
            .name           = "HardDrive0",
            .description    = "Hard Drive 0",
            .shell          = "hd0",
            .help           = { "Hard Drive n", "hd[n]" }
        },
        {
            .type           = Class::HardDrive,
            .name           = "HardDrive1",
            .description    = "Hard Drive 1",
            .shell          = "hd1",
            .help           = { "" }
        },
        {
            .type           = Class::HardDrive,
            .name           = "HardDrive2",
            .description    = "Hard Drive 2",
            .shell          = "hd2",
            .help           = { "" }
        },
        {
            .type           = Class::HardDrive,
            .name           = "HardDrive3",
            .description    = "Hard Drive 3",
            .shell          = "hd3",
            .help           = { "" }
        }
    };

    ConfigOptions options = {

        Opt::HDR_TYPE, 
        Opt::HDR_PAN,
        Opt::HDR_STEP_VOLUME
    };
    
    friend class HDFFile;
    friend class HdController;

    // Write-through storage files
    static std::fstream wtStream[4];
    
    // Current configuration
    HardDriveConfig config = {};
    
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
    util::Buffer<u8> data;
    
    // Keeps track of modified blocks (to update the run-ahead instance)
    util::Buffer<bool> dirty;

    // Current position of the read/write head
    DriveHead head;

    // Current drive state
    HardDriveState state = HardDriveState::IDLE;
    
    // Disk state flags
    long flags = 0;

    
    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
    ~HardDrive();
    
    HardDrive& operator= (const HardDrive& other);

    // Creates a hard drive with a certain geometry
    void init(const GeometryDescriptor &geometry);

    // Creates a hard drive with a certain capacity in bytes
    void init(isize size);

    // Creates a hard drive with the contents of a file system
    void init(const class MutableFileSystem &fs) throws;

    // Creates a hard drive with the contents of a media file
    void init(const class MediaFile &file) throws;

    // Creates a hard drive with the contents of an HDF or HDZ
    void init(const class HDFFile &hdf) throws;
    void init(const class HDZFile &hdz) throws;

    // Creates a hard drive with the contents of an HDF file
    void init(const fs::path &path) throws;

    const HardDriveTraits &getTraits() const {

        static HardDriveTraits traits;

        traits.nr = objid;
        
        traits.diskVendor = diskVendor.c_str();
        traits.diskProduct = diskProduct.c_str();
        traits.diskRevision = diskRevision.c_str();
        traits.controllerVendor = controllerVendor.c_str();
        traits.controllerProduct = controllerProduct.c_str();
        traits.controllerRevision = controllerRevision.c_str();

        traits.cylinders = geometry.cylinders;
        traits.heads = geometry.heads;
        traits.sectors = geometry.sectors;
        traits.bsize = geometry.bsize;

        traits.tracks = geometry.numTracks();
        traits.blocks = geometry.numBlocks();
        traits.bytes = geometry.numBytes();
        traits.upperCyl = geometry.upperCyl();
        traits.upperHead = geometry.upperHead();
        traits.upperTrack = geometry.upperTrack();

        return traits;
    }

    const PartitionTraits &getPartitionTraits(isize nr) const {

        static PartitionTraits traits;

        auto descr = getPartitionDescriptor(nr);
        traits.nr = nr;
        traits.name = descr.name.c_str();
        traits.lowerCyl = descr.lowCyl;
        traits.upperCyl = descr.highCyl;

        return traits;
    }

private:

    // Restors the initial state
    void init();

    
    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _initialize() override;
    
    template <class T>
    void serialize(T& worker)
    {
        if (isSoftResetter(worker)) return;

        worker

        << head.cylinder
        << head.head
        << head.offset
        << state;

        if (isResetter(worker)) return;

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
        << flags;

    } SERIALIZERS(serialize, override);

    void _didReset(bool hard) override;
    void _didLoad() override;

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

    
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

    bool getFlag(DiskFlags mask) const override;
    void setFlag(DiskFlags mask, bool value) override;

    bool hasDisk() const override;
    bool hasModifiedDisk() const override;
    bool hasProtectedDisk() const override;
    void setModificationFlag(bool value) override;
    void setProtectionFlag(bool value) override;
    
    
    //
    // Methods from Configurable
    //

public:
    
    const HardDriveConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;
    
private:
    
    void connect();
    void disconnect();

    
    //
    // Analyzing
    //

public:

    // Returns information about the disk
    void cacheInfo(HardDriveInfo &info) const override;

    // Returns information about a specific partition
    const PartitionDescriptor &getPartitionDescriptor(isize nr) const;

    // Returns the disk geometry
    const GeometryDescriptor &getGeometry() const { return geometry; }

    // Returns the number of partitions
    isize numPartitions() const { return isize(ptable.size()); }

    // Returns the number of loadable file system drivers
    isize numDrivers() const { return isize(drivers.size()); }

    // Returns the current drive state
    HardDriveState getState() const { return state; }
    
    // Gets or sets the 'modification' flag
    bool isModified() const { return flags & long(DiskFlags::MODIFIED); }
    void setModified(bool value) { value ? flags |= long(DiskFlags::MODIFIED) : flags &= ~long(DiskFlags::MODIFIED); }

    // Returns the current controller state
    HdcState getHdcState() const;

    // Checks whether the drive will work with the currently installed Rom
    bool isCompatible() const;
    
    // Checks whether the drive has a user directory block
    bool hasUserDir() const;
    
    
    //
    // Formatting
    //
    
    // Returns a default volume name
    string defaultName(isize partition = 0) const;

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
    void readDriver(isize nr, util::Buffer<u8> &driver);
    
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
    void writeToFile(const fs::path &path) throws;

    
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
