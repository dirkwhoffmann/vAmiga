// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"
#include "FSDevice.h"
#include "HardDriveTypes.h"

class Disk;

class HDFFile : public AmigaFile {
    
    // Collected device information
    HardDriveSpec driveSpec;
    
    // Geometry of this hard drive
    // DiskGeometry geometry;
        
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
    HDFFile(const class HardDrive &drive) throws { init(drive); }

    void init(const string &path) throws;
    void init(const u8 *buf, isize len) throws;
    void init(const class HardDrive &drive) throws;

    const char *getDescription() const override { return "HDF"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType type() const override { return FILETYPE_HDF; }
    
    //
    // Querying volume information
    //

public:
    
    // Returns the (predicted) geometry for this disk
    const DiskGeometry getGeometry() const;
    const HardDriveSpec getDriveSpec() const { return driveSpec; }
    
    // Returns true if this image contains a rigid disk block
    bool hasRDB() const; // DEPRECATED
    
    // Returns the layout parameters of the hard drive
    isize numPartitions() const;
    isize numCyls() const;
    isize numSides() const;
    isize numSectors() const;
    isize numReserved() const;
    isize numBlocks() const;
    isize bsize() const;

    struct FSDeviceDescriptor layout();
    struct FSDeviceDescriptor layoutOfPartition(isize nr);
    u8 *dataForPartition(isize nr);
    
    // Computes all possible drive geometries
    std::vector<DiskGeometry> driveGeometries(isize fileSize);
    
private:

    // Determines the drive geometry
    void deriveGeomentry();
    void predictGeometry();
    
    
    //
    // Scanning raw disk data
    //

private:
    
    // Collects drive information
    void scanDisk();
    void scanPartitions();
    void addDefaultPartition();

    // Returns a pointer to a certain block if it exists
    u8 *seekBlock(isize nr);
    
    // Return a pointer to the Rigid Disk Block if it exists
    u8 *seekRDB();
    
    // Returns a pointer to a certain partition block if it exists
    u8 *seekPB(isize nr);
    
    
    //
    // Querying partition information
    //

private:
    
    // Extracts the DOS revision number from a certain block
    FSVolumeType dos(isize blockNr);
    
    
};
