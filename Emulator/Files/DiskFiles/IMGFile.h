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

#define IMGSIZE_35_DD     737280  //  720 KB PC formatted disk

class IMGFile : public DiskFile {
    
public:
    
    //
    // Class methods
    //
    
    static bool isCompatibleName(const std::string &name);
    static bool isCompatibleStream(std::istream &stream);

    // Returns true iff the provided buffer contains an IMG file
    static bool isIMGBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if the provided path points to an IMG file
    static bool isIMGFile(const char *path);
    
    
    //
    // Initializing
    //

public:

    IMGFile();
    
    const char *getDescription() const override { return "IMG"; }
    
    static IMGFile *makeWithDiskType(DiskDiameter t, DiskDensity d);
    static IMGFile *makeWithDisk(Disk *disk);
  
    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType type() const override { return FILETYPE_IMG; }
    [[deprecated]] bool matchingBuffer(const u8 *buffer, size_t length) override {
        return isIMGBuffer(buffer, length); }
    [[deprecated]] bool matchingFile(const char *path) override { return isIMGFile(path); }
    
    
    //
    // Methods from DiskFile
    //
      
    FSVolumeType getDos() const override { return FS_NODOS; }
    void setDos(FSVolumeType dos) override { };
    DiskDiameter getDiskDiameter() const override { return INCH_35; }
    DiskDensity getDiskDensity() const override { return DISK_DD; }
    long numSides() const override;
    long numCyls() const override;
    long numSectors() const override;
    bool encodeDisk(class Disk *disk) override;
    bool decodeDisk(class Disk *disk) override;

private:
    
    bool encodeTrack(class Disk *disk, Track t);
    bool encodeSector(class Disk *disk, Track t, Sector s);

    bool decodeTrack(class Disk *disk, Track t);
    bool decodeSector(u8 *dst, u8 *src);
};
