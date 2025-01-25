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
#include "BusTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class DmaDisplayMode
{
    FG_LAYER,
    BG_LAYER,
    ODD_EVEN_LAYERS
};

struct DmaDisplayModeEnum : util::Reflection<DmaDisplayModeEnum, DmaDisplayMode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DmaDisplayMode::ODD_EVEN_LAYERS);
    
    static const char *prefix() { return "DMA_DISPLAY_MODE"; }
    static const char *_key(DmaDisplayMode value)
    {
        switch (value) {
                
            case DmaDisplayMode::FG_LAYER:        return "FG_LAYER";
            case DmaDisplayMode::BG_LAYER:        return "BG_LAYER";
            case DmaDisplayMode::ODD_EVEN_LAYERS: return "ODD_EVEN_LAYERS";
        }
        return "???";
    }
    static const char *help(DmaDisplayMode value)
    {
        switch (value) {
                
            case DmaDisplayMode::FG_LAYER:        return "Foreground layer";
            case DmaDisplayMode::BG_LAYER:        return "Background layer";
            case DmaDisplayMode::ODD_EVEN_LAYERS: return "Mixed layers";
        }
        return "???";
    }
};

enum_long(DMA_CHANNEL)
{
    DMA_CHANNEL_COPPER,
    DMA_CHANNEL_BLITTER,
    DMA_CHANNEL_DISK,
    DMA_CHANNEL_AUDIO,
    DMA_CHANNEL_SPRITE,
    DMA_CHANNEL_BITPLANE,
    DMA_CHANNEL_CPU,
    DMA_CHANNEL_REFRESH,
    
    DMA_CHANNEL_COUNT,
};
typedef DMA_CHANNEL DmaChannel;

struct DmaChannelEnum : util::Reflection<DmaChannelEnum, DmaChannel>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = DMA_CHANNEL_COUNT - 1;
    
    static const char *prefix() { return "DMA_CHANNEL"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case DMA_CHANNEL_COPPER:    return "COPPER";
            case DMA_CHANNEL_BLITTER:   return "BLITTER";
            case DMA_CHANNEL_DISK:      return "DISK";
            case DMA_CHANNEL_AUDIO:     return "AUDIO";
            case DMA_CHANNEL_SPRITE:    return "SPRITE";
            case DMA_CHANNEL_BITPLANE:  return "BITPLANE";
            case DMA_CHANNEL_CPU:       return "CPU";
            case DMA_CHANNEL_REFRESH:   return "REFRESH";
            case DMA_CHANNEL_COUNT:     return "???";
        }
        return "???";
    }
    static const char *help(long value)
    {
        switch (value) {
                
            case DMA_CHANNEL_COPPER:    return "Copper DMA";
            case DMA_CHANNEL_BLITTER:   return "Blitter DMA";
            case DMA_CHANNEL_DISK:      return "Disk DMA";
            case DMA_CHANNEL_AUDIO:     return "Audio DMA";
            case DMA_CHANNEL_SPRITE:    return "Sprite DMA";
            case DMA_CHANNEL_BITPLANE:  return "Bitplane DMA";
            case DMA_CHANNEL_CPU:       return "CPU access";
            case DMA_CHANNEL_REFRESH:   return "Refresh cycle";
            case DMA_CHANNEL_COUNT:     return "";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    // Global enable switch
    bool enabled;
    
    // Individual enable switch for each DMA channel
    bool visualize[DMA_CHANNEL_COUNT];
    
    // Color palette
    u32 debugColor[DMA_CHANNEL_COUNT];
    
    // Display mode
    DmaDisplayMode displayMode;
    
    // Opacity
    isize opacity;
}
DmaDebuggerConfig;

typedef struct
{
    bool visualizeCopper;
    bool visualizeBlitter;
    bool visualizeDisk;
    bool visualizeAudio;
    bool visualizeSprites;
    bool visualizeBitplanes;
    bool visualizeCpu;
    bool visualizeRefresh;
    
    double copperColor[3];
    double blitterColor[3];
    double diskColor[3];
    double audioColor[3];
    double spriteColor[3];
    double bitplaneColor[3];
    double cpuColor[3];
    double refreshColor[3];
}
DmaDebuggerInfo;

}
