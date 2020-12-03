// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DISK_FILE_H
#define _DISK_FILE_H

#include "AmigaFile.h"

// Base class of all file types encoding a disk
class DiskFile : public AmigaFile {

    //
    // Initializing
    //

public:

    static DiskFile *makeWithFile(const char *path);
    
    
    //
    // Querying disk properties
    //
    
public:

    // Returns the file system this disk is formatted with
    virtual FSVolumeType dos() = 0; 
    
    // Returns the layout parameters for this disk
    virtual DiskType getDiskType() = 0;
    virtual DiskDensity getDiskDensity() = 0;
    virtual long numSides() = 0;
    virtual long numCylinders() = 0;
    virtual long numSectors() = 0;
    virtual long numTracks() { return numSides() * numCylinders(); }
    virtual long numBlocks() { return numTracks() * numSectors(); }
    
    
    //
    // Reading data
    //
    
public:

    // Reads a single data byte
    virtual u8 readByte(long b, long offset);
    virtual u8 readByte(long t, long s, long offset);

    // Fills a buffer with the data of a single sector
    virtual void readSector(u8 *dst, long b);
    virtual void readSector(u8 *dst, long t, long s);

    // Writes a string representation into the provided buffer
    virtual void readSectorHex(char *dst, long b, size_t count);
    virtual void readSectorHex(char *dst, long t, long s, size_t count);

    
    //
    // Encoding
    //
 
public:
    
    virtual bool encodeDisk(class Disk *disk);
    virtual bool decodeDisk(class Disk *disk);
};

#endif
