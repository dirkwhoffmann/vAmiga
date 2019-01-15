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

#include "VAFile.h"

class KickRom : public VAFile {
    
private:
    
    // Accepted header signatures
    static const uint8_t magicBytes[];
    
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
    // Methods from VAFile
    //
    
    VAFileType type() override { return FILETYPE_KICK_ROM; }
    const char *typeAsString() override { return "Kickstart Rom"; }
    bool hasSameType(const char *path) override { return isKickRomFile(path); }
    bool readFromBuffer(const uint8_t *buffer, size_t length) override;
    
};
#endif
