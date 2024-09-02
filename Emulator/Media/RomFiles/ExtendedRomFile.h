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

namespace vamiga {

class ExtendedRomFile : public AmigaFile {

private:

    // Accepted header signatures
    static const u8 magicBytes1[];
    static const u8 magicBytes2[];

public:

    static bool isCompatible(const std::filesystem::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);

    bool isCompatiblePath(const std::filesystem::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }


    // static bool isExtendedRomFile(const std::filesystem::path &path);

    
    //
    // Initializing
    //

    ExtendedRomFile(const std::filesystem::path &path) throws { init(path); }
    ExtendedRomFile(const u8 *buf, isize len) throws { init(buf, len); }

    const char *objectName() const override { return "ExtendedRom"; }


    //
    // Methods from AmigaFile
    //

    FileType type() const override { return FILETYPE_EXTENDED_ROM; }
};

}
