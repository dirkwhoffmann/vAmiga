// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _HDF_H
#define _HDF_H

#include "DiskFile.h"
#include "FSDevice.h"

class Disk;

class HDFFile : public AmigaFile {
    
public:
    
    //
    // Class methods
    //
    
    // Returns true iff the provided buffer contains an HDF file
    static bool isHDFBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if the provided path points to an HDF file
    static bool isHDFFile(const char *path);
    
    
    //
    // Initializing
    //

public:

    HDFFile();
    
    const char *getDescription() const override { return "HDF"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType fileType() const override { return FILETYPE_HDF; }
    bool matchingBuffer(const u8 *buffer, size_t length) override {
        return isHDFBuffer(buffer, length); }
    bool matchingFile(const char *path) override { return isHDFFile(path); }


    //
    // Querying volume information
    //

public:
    
    // Returns true if this image contains a rigid disk block
    bool hasRDB() const;
    
    // Returns the layout parameters of the hard drive
    long numCyls() const;
    long numSides() const;
    long numSectors() const;
    long numReserved() const;
    long numBlocks() const;
    long bsize() const;
    struct FSDeviceDescriptor layout();

    
    //
    // Querying partition information
    //

private:
    
    // Extract the DOS revision number from a certain block
    FSVolumeType dos(int i);    
};

#endif
