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
#include "FSDevice.h"

#define ADFSIZE_35_DD     901120  //  880 KB
#define ADFSIZE_35_DD_81  912384  //  891 KB (1 extra cylinder)
#define ADFSIZE_35_DD_82  923648  //  902 KB (2 extra cylinders)
#define ADFSIZE_35_DD_83  934912  //  913 KB (3 extra cylinders)
#define ADFSIZE_35_DD_84  946176  //  924 KB (4 extra cylinders)
#define ADFSIZE_35_HD    1802240  // 1760 KB

class ADFFile : public DiskFile {
    
public:
    
    //
    // Class methods
    //
    
    static bool isCompatibleName(const std::string &name);
    static bool isCompatibleStream(std::istream &stream);

    // Returns true iff the provided buffer contains an ADF file
    static bool isADFBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if the provided path points to an ADF file
    static bool isADFFile(const char *path);
    
    // Returns the size of an ADF file of a given disk type in bytes
    static size_t fileSize(DiskDiameter t, DiskDensity d);

    
    //
    // Initializing
    //

public:

    ADFFile();
    
    const char *getDescription() const override { return "ADF"; }

    static ADFFile *makeWithType(DiskDiameter t, DiskDensity d);
    static ADFFile *makeWithDisk(class Disk *disk);
    static ADFFile *makeWithDrive(class Drive *drive);
    static ADFFile *makeWithVolume(FSDevice &volume, ErrorCode *error);

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType fileType() const override { return FILETYPE_ADF; }
    bool matchingBuffer(const u8 *buffer, size_t length) override {
        return isADFBuffer(buffer, length); }
    bool matchingFile(const char *path) override { return isADFFile(path); }
    
    
    //
    // Methods from DiskFile
    //
    
public:
    
    FSVolumeType getDos() const override; 
    void setDos(FSVolumeType dos) override;
    DiskDiameter getDiskDiameter() const override;
    DiskDensity getDiskDensity() const override;
    long numSides() const override;
    long numCyls() const override;
    long numSectors() const override;
    BootBlockType bootBlockType() const override;
    const char *bootBlockName() const override;
    
    void killVirus() override;

    bool encodeDisk(class Disk *disk) override;
    bool decodeDisk(class Disk *disk) override;

private:
    
    bool encodeTrack(class Disk *disk, Track t);
    bool encodeSector(class Disk *disk, Track t, Sector s);

    bool decodeTrack(class Disk *disk, Track t);
    bool decodeSector(u8 *dst, u8 *src);

    
    //
    // Querying disk properties
    //
    
public:

    // Returns the location of the root block (DEPRECATED)
    u32 rootBlock();
    
    // Returns the location of the bitmap block (DEPRECATED)
    u32 bitmapBlock();

    // Returns the layout of this disk in form of a device descriptor
    struct FSDeviceDescriptor layout();
    
    
    //
    // Formatting
    //
 
public:
    
    bool formatDisk(FSVolumeType fs, long bootBlockID);

    
    //
    // Debugging
    //
 
public:
    
    void dumpSector(int num);
};
