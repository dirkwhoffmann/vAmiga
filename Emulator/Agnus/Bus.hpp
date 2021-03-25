// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Reflection.hpp"

namespace va {

#include "Bus.h"

//
// Reflection APIs
//

struct BusOwnerEnum : Reflection<BusOwnerEnum, BusOwner> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < BUS_COUNT;
    }

    static const char *prefix() { return "BUS"; }
    static const char *key(BusOwner value)
    {
        switch (value) {
                
            case BUS_NONE:     return "NONE";
            case BUS_CPU:      return "CPU";
            case BUS_REFRESH:  return "OCS";
            case BUS_DISK:     return "OCS";
            case BUS_AUDIO:    return "OCS";
            case BUS_BPL1:     return "OCS";
            case BUS_BPL2:     return "OCS";
            case BUS_BPL3:     return "OCS";
            case BUS_BPL4:     return "OCS";
            case BUS_BPL5:     return "OCS";
            case BUS_BPL6:     return "OCS";
            case BUS_SPRITE0:  return "OCS";
            case BUS_SPRITE1:  return "OCS";
            case BUS_SPRITE2:  return "OCS";
            case BUS_SPRITE3:  return "OCS";
            case BUS_SPRITE4:  return "OCS";
            case BUS_SPRITE5:  return "OCS";
            case BUS_SPRITE6:  return "OCS";
            case BUS_SPRITE7:  return "OCS";
            case BUS_COPPER:   return "OCS";
            case BUS_BLITTER:  return "OCS";
            case BUS_COUNT:    return "???";
        }
        return "???";
    }
};

}
