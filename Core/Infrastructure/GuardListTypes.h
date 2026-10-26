// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class GuardType : long
{
    BREAKPOINT,
    WATCHPOINT,
    CATCHPOINT,
    COPPER_BREAKPOINT,
    COPPER_WATCHPOINT,
    BEAMTRAP
};

struct GuardTypeEnum : utl::Reflectable<GuardTypeEnum, GuardType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(GuardType::BEAMTRAP);

    static const char *_key(GuardType value)
    {
        switch (value) {

            case GuardType::BREAKPOINT:         return "BREAKPOINT";
            case GuardType::WATCHPOINT:         return "WATCHPOINT";
            case GuardType::CATCHPOINT:         return "CATCHPOINT";
            case GuardType::COPPER_BREAKPOINT:  return "COPPER_BREAKPOINT";
            case GuardType::COPPER_WATCHPOINT:  return "COPPER_WATCHPOINT";
            case GuardType::BEAMTRAP:           return "BEAMTRAP";
        }
        return "???";
    }
    static const char *help(GuardType value)
    {
        switch (value) {

            case GuardType::BREAKPOINT:         return "Breakpoint";
            case GuardType::WATCHPOINT:         return "Watchpoint";
            case GuardType::CATCHPOINT:         return "Catchpoint";
            case GuardType::COPPER_BREAKPOINT:  return "Copper breakpoint";
            case GuardType::COPPER_WATCHPOINT:  return "Copper watchpoint";
            case GuardType::BEAMTRAP:           return "Beamtrap";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    u32 addr;                               ///< Observed address
    bool enabled;                           ///< The guard is enabled
    isize ignore;                           ///< Number of hits that are ignored before the guard triggers
}
GuardInfo;

}
