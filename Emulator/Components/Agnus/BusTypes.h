// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

namespace vamiga {

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

struct BusOwnerEnum : util::Reflection<BusOwnerEnum, BusOwner>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = BUS_COUNT - 1;
    
    static const char *_key(long value)
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
    static const char *help(long value)
    {
        switch (value) {
                
            case BUS_NONE:     return "Free bus";
            case BUS_CPU:      return "CPU access";
            case BUS_REFRESH:  return "Memory refresh cycle";
            case BUS_DISK:     return "Disk DMA access";
            case BUS_AUD0:     return "Andio channel 0 DMA access";
            case BUS_AUD1:     return "Andio channel 1 DMA access";
            case BUS_AUD2:     return "Andio channel 2 DMA access";
            case BUS_AUD3:     return "Andio channel 3 DMA access";
            case BUS_BPL1:     return "Bitplane 1 DMA access";
            case BUS_BPL2:     return "Bitplane 2 DMA access";
            case BUS_BPL3:     return "Bitplane 3 DMA access";
            case BUS_BPL4:     return "Bitplane 4 DMA access";
            case BUS_BPL5:     return "Bitplane 5 DMA access";
            case BUS_BPL6:     return "Bitplane 6 DMA access";
            case BUS_SPRITE0:  return "Sprite 0 DMA access";
            case BUS_SPRITE1:  return "Sprite 1 DMA access";
            case BUS_SPRITE2:  return "Sprite 2 DMA access";
            case BUS_SPRITE3:  return "Sprite 3 DMA access";
            case BUS_SPRITE4:  return "Sprite 4 DMA access";
            case BUS_SPRITE5:  return "Sprite 5 DMA access";
            case BUS_SPRITE6:  return "Sprite 6 DMA access";
            case BUS_SPRITE7:  return "Sprite 7 DMA access";
            case BUS_COPPER:   return "Copper DMA access";
            case BUS_BLITTER:  return "Blitter DMA access";
            case BUS_BLOCKED:  return "Blocked bus";
            case BUS_COUNT:    return "";
        }
        return "???";
    }
};

}
