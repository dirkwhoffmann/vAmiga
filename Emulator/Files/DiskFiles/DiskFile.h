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
    
    // Returns the type of this disk
    virtual DiskType getDiskType() = 0;
    
    // Returns the layout parameters for this disk
    virtual long numSides() = 0;
    virtual long numCyclinders() = 0;
    virtual long numTracks() { return numSides() * numCyclinders(); }
    virtual long numSectorsPerTrack() = 0;
    
    
    //
    // Reading data
    //
    
public:
    
    // Fills a buffer with the data of a single sector
    virtual void readSector(u8 *target, long s);
    virtual void readSector(u8 *target, long t, long s);
};

#endif
