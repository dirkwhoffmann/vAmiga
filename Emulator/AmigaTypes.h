// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

//
// Enumerations
//

enum_long(EMULATOR_STATE)
{
    EMULATOR_STATE_OFF,
    EMULATOR_STATE_PAUSED,
    EMULATOR_STATE_RUNNING,

    EMULATOR_STATE_COUNT
};
typedef EMULATOR_STATE EmulatorState;

enum_u32(RunLoopControlFlag)
{
    RL_STOP               = 0b00000001,
    RL_INSPECT            = 0b00000010,
    RL_WARP_ON            = 0b00000100,
    RL_WARP_OFF           = 0b00001000,
    RL_BREAKPOINT_REACHED = 0b00010000,
    RL_WATCHPOINT_REACHED = 0b00100000,
    RL_AUTO_SNAPSHOT      = 0b01000000,
    RL_USER_SNAPSHOT      = 0b10000000,
};

//
// Structures
//

typedef struct
{
    Cycle cpuClock;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;
}
AmigaInfo;
