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

#include "PaulaTypes.h"
#include "CPUTypes.h"
#include "MemoryTypes.h"
#include "AgnusTypes.h"
#include "DeniseTypes.h"


//
// Enumeration types
//

typedef enum : long
{
    AMIGA_500,
    AMIGA_1000,
    AMIGA_2000
}
AmigaModel;

inline bool isAmigaModel(long model)
{
    return model >= AMIGA_500 && model <= AMIGA_2000;
}

inline const char *modelName(AmigaModel model)
{
    return
    model == AMIGA_500 ? "Amiga 500" :
    model == AMIGA_1000 ? "Amiga 1000" :
    model == AMIGA_2000 ? "Amiga 2000" : "???";
}

typedef enum : long
{
    VA_AMIGA_MODEL,
    VA_KB_LAYOUT,
    VA_CHIP_RAM,
    VA_SLOW_RAM,
    VA_FAST_RAM,
    VA_DRIVE_CONNECT,
    VA_DRIVE_TYPE,
    VA_DRIVE_SPEED,
    VA_RT_CLOCK,
    VA_EMULATE_SPRITES,
    VA_CLX_SPR_SPR,
    VA_CLX_SPR_PLF,
    VA_CLX_PLF_PLF,
    VA_FILTER_ACTIVATION,
    VA_FILTER_TYPE,
    VA_CPU_ENGINE,
    VA_CPU_SPEED,
    VA_BLITTER_ACCURACY,
    VA_FIFO_BUFFERING,
    VA_SERIAL_DEVICE
}
ConfigOption;

inline bool isConfigOption(long value)
{
    return value >= VA_AMIGA_MODEL && value <= VA_SERIAL_DEVICE;
}

typedef enum
{
    RL_SNAPSHOT           = 0b00001,
    RL_INSPECT            = 0b00010,
    RL_ENABLE_TRACING     = 0b00100,
    RL_ENABLE_BREAKPOINTS = 0b01000,
    RL_STOP               = 0b10000,
    
    RL_DEBUG              = 0b01100
}
RunLoopControlFlag;


//
// Structure types
//

typedef struct
{
    AmigaModel model;
    bool realTimeClock;
    long layout;
    FilterActivation filterActivation;
    FilterType filterType;
    CPUEngine cpuEngine;
    int cpuSpeed;
    CPUConfig cpu;
    MemoryConfig mem;
    DeniseConfig denise;
    BlitterConfig blitter;
    DiskControllerConfig diskController; 
    long serialDevice;
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
    CopperStats copper;
    AgnusStats agnus;
    DeniseStats denise;
    DiskControllerStats disk;
}
AmigaStats;

#endif

