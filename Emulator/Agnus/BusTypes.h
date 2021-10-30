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

enum_i8(BUS_OWNER)
{
    BUS_NONE,
    BUS_CPU,
    BUS_REFRESH,
    BUS_DISK,
    BUS_AUD0,
    BUS_AUD1,
    BUS_AUD2,
    BUS_AUD3,
    BUS_BPL1,
    BUS_BPL2,
    BUS_BPL3,
    BUS_BPL4,
    BUS_BPL5,
    BUS_BPL6,
    BUS_SPRITE0,
    BUS_SPRITE1,
    BUS_SPRITE2,
    BUS_SPRITE3,
    BUS_SPRITE4,
    BUS_SPRITE5,
    BUS_SPRITE6,
    BUS_SPRITE7,
    BUS_COPPER,
    BUS_BLITTER,
    BUS_BLOCKED,
    
    BUS_COUNT
};
typedef BUS_OWNER BusOwner;

#ifdef __cplusplus
struct BusOwnerEnum : util::Reflection<BusOwnerEnum, BusOwner>
{
    static long min() { return 0; }
    static long max() { return BUS_COUNT - 1; }
    static bool isValid(long value) { return value >= min() && value <= max(); }

    static const char *prefix() { return "BUS"; }
    static const char *key(BusOwner value)
    {
        switch (value) {
                
            case BUS_NONE:     return "NONE";
            case BUS_CPU:      return "CPU";
            case BUS_REFRESH:  return "REFRESH";
            case BUS_DISK:     return "DISK";
            case BUS_AUD0:     return "AUD0";
            case BUS_AUD1:     return "AUD1";
            case BUS_AUD2:     return "AUD2";
            case BUS_AUD3:     return "AUD3";
            case BUS_BPL1:     return "BPL1";
            case BUS_BPL2:     return "BPL2";
            case BUS_BPL3:     return "BPL3";
            case BUS_BPL4:     return "BPL4";
            case BUS_BPL5:     return "BPL5";
            case BUS_BPL6:     return "BPL6";
            case BUS_SPRITE0:  return "SPRITE0";
            case BUS_SPRITE1:  return "SPRITE1";
            case BUS_SPRITE2:  return "SPRITE2";
            case BUS_SPRITE3:  return "SPRITE3";
            case BUS_SPRITE4:  return "SPRITE4";
            case BUS_SPRITE5:  return "SPRITE5";
            case BUS_SPRITE6:  return "SPRITE6";
            case BUS_SPRITE7:  return "SPRITE7";
            case BUS_COPPER:   return "COPPER";
            case BUS_BLITTER:  return "BLITTER";
            case BUS_BLOCKED:  return "BLOCKED";
            case BUS_COUNT:    return "???";
        }
        return "???";
    }
};
#endif
