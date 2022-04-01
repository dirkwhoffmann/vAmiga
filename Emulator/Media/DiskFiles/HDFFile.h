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
    
    // Derived drive geometry
    GeometryDescriptor geometry;

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
    // Providing descriptors
    //
    
    struct GeometryDescriptor getGeometryDescriptor() const;
    struct PartitionDescriptor getPartitionDescriptor(isize part = 0) const;
    std::vector<PartitionDescriptor> getPartitionDescriptors() const;
    struct FileSystemDescriptor getFileSystemDescriptor(isize part = 0) const;

        
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
    
    // Returns the (predicted) geometry for this disk
    const GeometryDescriptor getGeometry() const { return geometry; }
    
    // Returns true if this image contains a rigid disk block
    bool hasRDB() const;
    
    // Returns the layout parameters of the hard drive
    isize numPartitions() const;

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

    // Return a pointer to the Root Block if it exists
    u8 *seekRB() const;

    // Return a pointer to the Rigid Disk Block if it exists
    u8 *seekRDB() const;
    
    // Returns a pointer to a certain partition block if it exists
    u8 *seekPB(isize nr) const;
    
    // Returns a string from the Rigid Disk Block if it exists
    std::optional<string> rdbString(isize offset, isize len) const;

    // Extracts the DOS revision number from a certain block
    FSVolumeType dos(isize nr) const;
    
    
    //
    // Serializing
    //
    
public:
    
    isize writePartitionToFile(const string &path, isize nr);
};
