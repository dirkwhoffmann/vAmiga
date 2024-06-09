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
// Public APIs
//

struct DefaultsAPI : API {

    class Defaults *defaults = nullptr;

    DefaultsAPI(Defaults *defaults) : defaults(defaults) { }
};

struct AmigaAPI : API {

    class Amiga *amiga = nullptr;
};

struct AgnusAPI : API {

    class Agnus *agnus = nullptr;
};

struct BlitterAPI : API {

    class Blitter *blitter = nullptr;
};

struct MemoryAPI : API {

    class Memory *mem = nullptr;
};

struct CIAAPI : API {

    class CIA *cia = nullptr;
};

struct ControlPortAPI : API {

    class ControlPort *controlPort = nullptr;
};

struct CopperAPI : API {

    class Copper *copper = nullptr;
};

struct CPUAPI : API {

    class CPU *cpu = nullptr;
};

struct DebuggerAPI : API {

    class Debugger *debugger = nullptr;
};

struct DeniseAPI : API {

    class Denise *denise = nullptr;
};

struct DiskControllerAPI : API {

    class Denise *diskController = nullptr;
};

struct DmaDebuggerAPI : API {

    class DmaDebugger *dmaDebugger = nullptr;
};

struct MouseAPI : API {

    class Mouse *mouse = nullptr;
};

struct PaulaAPI : API {

    class Paula *paula = nullptr;
};


class VAmiga : public API {

public:

    static DefaultsAPI defaults;
    AmigaAPI amiga;
    AgnusAPI agnus;
    BlitterAPI blitter;
    MemoryAPI mem;
    CopperAPI copper;
    CPUAPI cpu;
    CIAAPI ciaA, ciaB;
    DeniseAPI denise;


    //
    // Static methods
    //
    
public:
    
    /** @brief  Returns a version string for this release.
     */
    static string version();

    /** @brief  Returns a build-number string for this release.
     */
    static string build();

    
    //
    // Initializing
    //
    
public:
    
    VAmiga();
    ~VAmiga();
};

}
