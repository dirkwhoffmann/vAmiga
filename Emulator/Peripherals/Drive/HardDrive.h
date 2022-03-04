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
#include "Drive.h"
#include "FloppyDiskTypes.h"
#include "HDFFile.h"

class HardDrive : public Drive {
    
    friend class HDFFile;
    friend class HdController;
    
    // Current configuration
    HardDriveConfig config = {};
    
    // Result of the latest inspection
    mutable HardDriveInfo info = {};

    // Hard drive description
    HdrvDescriptor desc;
    
    // Partition table
    std::vector<PartitionDescriptor> ptable;
            
    // Disk data
    u8 *data = nullptr;
    
    // Current position of the read/write head
    struct { isize c; isize h; isize s; } head = { };
    
    // Disk state flags
    bool modified = false;
    bool writeProtected = false;

    
    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
    ~HardDrive() { dealloc(); }

    // Allocates or deallocates drive memory
    void alloc(isize size);
    void dealloc();
    
    // Starts from scratch
    void init();
    
    // Creates a hard drive with a certain geometry
    void init(const GeometryDescriptor &geometry);

    // Creates a hard drive with a certain capacity in bytes
    void init(isize size);

    // Creates a hard drive with the contents of a file system
    void init(const MutableFileSystem &fs) throws;

    // Creates a hard drive with the contents of an HDF
    void init(const HDFFile &hdf) throws;

    
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
        << config.connected
        >> desc
        >> ptable
        << modified
        << writeProtected;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << head.c
            << head.h
            << head.s;
        }
    }

    isize _size() override;
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    isize didSaveToBuffer(u8 *buffer) override;

    
    //
    // Methods from Drive
    //
    
public:
    
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

    // Returns the device number
    isize getNr() const { return nr; }

    // Returns information about the disk or one of its partitions
    HardDriveInfo getInfo() const { return AmigaComponent::getInfo(info); }
    const PartitionDescriptor &getPartitionInfo(isize nr);
    
    // Returns the disk geometry
    const GeometryDescriptor &getGeometry() const { return desc.geometry; }

    // Returns the number of partitions
    isize numPartitions() const { return isize(ptable.size()); }
    
    // Checks whether this drive is connected to the Amiga
    bool isConnected() const { return config.connected; }
    
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
    // void format(FSVolumeType fs) { format(fs, defaultName()); } throws;

    // Change the drive geometry
    void changeGeometry(isize c, isize h, isize s, isize b = 512) throws;
    void changeGeometry(const GeometryDescriptor &geometry) throws;
    
    
    //
    // Reading and writing
    //
    
public:
    
    // Reads a data block from the hard drive and copies it into RAM
    i8 read(isize partition, isize block, isize length, u32 addr);
    i8 read(isize offset, isize length, u32 addr);
    
    // Reads a data block from RAM and writes it onto the hard drive
    i8 write(isize partition, isize block, isize length, u32 addr);
    i8 write(isize offset, isize length, u32 addr);
    
private:
    
    // Converts a partition / block pair into an offset
    isize offset(isize partition, isize block);
    
    // Checks the given argument list for consistency
    i8 verify(isize offset, isize length, u32 addr);

    // Moves the drive head to the specified block
    void moveHead(isize lba);
    void moveHead(isize c, isize h, isize s);
};
