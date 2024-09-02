// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"

namespace vamiga {

class Amiga;

class Script : public AmigaFile {
    
public:
    
    static bool isCompatible(const std::filesystem::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
    bool isCompatiblePath(const std::filesystem::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }

    
    //
    // Initializing
    //

public:

    Script(const std::filesystem::path &path) throws { init(path); }
    Script(const u8 *buf, isize len) throws { init(buf, len); }

    const char *objectName() const override { return "Script"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType type() const override { return FILETYPE_SCRIPT; }

    
    //
    // Processing
    //
    
    // Executes the script
    void execute(Amiga &amiga);
};

}
