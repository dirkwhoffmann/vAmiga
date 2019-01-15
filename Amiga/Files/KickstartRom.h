// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _KICKSTARTROM_INC
#define _KICKSTARTROM_INC

#include "VAFile.h"

class KickstartRom : public VAFile {
    
private:
    
    // Accepted header signatures
    static const uint8_t magicBytes[];
    
public:
    
    //
    // Class methods
    //
    
    // Returns true iff buffer contains a Kickstart Rom image
    static bool isKickstartRomBuffer(const uint8_t *buffer, size_t length);
    
    // Returns true iff path points to a Kickstart Rom file
    static bool isKickstartRomFile(const char *path);
    
    
    //
    // Creating and destructing
    //
    
    KickstartRom();
    
    // Factory methods
    static KickstartRom *makeWithBuffer(const uint8_t *buffer, size_t length);
    static KickstartRom *makeWithFile(const char *path);
    
    
    //
    // Methods from VAFile
    //
    
    VAFileType type() override { return FILETYPE_KICKSTART_ROM; }
    const char *typeAsString() override { return "Kickstart Rom"; }
    bool hasSameType(const char *path) override { return isKickstartRomFile(path); }
    bool readFromBuffer(const uint8_t *buffer, size_t length) override;
    
};
#endif
