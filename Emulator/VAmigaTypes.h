// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file
/// 
#pragma once

#include "VAmiga/EmulatorTypes.h"

// Foundation
#include "VAmiga/Foundation/Constants.h"
#include "VAmiga/Foundation/CmdQueueTypes.h"
#include "VAmiga/Foundation/CoreComponentTypes.h"
#include "VAmiga/Foundation/DumpableTypes.h"
#include "VAmiga/Foundation/ErrorTypes.h"
#include "VAmiga/Foundation/GuardListTypes.h"
#include "VAmiga/Foundation/MsgQueueTypes.h"

// Components
#include "VAmiga/Components/AmigaTypes.h"
#include "VAmiga/Components/Agnus/AgnusTypes.h"
#include "VAmiga/Components/CIA/CIATypes.h"
#include "VAmiga/Components/CPU/CPUTypes.h"
#include "VAmiga/Components/Denise/DeniseTypes.h"
#include "VAmiga/Components/Memory/MemoryTypes.h"
#include "VAmiga/Components/Paula/PaulaTypes.h"
#include "VAmiga/Components/RTC/RTCTypes.h"
#include "VAmiga/Components/Zorro/HdControllerTypes.h"
#include "VAmiga/Components/Zorro/ZorroBoardTypes.h"

// Ports
#include "VAmiga/Ports/AudioPortTypes.h"
#include "VAmiga/Ports/ControlPortTypes.h"
#include "VAmiga/Ports/SerialPortTypes.h"
#include "VAmiga/Ports/VideoPortTypes.h"

// Peripherals
#include "VAmiga/Peripherals/Drive/FloppyDriveTypes.h"
#include "VAmiga/Peripherals/Drive/HardDriveTypes.h"
#include "VAmiga/Peripherals/Joystick/JoystickTypes.h"
#include "VAmiga/Peripherals/Keyboard/KeyboardTypes.h"
#include "VAmiga/Peripherals/Mouse/MouseTypes.h"

// Media
#include "VAmiga/Media/MediaFileTypes.h"
#include "VAmiga/FileSystems/FSTypes.h"

// Miscellaneous
#include "VAmiga/Misc/LogicAnalyzer/LogicAnalyzerTypes.h"
#include "VAmiga/Misc/Recorder/RecorderTypes.h"
#include "VAmiga/Misc/RemoteServers/RemoteManagerTypes.h"
#include "VAmiga/Misc/RemoteServers/RemoteServerTypes.h"
#include "VAmiga/Misc/RetroShell/RetroShellTypes.h"
