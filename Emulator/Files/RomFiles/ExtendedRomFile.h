// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"

class ExtendedRomFile : public AmigaFile {

private:

    // Accepted header signatures
    static const u8 magicBytes1[];
    static const u8 magicBytes2[];

public:

    //
    // Class methods
    //

    static bool isCompatibleName(const std::string &name);
    static bool isCompatibleStream(std::istream &stream);

    // Returns true iff buffer contains an Extended Rom image
    static bool isExtendedRomBuffer(const u8 *buffer, size_t length);

    // Returns true iff path points to a Extended Rom file
    static bool isExtendedRomFile(const char *path);


    //
    // Initializing
    //

    ExtendedRomFile();

    const char *getDescription() const override { return "ExtendedRom"; }


    //
    // Methods from AmigaFile
    //

    FileType type() const override { return FILETYPE_EXTENDED_ROM; }
};
