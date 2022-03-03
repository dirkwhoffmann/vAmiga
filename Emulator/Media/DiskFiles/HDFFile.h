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
#include "MutableFileSystem.h"
#include "DriveTypes.h"

class FloppyDisk;

class HDFFile : public DiskFile {
    
    // Derived hard drive descriptor
    HdrvDescriptor hdrv;

    // Derived partition table
    std::vector<PartitionDescriptor> ptable;
                
public:
    
    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);
    static bool isOversized(isize size) { return size > MB(504); }

    bool isCompatiblePath(const string &path) const override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) const override { return isCompatible(stream); }

    void finalizeRead() override;
    
    
    //
    // Initializing
    //

public:
    
    HDFFile(const string &path) throws { init(path); }
    HDFFile(const u8 *buf, isize len) throws { init(buf, len); }
    HDFFile(const class HardDrive &hdn) throws { init(hdn); }

    void init(const string &path) throws;
    void init(const u8 *buf, isize len) throws;
    void init(const class HardDrive &hdn) throws;

    const char *getDescription() const override { return "HDF"; }

    
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
    // Providing suitable descriptors
    //
    
    struct GeometryDescriptor getGeometryDescriptor() const;
    struct HdrvDescriptor getHdrvDescriptor() const;
    struct PartitionDescriptor getPartitionDescriptor(isize part = 0) const;
    std::vector<PartitionDescriptor> getPartitionDescriptors() const;
    struct FileSystemDescriptor getFileSystemDescriptor(isize part = 0) const;

        
    //
    // Querying volume information
    //

public:
    
    // Returns the (predicted) geometry for this disk
    const GeometryDescriptor getGeometry() const;
    
    // Returns true if this image contains a rigid disk block
    bool hasRDB() const;
    
    // Returns the layout parameters of the hard drive
    isize numPartitions() const;
    isize numReserved() const;

    // Returns a file system descriptor for this volume
    u8 *dataForPartition(isize nr) const;
    
    // Computes all possible drive geometries
    std::vector<GeometryDescriptor> driveGeometries(isize fileSize);
    
    
    //
    // Scanning raw disk data
    //

private:
    
    // Returns a pointer to a certain block if it exists
    u8 *seekBlock(isize nr) const;
    
    // Return a pointer to the Rigid Disk Block if it exists
    u8 *seekRDB() const;
    
    // Returns a pointer to a certain partition block if it exists
    u8 *seekPB(isize nr) const;
    
    
    //
    // Querying partition information
    //

private:
    
    // Extracts the DOS revision number from a certain block
    FSVolumeType dos(isize blockNr) const;
};
