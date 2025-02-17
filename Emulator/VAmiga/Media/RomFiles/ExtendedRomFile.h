// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyFile.h"

namespace vamiga {

class ExtendedRomFile : public AnyFile {

private:

    // Accepted header signatures
    static const u8 magicBytes1[];
    static const u8 magicBytes2[];

public:

    static bool isCompatible(const fs::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);

    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }


    // static bool isExtendedRomFile(const fs::path &path);

    
    //
    // Initializing
    //

    ExtendedRomFile(const fs::path &path) throws { init(path); }
    ExtendedRomFile(const u8 *buf, isize len) throws { init(buf, len); }

    const char *objectName() const override { return "ExtendedRom"; }


    //
    // Methods from AnyFile
    //

    FileType type() const override { return FileType::EXTENDED_ROM; }
};

}
