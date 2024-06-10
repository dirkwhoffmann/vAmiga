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

namespace moira { class Guards; }

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

struct CIAAPI : API {

    class CIA *cia = nullptr;
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

    class DiskController *diskController = nullptr;
};

struct DmaDebuggerAPI : API {

    class DmaDebugger *dmaDebugger = nullptr;
};

struct FloppyDriveAPI : API {

    class FloppyDrive *drive = nullptr;
};

struct GuardsAPI : API {

    class moira::Guards *guards = nullptr;
};

struct HardDriveAPI : API {

    class HardDrive *drive = nullptr;
};

struct HostAPI : API {

    class Host *host = nullptr;
};

struct JoystickAPI : API {

    class Joystick *joystick = nullptr;
};

struct KeyboardAPI : API {

    class Keyboard *keyboard = nullptr;
};

struct MemoryAPI : API {

    class Memory *mem = nullptr;
};

struct MouseAPI : API {

    class Mouse *mouse = nullptr;
};

struct PaulaAPI : API {

    class Paula *paula = nullptr;
};

struct RetroShellAPI : API {

    class RetroShell *retroShell = nullptr;
};

struct RtcAPI : API {

    class RTC *rtc = nullptr;
};

struct RecorderAPI : API {

    class Recorder *recorder = nullptr;
};

struct RemoteManagerAPI : API {

    class RemoteManager *remoteManager = nullptr;
};

struct SerialPortAPI : API {

    class SerialPort *serialPort = nullptr;
};

struct ControlPortAPI : API {

    class ControlPort *controlPort = nullptr;

    JoystickAPI joystick;
    MouseAPI mouse;
};

class VAmiga : public API {

public:

    static DefaultsAPI defaults;

    AmigaAPI amiga;
    AgnusAPI agnus;
    BlitterAPI blitter;
    GuardsAPI breakpoints;
    CIAAPI ciaA, ciaB;
    ControlPortAPI controlPort1;
    ControlPortAPI controlPort2;
    CopperAPI copper;
    GuardsAPI copperBreakpoints;
    CPUAPI cpu;
    DebuggerAPI debugger;
    DeniseAPI denise;
    DiskControllerAPI diskController;
    DmaDebuggerAPI dmaDebugger;
    FloppyDriveAPI df0, df1, df2, df3;
    HardDriveAPI hd0,hd1, hd2, hd3;
    HostAPI host;
    KeyboardAPI keyboard;
    MemoryAPI mem;
    PaulaAPI paula;
    RetroShellAPI retroShell;
    RtcAPI rtc;
    RecorderAPI recorder;
    RemoteManagerAPI remoteManager;
    SerialPortAPI serialPort;
    GuardsAPI watchpoints;

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
