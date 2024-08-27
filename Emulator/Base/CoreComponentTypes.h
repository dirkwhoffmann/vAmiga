// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"


//
// Enumerations
//

enum_long(COMP_TYPE)
{
    GenericClass,

    // Internal components
    AgnusClass,
    AmigaClass,
    AudioFilterClass,
    BlitterClass,
    CopperClass,
    CopperDebuggerClass,
    CIAClass,
    ConsoleClass,
    CPUClass,
    DeniseClass,
    DeniseDebuggerClass,
    DiskControllerClass,
    DmaDebuggerClass,
    HdControllerClass,
    HostClass,
    MemoryClass,
    MemoryDebuggerClass,
    OSDebuggerClass,
    PaulaClass,
    PixelEngineClass,
    RecorderClass,
    RegressionTesterClass,
    RetroShellClass,
    SequencerClass,
    StateMachineClass,
    RTCClass,
    TODClass,
    UARTClass,
    ZorroBoardClass,
    ZorroManagerClass,

    // Ports
    AudipPortClass,
    ControlPortClass,
    SerialPortClass,
    VideoPortClass,
    SampleRateDetectorClass,

    // Peripherals
    FloppyDriveClass,
    HardDriveClass,
    JoystickClass,
    KeyboardClass,
    MouseClass,
    PaddleClass
};
typedef COMP_TYPE CType;
