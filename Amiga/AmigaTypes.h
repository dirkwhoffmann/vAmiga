// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _AMIGA_T_INC
#define _AMIGA_T_INC

#include "Aliases.h"

#include "AgnusTypes.h"
#include "CPUTypes.h"
#include "CIATypes.h"
#include "DeniseTypes.h"
#include "DiskTypes.h"
#include "DriveTypes.h"
#include "EventHandlerTypes.h"
#include "FileTypes.h"
#include "KeyboardTypes.h"
#include "MemoryTypes.h"
#include "MessageQueueTypes.h"
#include "PaulaTypes.h"
#include "PortTypes.h"
#include "RTCTypes.h"

//
// Enumerations
//

typedef enum : long
{
    VA_AGNUS_REVISION,
    VA_DENISE_REVISION,
    VA_RT_CLOCK,
    VA_CHIP_RAM,
    VA_SLOW_RAM,
    VA_FAST_RAM,
    VA_EXT_START,
    VA_DRIVE_CONNECT,
    VA_DRIVE_TYPE,
    VA_DRIVE_SPEED,
    VA_EMULATE_SPRITES,
    VA_CLX_SPR_SPR,
    VA_CLX_SPR_PLF,
    VA_CLX_PLF_PLF,
    VA_FILTER_ACTIVATION,
    VA_FILTER_TYPE,
    VA_SAMPLING_METHOD,
    VA_BLITTER_ACCURACY,
    VA_FIFO_BUFFERING,
    VA_SERIAL_DEVICE,
    VA_TODBUG
}
ConfigOption;

inline bool isConfigOption(long value)
{
    return value >= VA_AGNUS_REVISION && value <= VA_TODBUG;
}

typedef enum
{
    RL_SNAPSHOT           = 0b00001,
    RL_INSPECT            = 0b00010,
    RL_BREAKPOINT_REACHED = 0b00100,
    RL_WATCHPOINT_REACHED = 0b01000,
    RL_STOP               = 0b10000
}
RunLoopControlFlag;

typedef enum
{
    ERR_OK,
    ERR_ROM_MISSING,
    ERR_AROS_NO_EXTROM,
    ERR_AROS_RAM_LIMIT,
    ERR_CHIP_RAM_LIMIT,
}
ErrorCode;

//
// Structures
//

typedef struct
{
    int cpuSpeed;
    CIAConfig ciaA;
    CIAConfig ciaB;
    RTCConfig rtc;
    AudioConfig audio;
    MemoryConfig mem;
    AgnusConfig agnus;
    DeniseConfig denise;
    BlitterConfig blitter;
    SerialPortConfig serialPort;
    DiskControllerConfig diskController;
    DriveConfig df0;
    DriveConfig df1;
    DriveConfig df2;
    DriveConfig df3;
}
AmigaConfiguration;

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

typedef struct
{
    MemoryStats mem;
    AgnusStats agnus;
    DeniseStats denise;
    AudioStats audio;
    UARTStats uart;
    DiskControllerStats disk;
    long frames;
}
AmigaStats;

#endif

