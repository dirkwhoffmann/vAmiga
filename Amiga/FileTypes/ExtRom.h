// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _EXTROM_INC
#define _EXTROM_INC

#include "AmigaFile.h"

class ExtRom : public AmigaFile {

private:

    // Accepted header signatures
    static const uint8_t magicBytes1[];

public:

    //
    // Class methods
    //

    // Returns true iff buffer contains an Extended Rom image
    static bool isExtRomBuffer(const uint8_t *buffer, size_t length);

    // Returns true iff path points to a Extended Rom file
    static bool isExtRomFile(const char *path);


    //
    // Creating and destructing
    //

    ExtRom();

    // Factory methods
    static ExtRom *makeWithBuffer(const uint8_t *buffer, size_t length);
    static ExtRom *makeWithFile(const char *path);


    //
    // Methods from AmigaFile
    //

    AmigaFileType fileType() override { return FILETYPE_KICK_ROM; }
    const char *typeAsString() override { return "Extended Rom"; }
    bool bufferHasSameType(const uint8_t *buffer, size_t length) override {
        return isExtRomBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isExtRomFile(path); }
    bool readFromBuffer(const uint8_t *buffer, size_t length) override;

};

#endif
