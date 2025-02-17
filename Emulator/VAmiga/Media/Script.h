// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyFile.h"

namespace vamiga {

class Amiga;

class Script : public AnyFile {
    
public:
    
    static bool isCompatible(const fs::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
    
    //
    // Initializing
    //

public:

    Script(const fs::path &path) throws { init(path); }
    Script(const u8 *buf, isize len) throws { init(buf, len); }

    const char *objectName() const override { return "Script"; }

    
    //
    // Methods from AnyFile
    //
    
public:
    
    FileType type() const override { return FileType::SCRIPT; }
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }

    
    //
    // Processing
    //
    
    // Executes the script
    void execute(Amiga &amiga);
};

}
