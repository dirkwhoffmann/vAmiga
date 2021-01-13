// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaPublicTypes.h"

#include "AgnusTypes.h"
#include "CPUTypes.h"
#include "CIATypes.h"
#include "DeniseTypes.h"
#include "DiskTypes.h"
#include "DmaDebuggerTypes.h"
#include "DriveTypes.h"
#include "EventHandlerTypes.h"
#include "FileTypes.h"
#include "FSTypes.h"
#include "KeyboardTypes.h"
#include "MemoryTypes.h"
#include "MsgQueueTypes.h"
#include "PaulaTypes.h"
#include "PortTypes.h"
#include "RTCTypes.h"

//
// Private types
//

enum_u32(RunLoopControlFlag)
{
    RL_STOP               = 0b000001,
    RL_INSPECT            = 0b000010,
    RL_BREAKPOINT_REACHED = 0b000100,
    RL_WATCHPOINT_REACHED = 0b001000,
    RL_AUTO_SNAPSHOT      = 0b010000,
    RL_USER_SNAPSHOT      = 0b100000
};
