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
#include "DiskTypes.h"
#include "Drive.h"
#include "HDFFile.h"

class HardDrive : public Drive {
    
    friend class HDFFile;
    friend class HdrController;
    
    // Current configuration
    HardDriveConfig config = {};
    
    // Result of the latest inspection
    mutable HardDriveInfo info = {};

    // Hard drive spec (DEPRECATED)
    HardDriveSpec driveSpec = {};
        
    // Disk data
    u8 *data = nullptr;
    
    // Current position of the read/write head
    struct { isize c; isize h; isize s; } head = { };
    
    // Indicates if a write operation has been performed
    bool modified = false;
    
    
    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
    ~HardDrive();

    void dealloc();

    // Creates a hard drive with a certain geometry
    void init(const Geometry &geometry);

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
        
        << driveSpec.geometry.cylinders
        << driveSpec.geometry.heads
        << driveSpec.geometry.sectors
        << driveSpec.geometry.bsize
        << modified;
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
    isize didSaveToBuffer(u8 *buffer) const override;

    
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
    const PartitionSpec &getPartitionInfo(isize nr);
    
    // Returns the disk geometry
    const Geometry &getGeometry() const { return driveSpec.geometry; }

    // Returns the number of partitions
    isize numPartitions() { return isize(driveSpec.partitions.size()); }
    
    // Checks whether this drive is connected to the Amiga
    bool isConnected() const { return config.connected; }
    
    // Gets or sets the 'modification' flag
    bool isModified() const { return modified; }
    void setModified(bool value) { modified = value; }
        

    //
    // Formatting
    //
    
    // Formats the disk
    void format(FSVolumeType fs, BootBlockId bb) throws;

    // Change the drive geometry
    void changeGeometry(isize c, isize h, isize s, isize b = 512) throws;
    void changeGeometry(const Geometry &geometry) throws;
    
    
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
