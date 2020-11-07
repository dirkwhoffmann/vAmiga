// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _EXT1_FILE_H
#define _EXT1_FILE_H

#include "ADFFile.h"

// THIS CLASS CONTAINS EXPERIMENTAL CODE FOR EXTENDED ADFs OF TYPE EXT1.
// THE CLASS IS NOT FUNCTIONAL YET.

class EXT1File : public DiskFile {
    
    static const int HEADER_SIZE = 160 * 4 + 8;
    
public:
        
    //
    // Class methods
    //
    
    // Returns true iff buffer contains an extended ADF in EXT1 format
    static bool isEXT1Buffer(const u8 *buffer, size_t length);
    
    // Returns true iff if path points to an extended ADF in EXT1 format
    static bool isEXT1File(const char *path);
    
    
    //
    // Initializing
    //
    
    EXT1File();
    
    static EXT1File *makeWithBuffer(const u8 *buffer, size_t length);
    static EXT1File *makeWithFile(const char *path);
    static EXT1File *makeWithFile(FILE *file);

    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_EXT1; }
    const char *typeAsString() override { return "EXT1"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isEXT1Buffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isEXT1File(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;
    
    
    //
    // Methods from DiskFile
    //
    
    DiskType getDiskType() override { return DISK_35_DD; }
    long numSides() override { return 2; }
    long numCyclinders() override { return 80; }
    long numSectorsPerTrack() override { return 11; }
    void readSector(u8 *target, long s) override { assert(false); }
    void readSector(u8 *target, long t, long s) override { assert(false); }
    
    
    //
    // Debugging
    //
    
    void dumpHeader();
};

#endif
