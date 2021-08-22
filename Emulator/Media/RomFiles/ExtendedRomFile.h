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

    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);

    bool isCompatiblePath(const string &path) const override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) const override { return isCompatible(stream); }


    static bool isExtendedRomFile(const string &path);

    
    //
    // Initializing
    //

    ExtendedRomFile(const string &path) throws { init(path); }
    ExtendedRomFile(const u8 *buf, isize len) throws { init(buf, len); }

    const char *getDescription() const override { return "ExtendedRom"; }


    //
    // Methods from AmigaFile
    //

    FileType type() const override { return FILETYPE_EXTENDED_ROM; }
};
