// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _BOOTROM_INC
#define _BOOTROM_INC

#include "AmigaFile.h"

/* Instead of a Kickstart Rom, the Amiga 1000 only has a stripped down Boot
 * Rom which loads the Kickstart from disk.
 * The Boot Rom is a 64 KB Rom, but only the first 8 KB are used.
 */
 class BootRom : public AmigaFile {
    
private:
     
    // Accepted header signatures
    static const uint8_t magicBytes1[];
    static const uint8_t magicBytes2[];
    static const uint8_t magicBytes3[];
    static const uint8_t magicBytes4[];

public:
    
    //
    // Class methods
    //
    
    // Returns true iff buffer contains a Boot Rom image
    static bool isBootRomBuffer(const uint8_t *buffer, size_t length);
    
    // Returns true iff path points to a Boot Rom file
    static bool isBootRomFile(const char *path);
  
    
    //
    // Creating and destructing
    //
    
    BootRom();
    
    // Factory methods
    static BootRom *makeWithBuffer(const uint8_t *buffer, size_t length);
    static BootRom *makeWithFile(const char *path);
    
     
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_BOOT_ROM; }
    const char *typeAsString() override { return "Boot Rom"; }
    bool bufferHasSameType(const uint8_t *buffer, size_t length) override {
        return isBootRomBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isBootRomFile(path); }
    bool readFromBuffer(const uint8_t *buffer, size_t length) override;
    
};

#endif
