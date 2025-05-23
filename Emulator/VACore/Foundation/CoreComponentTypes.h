// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VACore/Foundation/Types.h"

namespace vamiga {

//
// Enumerations
//

enum class Class : long
{
    Generic,
    
    // Internal components
    Agnus,
    Amiga,
    AudioFilter,
    Blitter,
    Copper,
    CopperDebugger,
    CIA,
    Console,
    CPU,
    Denise,
    DeniseDebugger,
    DiskController,
    DmaDebugger,
    HdController,
    Host,
    LogicAnalyzer,
    Memory,
    MemoryDebugger,
    OSDebugger,
    Paula,
    PixelEngine,
    Recorder,
    RegressionTester,
    RetroShell,
    Sequencer,
    StateMachine,
    RTC,
    TOD,
    UART,
    ZorroBoard,
    ZorroManager,
    
    // Ports
    AudioPort,
    ControlPort,
    SerialPort,
    VideoPort,
    SampleRateDetector,
    
    // Peripherals
    FloppyDrive,
    HardDrive,
    Joystick,
    Keyboard,
    Mouse,
    Paddl,
};

}
