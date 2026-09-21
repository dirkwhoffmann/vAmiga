// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BusTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class XRayMode
{
    XRAY_NONE,
    XRAY_DMA,
    XRAY_LAYERS
};

struct XRayModeEnum : Reflectable<XRayModeEnum, XRayMode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(XRayMode::XRAY_LAYERS);

    static const char *_key(XRayMode value)
    {
        switch (value) {

            case XRayMode::XRAY_NONE:   return "XRAY_NONE";
            case XRayMode::XRAY_DMA:    return "XRAY_DMA";
            case XRayMode::XRAY_LAYERS: return "XRAY_LAYERS";
        }
        return "???";
    }
    static const char *help(XRayMode value)
    {
        switch (value) {

            case XRayMode::XRAY_NONE:   return "Off";
            case XRayMode::XRAY_DMA:    return "DMA debugger";
            case XRayMode::XRAY_LAYERS: return "Layer inspector";
        }
        return "???";
    }
};

enum class DmaDisplayMode
{
    FG_LAYER,
    BG_LAYER,
    ODD_EVEN_LAYERS
};

struct DmaDisplayModeEnum : Reflectable<DmaDisplayModeEnum, DmaDisplayMode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DmaDisplayMode::ODD_EVEN_LAYERS);
    
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

enum class DmaChannel
{
    COPPER,
    BLITTER,
    DISK,
    AUDIO,
    SPRITE,
    BITPLANE,
    CPU,
    REFRESH,
    COUNT
};

struct DmaChannelEnum : Reflectable<DmaChannelEnum, DmaChannel>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DmaChannel::REFRESH);
    
    static const char *_key(DmaChannel value)
    {
        switch (value) {
                
            case DmaChannel::COPPER:    return "COPPER";
            case DmaChannel::BLITTER:   return "BLITTER";
            case DmaChannel::DISK:      return "DISK";
            case DmaChannel::AUDIO:     return "AUDIO";
            case DmaChannel::SPRITE:    return "SPRITE";
            case DmaChannel::BITPLANE:  return "BITPLANE";
            case DmaChannel::CPU:       return "CPU";
            case DmaChannel::REFRESH:   return "REFRESH";
            case DmaChannel::COUNT:     return "???";
        }
        return "???";
    }
    static const char *help(DmaChannel value)
    {
        switch (value) {
                
            case DmaChannel::COPPER:    return "Copper DMA";
            case DmaChannel::BLITTER:   return "Blitter DMA";
            case DmaChannel::DISK:      return "Disk DMA";
            case DmaChannel::AUDIO:     return "Audio DMA";
            case DmaChannel::SPRITE:    return "Sprite DMA";
            case DmaChannel::BITPLANE:  return "Bitplane DMA";
            case DmaChannel::CPU:       return "CPU access";
            case DmaChannel::REFRESH:   return "Refresh cycle";
            case DmaChannel::COUNT:     return "";
        }
        return "???";
    }
};

/* Number of persisted Opt::XRAY_COLORn slots. XRayMode::XRAY_DMA uses the
 * first isize(DmaChannel::COUNT) of these, indexed by DmaChannel.
 * XRayMode::XRAY_LAYERS (see PixelEngine::hide) uses all of them, indexed
 * by sprite number 0-7, then playfield 1 and playfield 2 -- both modes
 * share the same palette.
 */
constexpr isize XRAY_COLOR_COUNT = 10;


//
// Structures
//

typedef struct
{
    // X-Ray mode (off, DMA debugger, layer inspector)
    XRayMode mode;

    // If true, the visualization is blended into the displayed picture. If
    // false, it is only computed into the dedicated DMA debug texture (see
    // PixelEngine::xrayTexture), leaving the real picture untouched.
    bool overlay;

    // Individual enable switch for each DMA channel
    bool visualize[isize(DmaChannel::COUNT)];
    
    // Color palette
    u32 debugColor[XRAY_COLOR_COUNT];
    
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
