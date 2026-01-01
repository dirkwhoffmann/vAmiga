// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HDFFileTypes.h"
#include "HardDiskImage.h"
#include "FSDescriptor.h"
#include "utl/common.h"

namespace vamiga {

class FloppyDisk;

class HDFFile : public HardDiskImage {

public:
    
    // Derived drive geometry
    GeometryDescriptor geometry;

    // Derived partition table
    std::vector <PartitionDescriptor> ptable;
    
    // Included device drivers
    std::vector <DriverDescriptor> drivers;

    static optional<ImageInfo> isCompatible(const fs::path &path);

    static bool isOversized(isize size) { return size > 504_MB; }

    bool isCompatiblePath(const fs::path &path) const override {
        return isCompatible(path).has_value();
    }
    void didLoad() override;
    
    
    //
    // Initializing
    //

public:
    
    HDFFile() { }
    HDFFile(const fs::path &path) { init(path); }
    HDFFile(const u8 *buf, isize len) { init(buf, len); }

    void init(const fs::path &path);
    void init(const u8 *buf, isize len);

//    optional<ImageInfo> validateURL(const fs::path& url) override;


    //
    // Methods from AnyImage
    //

public:

    ImageType type() const noexcept override { return ImageType::HARDDISK; }
    ImageFormat format() const noexcept override { return ImageFormat::HDF; }


    //
    // Methods from TrackDevice
    //

public:

    isize numCyls() const override;
    isize numHeads() const override;
    isize numSectors(isize) const override { return numSectors(); }
    isize numSectors() const;


    //
    // Methods from HardDiskImage
    //

public:

    isize numPartitions() const override { return isize(ptable.size()); }
    Range<isize> partition(isize nr) const override { return ptable[nr].range(); }


    //
    // Methods from BlockDevice
    //

public:

    isize bsize() const override { return 512; }


    //
    // Providing descriptors
    //

public:
    
    GeometryDescriptor getGeometryDescriptor() const;
    PartitionDescriptor getPartitionDescriptor(isize part = 0) const;
    std::vector<PartitionDescriptor> getPartitionDescriptors() const;
    DriverDescriptor getDriverDescriptor(isize driver = 0) const;
    std::vector<DriverDescriptor> getDriverDescriptors() const;
    FSDescriptor getFileSystemDescriptor(isize part = 0) const;


    //
    // Querying product information
    //

public:
    
    std::optional<string> getDiskVendor() const { return rdbString(160, 8); }
    std::optional<string> getDiskProduct() const { return rdbString(168, 16); }
    std::optional<string> getDiskRevision() const { return rdbString(184, 4); }
    std::optional<string> getControllerVendor() const { return rdbString(188, 8); }
    std::optional<string> getControllerProduct() const { return rdbString(196, 16); }
    std::optional<string> getControllerRevision() const { return rdbString(212, 4); }


    //
    // Querying volume information
    //

public:
    
    // Returns information about the hard drive in this file
    HDFInfo getInfo() const;

    // Returns the (predicted) geometry for this disk
    const GeometryDescriptor getGeometry() const { return geometry; }

    // Returns true if this image contains a rigid disk block
    bool hasRDB() const;
    
    // Returns the number of loadable file system drivers
    isize numDrivers() const { return isize(drivers.size()); }
    
    // Returns the number of partitions
    // isize numPartitions() const { return isize(ptable.size()); }

    // Returns the byte count and the location of a certain partition
    // Range<isize> range(isize nr) const { return ptable[nr].range(); }
    isize partitionSize(isize nr) const;
    isize partitionOffset(isize nr) const;
    u8 *partitionData(isize nr) const;
    
    // Predicts the number of blocks of this hard drive
    isize predictNumBlocks() const;

    
    //
    // Scanning raw disk data
    //

private:

    // Returns a pointer to a certain block if it exists
    u8 *seekBlock(isize nr) const;

    // Checks whether the provided pointer points to a Root Block
    bool isRB(u8 *ptr) const;
    
    // Return a pointer to the Root Block if it exists
    u8 *seekRB() const;

    // Return a pointer to the Rigid Disk Block if it exists
    u8 *seekRDB() const;
    
    // Returns a pointer to a certain partition block if it exists
    u8 *seekPB(isize nr) const;

    // Returns a pointer to a certain filesystem header block if it exists
    u8 *seekFSH(isize nr) const;

    // Returns a string from the Rigid Disk Block if it exists
    std::optional<string> rdbString(isize offset, isize len) const;

    // Extracts the DOS revision number from a certain block
    FSFormat dos(isize nr) const;

    
    //
    // Serializing
    //
    
public:
    
    isize writePartitionToFile(const fs::path &path, isize nr) const;
};

}
