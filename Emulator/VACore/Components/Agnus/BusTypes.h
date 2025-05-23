// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VACore/Foundation/Reflection.h"

namespace vamiga {

enum class BusOwner : i8
{
    NONE,
    CPU,
    REFRESH,
    DISK,
    AUD0,
    AUD1,
    AUD2,
    AUD3,
    BPL1,
    BPL2,
    BPL3,
    BPL4,
    BPL5,
    BPL6,
    SPRITE0,
    SPRITE1,
    SPRITE2,
    SPRITE3,
    SPRITE4,
    SPRITE5,
    SPRITE6,
    SPRITE7,
    COPPER,
    BLITTER,
    BLOCKED,
    
    COUNT
};

static constexpr i8 BUS_NONE = i8(BusOwner::NONE);
static constexpr i8 BUS_CPU = i8(BusOwner::CPU);
static constexpr i8 BUS_REFRESH = i8(BusOwner::REFRESH);
static constexpr i8 BUS_DISK = i8(BusOwner::DISK);
static constexpr i8 BUS_AUD0 = i8(BusOwner::AUD0);
static constexpr i8 BUS_AUD1 = i8(BusOwner::AUD1);
static constexpr i8 BUS_AUD2 = i8(BusOwner::AUD2);
static constexpr i8 BUS_AUD3 = i8(BusOwner::AUD3);
static constexpr i8 BUS_BPL1 = i8(BusOwner::BPL1);
static constexpr i8 BUS_BPL2 = i8(BusOwner::BPL2);
static constexpr i8 BUS_BPL3 = i8(BusOwner::BPL3);
static constexpr i8 BUS_BPL4 = i8(BusOwner::BPL4);
static constexpr i8 BUS_BPL5 = i8(BusOwner::BPL5);
static constexpr i8 BUS_BPL6 = i8(BusOwner::BPL6);
static constexpr i8 BUS_SPRITE0 = i8(BusOwner::SPRITE0);
static constexpr i8 BUS_SPRITE1 = i8(BusOwner::SPRITE1);
static constexpr i8 BUS_SPRITE2 = i8(BusOwner::SPRITE2);
static constexpr i8 BUS_SPRITE3 = i8(BusOwner::SPRITE3);
static constexpr i8 BUS_SPRITE4 = i8(BusOwner::SPRITE4);
static constexpr i8 BUS_SPRITE5 = i8(BusOwner::SPRITE5);
static constexpr i8 BUS_SPRITE6 = i8(BusOwner::SPRITE6);
static constexpr i8 BUS_SPRITE7 = i8(BusOwner::SPRITE7);
static constexpr i8 BUS_COPPER = i8(BusOwner::COPPER);
static constexpr i8 BUS_BLITTER = i8(BusOwner::BLITTER);
static constexpr i8 BUS_BLOCKED = i8(BusOwner::BLOCKED);
static constexpr i8 BUS_COUNT = i8(BusOwner::COUNT);

struct BusOwnerEnum : Reflection<BusOwnerEnum, BusOwner>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(BusOwner::COUNT) -1;
    
    static const char *_key(BusOwner value)
    {
        switch (value) {
                
            case BusOwner::NONE:     return "NONE";
            case BusOwner::CPU:      return "CPU";
            case BusOwner::REFRESH:  return "REFRESH";
            case BusOwner::DISK:     return "DISK";
            case BusOwner::AUD0:     return "AUD0";
            case BusOwner::AUD1:     return "AUD1";
            case BusOwner::AUD2:     return "AUD2";
            case BusOwner::AUD3:     return "AUD3";
            case BusOwner::BPL1:     return "BPL1";
            case BusOwner::BPL2:     return "BPL2";
            case BusOwner::BPL3:     return "BPL3";
            case BusOwner::BPL4:     return "BPL4";
            case BusOwner::BPL5:     return "BPL5";
            case BusOwner::BPL6:     return "BPL6";
            case BusOwner::SPRITE0:  return "SPRITE0";
            case BusOwner::SPRITE1:  return "SPRITE1";
            case BusOwner::SPRITE2:  return "SPRITE2";
            case BusOwner::SPRITE3:  return "SPRITE3";
            case BusOwner::SPRITE4:  return "SPRITE4";
            case BusOwner::SPRITE5:  return "SPRITE5";
            case BusOwner::SPRITE6:  return "SPRITE6";
            case BusOwner::SPRITE7:  return "SPRITE7";
            case BusOwner::COPPER:   return "COPPER";
            case BusOwner::BLITTER:  return "BLITTER";
            case BusOwner::BLOCKED:  return "BLOCKED";
            case BusOwner::COUNT:    return "???";
        }
        return "???";
    }
    static const char *help(BusOwner value)
    {
        switch (value) {
                
            case BusOwner::NONE:     return "Free bus";
            case BusOwner::CPU:      return "CPU access";
            case BusOwner::REFRESH:  return "Memory refresh cycle";
            case BusOwner::DISK:     return "Disk DMA access";
            case BusOwner::AUD0:     return "Andio channel 0 DMA access";
            case BusOwner::AUD1:     return "Andio channel 1 DMA access";
            case BusOwner::AUD2:     return "Andio channel 2 DMA access";
            case BusOwner::AUD3:     return "Andio channel 3 DMA access";
            case BusOwner::BPL1:     return "Bitplane 1 DMA access";
            case BusOwner::BPL2:     return "Bitplane 2 DMA access";
            case BusOwner::BPL3:     return "Bitplane 3 DMA access";
            case BusOwner::BPL4:     return "Bitplane 4 DMA access";
            case BusOwner::BPL5:     return "Bitplane 5 DMA access";
            case BusOwner::BPL6:     return "Bitplane 6 DMA access";
            case BusOwner::SPRITE0:  return "Sprite 0 DMA access";
            case BusOwner::SPRITE1:  return "Sprite 1 DMA access";
            case BusOwner::SPRITE2:  return "Sprite 2 DMA access";
            case BusOwner::SPRITE3:  return "Sprite 3 DMA access";
            case BusOwner::SPRITE4:  return "Sprite 4 DMA access";
            case BusOwner::SPRITE5:  return "Sprite 5 DMA access";
            case BusOwner::SPRITE6:  return "Sprite 6 DMA access";
            case BusOwner::SPRITE7:  return "Sprite 7 DMA access";
            case BusOwner::COPPER:   return "Copper DMA access";
            case BusOwner::BLITTER:  return "Blitter DMA access";
            case BusOwner::BLOCKED:  return "Blocked bus";
            case BusOwner::COUNT:    return "";
        }
        return "???";
    }
};

}
