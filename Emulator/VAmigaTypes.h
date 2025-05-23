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

#include "VACore/EmulatorTypes.h"

// Foundation
#include "VACore/Foundation/Constants.h"
#include "VACore/Foundation/CmdQueueTypes.h"
#include "VACore/Foundation/CoreComponentTypes.h"
#include "VACore/Foundation/DumpableTypes.h"
#include "VACore/Foundation/ErrorTypes.h"
#include "VACore/Foundation/GuardListTypes.h"
#include "VACore/Foundation/MsgQueueTypes.h"

// Components
#include "VACore/Components/AmigaTypes.h"
#include "VACore/Components/Agnus/AgnusTypes.h"
#include "VACore/Components/CIA/CIATypes.h"
#include "VACore/Components/CPU/CPUTypes.h"
#include "VACore/Components/Denise/DeniseTypes.h"
#include "VACore/Components/Memory/MemoryTypes.h"
#include "VACore/Components/Paula/PaulaTypes.h"
#include "VACore/Components/RTC/RTCTypes.h"
#include "VACore/Components/Zorro/HdControllerTypes.h"
#include "VACore/Components/Zorro/ZorroBoardTypes.h"

// Ports
#include "VACore/Ports/AudioPortTypes.h"
#include "VACore/Ports/ControlPortTypes.h"
#include "VACore/Ports/SerialPortTypes.h"
#include "VACore/Ports/VideoPortTypes.h"

// Peripherals
#include "VACore/Peripherals/Drive/FloppyDriveTypes.h"
#include "VACore/Peripherals/Drive/HardDriveTypes.h"
#include "VACore/Peripherals/Joystick/JoystickTypes.h"
#include "VACore/Peripherals/Keyboard/KeyboardTypes.h"
#include "VACore/Peripherals/Mouse/MouseTypes.h"

// Media
#include "VACore/Media/MediaFileTypes.h"
#include "VACore/FileSystems/FSTypes.h"

// Miscellaneous
#include "VACore/Misc/LogicAnalyzer/LogicAnalyzerTypes.h"
#include "VACore/Misc/Recorder/RecorderTypes.h"
#include "VACore/Misc/RemoteServers/RemoteManagerTypes.h"
#include "VACore/Misc/RemoteServers/RemoteServerTypes.h"
#include "VACore/Misc/RetroShell/RetroShellTypes.h"
