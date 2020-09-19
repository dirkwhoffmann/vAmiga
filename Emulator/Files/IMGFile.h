// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _IMG_H
#define _IMG_H

#include "AmigaFile.h"

#define IMGSIZE_35_DD     737280  //  720 KB PC formatted disk

class IMGFile : public AmigaFile {
    
public:
    
    //
    // Class methods
    //
    
    // Returns true iff the provided buffer contains an IMG file
    static bool isIMGBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if the provided path points to an IMG file
    static bool isIMGFile(const char *path);
    
    
    //
    // Initializing
    //

public:

    IMGFile();
    
    static IMGFile *makeWithBuffer(const u8 *buffer, size_t length);
    static IMGFile *makeWithFile(const char *path);
    static IMGFile *makeWithFile(FILE *file);
    static IMGFile *makeWithDisk(Disk *disk);
  
    
    //
    // Methods from AmigaFile
    //
    
public:
    
    AmigaFileType fileType() override { return FILETYPE_IMG; }
    const char *typeAsString() override { return "IMG"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isIMGBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isIMGFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;
    
    
    //
    // Properties
    //
      
    // Returns the type of this disk
    DiskType getDiskType() { return DISK_35_DD; }

    // Returns a unique fingerprint for this file
    u64 fnv();

    // Cylinder, track, and sector counts
    long numSectorsPerTrack() { return 9; };
    long numSectorsTotal() { return 1440; }
    long numTracks() { return 160; }
    long numCyclinders() { return 80; }

    // Returns the location of the root and bitmap block
    long rootBlockNr();
    long bitmapBlockNr() { return rootBlockNr() + 1; }

    // Consistency checking
    bool isCylinderNr(long nr) { return nr >= 0 && nr < numCyclinders(); }
    bool isTrackNr(long nr)    { return nr >= 0 && nr < numTracks(); }
    bool isSectorNr(long nr)   { return nr >= 0 && nr < numSectorsTotal(); }
    
    //
    // Seeking tracks and sectors
    //
    
public:
        
    // Fills the provided buffer with the data of a single sector
    void readSector(u8 *buffer, long t, long s);
};

#endif
