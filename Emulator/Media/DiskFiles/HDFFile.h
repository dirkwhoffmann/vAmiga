// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskFile.h"
#include "MediaFileTypes.h"
#include "MutableFileSystem.h"
#include "DriveTypes.h"

namespace vamiga {

class FloppyDisk;

class HDFFile : public DiskFile {
    
public:
    
    // Derived drive geometry
    GeometryDescriptor geometry;

    // Derived partition table
    std::vector <PartitionDescriptor> ptable;
    
    // Included device drivers
    std::vector <DriverDescriptor> drivers;

    static bool isCompatible(const std::filesystem::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
    static bool isOversized(isize size) { return size > MB(504); }

    bool isCompatiblePath(const std::filesystem::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }

    void finalizeRead() override;
    
    
    //
    // Initializing
    //

public:
    
    HDFFile(const std::filesystem::path &path) throws { init(path); }
    HDFFile(const u8 *buf, isize len) throws { init(buf, len); }
    HDFFile(const class HardDrive &hdn) throws { init(hdn); }

    void init(const std::filesystem::path &path) throws;
    void init(const u8 *buf, isize len) throws;
    void init(const class HardDrive &hdn) throws;

    const char *objectName() const override { return "HDF"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType type() const override { return FILETYPE_HDF; }
    

    //
    // Methods from DiskFile
    //

    isize numCyls() const override;
    isize numHeads() const override;
    isize numSectors() const override;
    
    
    //
    // Providing descriptors
    //
    
    GeometryDescriptor getGeometryDescriptor() const;
    PartitionDescriptor getPartitionDescriptor(isize part = 0) const;
    std::vector<PartitionDescriptor> getPartitionDescriptors() const;
    DriverDescriptor getDriverDescriptor(isize driver = 0) const;
    std::vector<DriverDescriptor> getDriverDescriptors() const;
    FileSystemDescriptor getFileSystemDescriptor(isize part = 0) const;


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
    isize numPartitions() const { return isize(ptable.size()); }

    // Returns the byte count and the location of a certain partition
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
    FSVolumeType dos(isize nr) const;

    
    //
    // Serializing
    //
    
public:
    
    isize writePartitionToFile(const std::filesystem::path &path, isize nr) override;
};

}
