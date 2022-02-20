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
#include "DiskTypes.h"
#include "HDFFile.h"
#include "SubComponent.h"

class HardDrive : public SubComponent {
    
    // Number of the emulated drive (dh<nr>)
    const isize nr;

    // Result of the latest inspection
    mutable HardDriveInfo info = {};
    
    // Disk geometry
    DiskGeometry geometry;
    
    // Disk data
    u8 *data = nullptr;
    
    // Current position of the read/write head
    struct { isize c; isize h; isize s; } head;
    
    // Indicates if a write operation has been performed
    bool modified = false;
    
    
    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
    ~HardDrive();
    
    // Creates or deletes the data storage
    void alloc(const DiskGeometry &geometry);
    void dealloc();
    
    
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
        
        << geometry.cylinders
        << geometry.heads
        << geometry.sectors
        << geometry.bsize
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
    // Analyzing
    //

public:

    // Returns the device number
    isize getNr() const { return nr; }

    // Returns information about the current state
    HardDriveInfo getInfo() const { return AmigaComponent::getInfo(info); }

    // Checks whether this drive is attached to the Amiga
    const DiskGeometry &getGeometry() const { return geometry; }

    // Checks whether this drive is attached to the Amiga
    bool isAttached() const { return geometry.cylinders != 0; }
    
    // Gets or sets the 'modification' flag
    bool isModified() const { return modified; }
    void setModified(bool value) { modified = value; }
        
    
    //
    // Attaching
    //

public:
    
    // Creates an empty hard drive
    void attach(const DiskGeometry &geometry) throws;

    // Creates a disk with the contents of a file system
    void attach(const FSDevice &fs) throws;

    // Creates a disk with the contents of an HDF
    void attach(const HDFFile &hdf) throws;

    
    //
    // Formatting
    //
    
    // Formats the disk
    void format(FSVolumeType fs, BootBlockId bb) throws;

    
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
};
