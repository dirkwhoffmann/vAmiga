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

enum_long(DMA_DISPLAY_MODE)
{
    DMA_DISPLAY_MODE_FG_LAYER,
    DMA_DISPLAY_MODE_BG_LAYER,
    DMA_DISPLAY_MODE_ODD_EVEN_LAYERS
};
typedef DMA_DISPLAY_MODE DmaDisplayMode;

#ifdef __cplusplus
struct DmaDisplayModeEnum : vamiga::util::Reflection<DmaDisplayModeEnum, DmaDisplayMode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = DMA_DISPLAY_MODE_ODD_EVEN_LAYERS;

    static const char *prefix() { return "DMA_DISPLAY_MODE"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case DMA_DISPLAY_MODE_FG_LAYER:        return "FG_LAYER";
            case DMA_DISPLAY_MODE_BG_LAYER:        return "BG_LAYER";
            case DMA_DISPLAY_MODE_ODD_EVEN_LAYERS: return "ODD_EVEN_LAYERS";
        }
        return "???";
    }
};
#endif

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

#ifdef __cplusplus
struct DmaChannelEnum : vamiga::util::Reflection<DmaChannelEnum, DmaChannel>
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
};
#endif


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
