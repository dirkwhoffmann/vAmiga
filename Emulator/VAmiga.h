// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmigaTypes.h"
#include "Error.h"
#include <filesystem>

namespace vamiga {

//
// Base class for all APIs
//

class API {
    
public:
    
    class Emulator *emu = nullptr;
    
    API() { }
    API(Emulator *emu) : emu(emu) { }
    
    void suspend();
    void resume();
    
    bool isUserThread() const;
};

//
// Public API
//

class VAmiga : public API {
    
    //
    // Static methods
    //
    
public:
    
    //
    // Initializing
    //
    
public:
    
    VAmiga();
    ~VAmiga();
};

}
