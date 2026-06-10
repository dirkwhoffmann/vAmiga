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

class Workspace : public AnyFile {
    
public:
    
    static bool isCompatible(const fs::path &path);

    
    //
    // Initializing
    //
    
    Workspace(const fs::path &path) { init(path); }
    
private:
    
    void init(const fs::path &path);
    
    
    //
    // Methods from AnyFile
    //
    
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
};

}
