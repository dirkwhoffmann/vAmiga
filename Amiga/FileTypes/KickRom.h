// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _KICKROM_INC
#define _KICKROM_INC

#include "AmigaFile.h"

class KickRom : public AmigaFile {
    
private:
    
    // Accepted header signatures
    static const uint8_t magicBytes1[];
    static const uint8_t magicBytes1a[];
    static const uint8_t magicBytes2[];
    static const uint8_t magicBytes3[];
    static const uint8_t magicBytes4[];

public:
    
    //
    // Class methods
    //
    
    // Returns true iff buffer contains a Kickstart Rom image
    static bool isKickRomBuffer(const uint8_t *buffer, size_t length);
    
    // Returns true iff path points to a Kickstart Rom file
    static bool isKickRomFile(const char *path);
    
    
    //
    // Creating and destructing
    //
    
    KickRom();
    
    // Factory methods
    static KickRom *makeWithBuffer(const uint8_t *buffer, size_t length);
    static KickRom *makeWithFile(const char *path);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_KICK_ROM; }
    const char *typeAsString() override { return "Kickstart Rom"; }
    bool bufferHasSameType(const uint8_t *buffer, size_t length) override {
        return isKickRomBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isKickRomFile(path); }
    bool readFromBuffer(const uint8_t *buffer, size_t length) override;
    
};

#endif
