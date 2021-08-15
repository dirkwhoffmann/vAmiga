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

    static bool isCompatiblePath(const string &path);
    static bool isCompatibleStream(std::istream &stream);

    bool compatiblePath(const string &path) override { return isCompatiblePath(path); }
    bool compatibleStream(std::istream &stream) override { return isCompatibleStream(stream); }


    static bool isExtendedRomFile(const string &path);

    
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
