// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

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
    
    static bool isCompatibleName(const std::string &name);
    static bool isCompatibleStream(std::istream &stream);

    // Returns true iff buffer contains an extended ADF
    [[deprecated]] static bool isEXTBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if path points to an extended ADF
    [[deprecated]] static bool isEXTFile(const char *path);
    
    
    //
    // Methods from AmigaObject
    //

    const char *getDescription() const override { return "EXT"; }
    
    
    //
    // Methods from AmigaFile
    //
    
    FileType type() const override { return FILETYPE_EXT; }
    
    
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
    void readSector(u8 *target, long s) const override { assert(false); }
    void readSector(u8 *target, long t, long s) const override { assert(false); }
};
