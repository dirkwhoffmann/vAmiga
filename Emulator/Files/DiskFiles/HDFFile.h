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
#include "FSVolume.h"

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
    
    const char *getDescription() override { return "HDF"; }

    static HDFFile *makeWithFile(const char *path);
    static HDFFile *makeWithBuffer(const u8 *buffer, size_t length);
    // static HDFFile *makeWithFile(FILE *file);
    // static HDFFile *makeWithVolume(FSVolume &volume, FSError *error);

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    AmigaFileType fileType() override { return FILETYPE_HDF; }
    const char *typeAsString() override { return "HDF"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isHDFBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isHDFFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;


    //
    // Querying volume information
    //

public:
    
    long numBlocks() { return size / 512; }

};

#endif
