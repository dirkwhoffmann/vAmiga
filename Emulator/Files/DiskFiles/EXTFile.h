// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _EXT_FILE_H
#define _EXT_FILE_H

#include "ADFFile.h"

/* This class is a dummy class for detecting extended ADFs. Once the emulator
 * detects such a file, it tells the user that this format is not supported.
 */

class EXTFile : public DiskFile {
    
    static const int HEADER_SIZE = 160 * 4 + 8;
    
    // Accepted header signatures
    static const u8 extAdfHeaders[2][8];
    
public:
        
    //
    // Class methods
    //
    
    // Returns true iff buffer contains an extended ADF
    static bool isEXTBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if path points to an extended ADF
    static bool isEXTFile(const char *path);
    
    
    //
    // Initializing
    //
    
    EXTFile();
    
    const char *getDescription() const override { return "EXT"; }
    
    
    //
    // Methods from AmigaFile
    //
    
    FileType fileType() override { return FILETYPE_EXT; }
    bool matchingBuffer(const u8 *buffer, size_t length) override {
        return isEXTBuffer(buffer, length); }
    bool matchingFile(const char *path) override { return isEXTFile(path); }
    
    
    //
    // Methods from DiskFile
    //
    
    FSVolumeType getDos() const override { return FS_NODOS; }
    void setDos(FSVolumeType dos) override { };
    DiskDiameter getDiskDiameter() const override { return INCH_35; }
    DiskDensity getDiskDensity() const override { return DISK_DD; }
    long numSides() const override { return 2; }
    long numCyls() const override { return 80; }
    long numSectors() const override { return 11; }
    void readSector(u8 *target, long s) override { assert(false); }
    void readSector(u8 *target, long t, long s) override { assert(false); }
};

#endif
