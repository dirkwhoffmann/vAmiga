// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(CONFIG_SCHEME)
{
    CONFIG_A500_OCS_1MB,
    CONFIG_A500_ECS_1MB
};
typedef CONFIG_SCHEME ConfigScheme;

#ifdef __cplusplus
struct ConfigSchemeEnum : util::Reflection<ConfigSchemeEnum, ConfigScheme> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <= CONFIG_A500_ECS_1MB;
    }

    static const char *prefix() { return "CONFIG"; }
    static const char *key(ConfigScheme value)
    {
        switch (value) {
                
            case CONFIG_A500_OCS_1MB:  return "A500_OCS_1MB";
            case CONFIG_A500_ECS_1MB:  return "A500_ECS_1MB";
        }
        return "???";
    }
};
#endif


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


//
// Private data types
//

#ifdef __cplusplus

typedef u32 RunLoopFlags;

namespace RL
{
constexpr u32 STOP               = 0b000000001;
constexpr u32 INSPECT            = 0b000000010;
constexpr u32 WARP_ON            = 0b000000100;
constexpr u32 WARP_OFF           = 0b000001000;
constexpr u32 BREAKPOINT_REACHED = 0b000010000;
constexpr u32 WATCHPOINT_REACHED = 0b000100000;
constexpr u32 AUTO_SNAPSHOT      = 0b001000000;
constexpr u32 USER_SNAPSHOT      = 0b010000000;
constexpr u32 SYNC_THREAD        = 0b100000000;
};

#endif
