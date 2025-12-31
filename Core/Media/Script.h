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

    
    //
    // Initializing
    //

public:

    Script(const fs::path &path) { init(path); }
    Script(const u8 *buf, isize len) { init(buf, len); }

     
    //
    // Methods from AnyFile
    //
    
public:
    
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
};

}
