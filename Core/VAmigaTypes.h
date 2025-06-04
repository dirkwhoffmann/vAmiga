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

// Infrastructure
#include "Infrastructure/EmulatorTypes.h"
#include "Infrastructure/Constants.h"
#include "Infrastructure/CmdQueueTypes.h"
#include "Infrastructure/CoreComponentTypes.h"
#include "Infrastructure/DumpableTypes.h"
#include "Infrastructure/ErrorTypes.h"
#include "Infrastructure/GuardListTypes.h"
#include "Infrastructure/MsgQueueTypes.h"

// Components
#include "Components/AmigaTypes.h"
#include "Components/Agnus/AgnusTypes.h"
#include "Components/CIA/CIATypes.h"
#include "Components/CPU/CPUTypes.h"
#include "Components/Denise/DeniseTypes.h"
#include "Components/Memory/MemoryTypes.h"
#include "Components/Paula/PaulaTypes.h"
#include "Components/RTC/RTCTypes.h"
#include "Components/Zorro/HdControllerTypes.h"
#include "Components/Zorro/ZorroBoardTypes.h"

// Ports
#include "Ports/AudioPortTypes.h"
#include "Ports/ControlPortTypes.h"
#include "Ports/SerialPortTypes.h"
#include "Ports/VideoPortTypes.h"

// Peripherals
#include "Peripherals/Drive/FloppyDriveTypes.h"
#include "Peripherals/Drive/HardDriveTypes.h"
#include "Peripherals/Joystick/JoystickTypes.h"
#include "Peripherals/Keyboard/KeyboardTypes.h"
#include "Peripherals/Mouse/MouseTypes.h"
#include "Peripherals/Monitor/MonitorTypes.h"

// Media
#include "Media/MediaFileTypes.h"
#include "FileSystems/FSTypes.h"

// Miscellaneous
#include "Misc/LogicAnalyzer/LogicAnalyzerTypes.h"
#include "Misc/Recorder/RecorderTypes.h"
#include "Misc/RemoteServers/RemoteManagerTypes.h"
#include "Misc/RemoteServers/RemoteServerTypes.h"
#include "Misc/RetroShell/RetroShellTypes.h"
