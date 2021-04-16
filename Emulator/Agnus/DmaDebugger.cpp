// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DmaDebugger.h"
#include "Agnus.h"
#include "Bus.h"
#include "Denise.h"
#include "MsgQueue.h"
#include "PixelEngine.h"

DmaDebugger::DmaDebugger(Amiga &ref) : AmigaComponent(ref)
{
    config.enabled = false;

    // By default all DMA channels are visualized, except the CPU channel
    for (isize i = 0; i < BUS_COUNT; i++) {
        config.visualize[i] = (i != BUS_NONE) && (i != BUS_CPU);
    }
    
    // Assign default colors
    setConfigItem(OPT_DMA_DEBUG_COLOR, BUS_CPU,     0xFFFFFF00);
    setConfigItem(OPT_DMA_DEBUG_COLOR, BUS_REFRESH, 0xFF000000);
    setConfigItem(OPT_DMA_DEBUG_COLOR, BUS_DISK,    0x00FF0000);
    setConfigItem(OPT_DMA_DEBUG_COLOR, BUS_AUDIO,   0xFF00FF00);
    setConfigItem(OPT_DMA_DEBUG_COLOR, BUS_BPL1,    0x00FFFF00);
    setConfigItem(OPT_DMA_DEBUG_COLOR, BUS_SPRITE0, 0x0088FF00);
    setConfigItem(OPT_DMA_DEBUG_COLOR, BUS_COPPER,  0xFFFF0000);
    setConfigItem(OPT_DMA_DEBUG_COLOR, BUS_BLITTER, 0xFFCC0000);
 
    config.displayMode = DMA_DISPLAY_MODE_FG_LAYER;
    config.opacity = 50;
}

i64
DmaDebugger::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_DMA_DEBUG_ENABLE: return config.enabled;
        case OPT_DMA_DEBUG_MODE: return config.displayMode;
        case OPT_DMA_DEBUG_OPACITY: return config.opacity;
                        
        default:
            assert(false);
            return 0;
    }
}

i64
DmaDebugger::getConfigItem(Option option, long id) const
{
    assert(id >= 0 && id < BUS_COUNT);
    
    switch (option) {
            
        case OPT_DMA_DEBUG_ENABLE: return config.visualize[id];
        case OPT_DMA_DEBUG_COLOR: return config.debugColor[id];
                        
        default:
            assert(false);
            return 0;
    }
}

bool
DmaDebugger::setConfigItem(Option option, i64 value)
{
    switch (option) {
                                    
        case OPT_DMA_DEBUG_ENABLE:
            
            if (config.enabled == value) {
                return false;
            }
            
            config.enabled = value;
            messageQueue.put(value ? MSG_DMA_DEBUG_ON : MSG_DMA_DEBUG_OFF);
            return true;
            
        case OPT_DMA_DEBUG_MODE:
            
            if (!DmaDisplayModeEnum::isValid(value)) {
                throw ConfigArgError(DeniseRevisionEnum::keyList());
            }
            if (config.displayMode == value) {
                return false;
            }
            
            config.displayMode = (DmaDisplayMode)value;
            return true;

        case OPT_DMA_DEBUG_OPACITY:
            
            if (config.opacity == value) {
                return false;
            }
            
            config.opacity = value;
            return true;

        default:
            return false;
    }
}

bool
DmaDebugger::setConfigItem(Option option, long id, i64 value)
{
    if (!BusOwnerEnum::isValid(id)) { return false; }
    BusOwner owner = (BusOwner)id;
    
    switch (option) {
                                    
        case OPT_DMA_DEBUG_ENABLE:
            
            switch(owner) {
                    
                case BUS_CPU:
                case BUS_REFRESH:
                case BUS_DISK:
                case BUS_AUDIO:
                case BUS_COPPER:
                case BUS_BLITTER:
                    
                    config.visualize[owner] = value;
                    return true;
        
                case BUS_BPL1:
                case BUS_BPL2:
                case BUS_BPL3:
                case BUS_BPL4:
                case BUS_BPL5:
                case BUS_BPL6:

                    config.visualize[BUS_BPL1] = value;
                    config.visualize[BUS_BPL2] = value;
                    config.visualize[BUS_BPL3] = value;
                    config.visualize[BUS_BPL4] = value;
                    config.visualize[BUS_BPL5] = value;
                    config.visualize[BUS_BPL6] = value;
                    return true;
                                         
                case BUS_SPRITE0:
                case BUS_SPRITE1:
                case BUS_SPRITE2:
                case BUS_SPRITE3:
                case BUS_SPRITE4:
                case BUS_SPRITE5:
                case BUS_SPRITE6:
                case BUS_SPRITE7:

                    config.visualize[BUS_SPRITE0] = value;
                    config.visualize[BUS_SPRITE1] = value;
                    config.visualize[BUS_SPRITE2] = value;
                    config.visualize[BUS_SPRITE3] = value;
                    config.visualize[BUS_SPRITE4] = value;
                    config.visualize[BUS_SPRITE5] = value;
                    config.visualize[BUS_SPRITE6] = value;
                    config.visualize[BUS_SPRITE7] = value;
                    return true;
                    
                default:
                    return false;
            }

        case OPT_DMA_DEBUG_COLOR:
            
            switch(owner) {
                    
                case BUS_CPU:
                case BUS_REFRESH:
                case BUS_DISK:
                case BUS_AUDIO:
                case BUS_COPPER:
                case BUS_BLITTER:
                    
                    setColor(owner, (u32)value);
                    return true;
        
                case BUS_BPL1:
                case BUS_BPL2:
                case BUS_BPL3:
                case BUS_BPL4:
                case BUS_BPL5:
                case BUS_BPL6:

                    setColor(BUS_BPL1, (u32)value);
                    setColor(BUS_BPL2, (u32)value);
                    setColor(BUS_BPL3, (u32)value);
                    setColor(BUS_BPL4, (u32)value);
                    setColor(BUS_BPL5, (u32)value);
                    setColor(BUS_BPL6, (u32)value);
                    return true;
                                         
                case BUS_SPRITE0:
                case BUS_SPRITE1:
                case BUS_SPRITE2:
                case BUS_SPRITE3:
                case BUS_SPRITE4:
                case BUS_SPRITE5:
                case BUS_SPRITE6:
                case BUS_SPRITE7:

                    setColor(BUS_SPRITE0, (u32)value);
                    setColor(BUS_SPRITE1, (u32)value);
                    setColor(BUS_SPRITE2, (u32)value);
                    setColor(BUS_SPRITE3, (u32)value);
                    setColor(BUS_SPRITE4, (u32)value);
                    setColor(BUS_SPRITE5, (u32)value);
                    setColor(BUS_SPRITE6, (u32)value);
                    setColor(BUS_SPRITE7, (u32)value);
                    return true;
                    
                default:
                    return false;
            }
        default:
            return false;
    }
}

DMADebuggerInfo
DmaDebugger::getInfo()
{
    DMADebuggerInfo result;
    
    synchronized {
                
        result.visualizeCopper = config.visualize[BUS_COPPER];
        result.visualizeBlitter = config.visualize[BUS_BLITTER];
        result.visualizeDisk = config.visualize[BUS_DISK];
        result.visualizeAudio = config.visualize[BUS_AUDIO];
        result.visualizeSprites = config.visualize[BUS_SPRITE0];
        result.visualizeBitplanes = config.visualize[BUS_BPL1];
        result.visualizeCpu = config.visualize[BUS_CPU];
        result.visualizeRefresh = config.visualize[BUS_REFRESH];
        
        getColor(BUS_COPPER, result.copperColor);
        getColor(BUS_BLITTER, result.blitterColor);
        getColor(BUS_DISK, result.diskColor);
        getColor(BUS_AUDIO, result.audioColor);
        getColor(BUS_SPRITE0, result.spriteColor);
        getColor(BUS_BPL1, result.bitplaneColor);
        getColor(BUS_CPU, result.cpuColor);
        getColor(BUS_REFRESH, result.refreshColor);
    }

    return result;
}

void
DmaDebugger::getColor(BusOwner owner, double *rgb)
{
    assert_enum(BusOwner, owner);
    
    RgbColor color = RgbColor(config.debugColor[owner]);
    rgb[0] = color.r;
    rgb[1] = color.g;
    rgb[2] = color.b;
}

void
DmaDebugger::setColor(BusOwner owner, u32 rgba)
{
    assert_enum(BusOwner, owner);
    
    config.debugColor[owner] = rgba;
                      
    // Compute the color variants that are used for drawing
    RgbColor color = RgbColor(rgba);
    debugColor[owner][0] = color.shade(0.3);
    debugColor[owner][1] = color.shade(0.1);
    debugColor[owner][2] = color.tint(0.1);
    debugColor[owner][3] = color.tint(0.3);
}

void
DmaDebugger::computeOverlay()
{
    // Only proceed if DMA debugging has been turned on
    if (!config.enabled) return;

    BusOwner *owners = agnus.busOwner;
    u16 *values = agnus.busValue;
    u32 *ptr = denise.pixelEngine.pixelAddr(0);

    double opacity = config.opacity / 100.0;
    double bgWeight = 0;
    double fgWeight = 0;

    switch (config.displayMode) {

        case DMA_DISPLAY_MODE_FG_LAYER:

            bgWeight = 0.0;
            fgWeight = 1.0 - opacity;
            break;

        case DMA_DISPLAY_MODE_BG_LAYER:

            bgWeight = 1.0 - opacity;
            fgWeight = 0.0;
            break;

        case DMA_DISPLAY_MODE_ODD_EVEN_LAYERS:

            bgWeight = opacity;
            fgWeight = 1.0 - opacity;
            break;

        default: assert(false);

    }

    for (isize i = 0; i < HPOS_CNT; i++, ptr += 4) {

        BusOwner owner = owners[i];

        // Handle the easy case first: No foreground pixels
        if (!config.visualize[owner]) {

            if (bgWeight != 0.0) {
                ptr[0] = GpuColor(ptr[0]).shade(bgWeight).rawValue;
                ptr[1] = GpuColor(ptr[1]).shade(bgWeight).rawValue;
                ptr[2] = GpuColor(ptr[2]).shade(bgWeight).rawValue;
                ptr[3] = GpuColor(ptr[3]).shade(bgWeight).rawValue;
            }
            continue;
        }

        // Get RGBA values of foreground pixels
        GpuColor col0 = debugColor[owner][(values[i] & 0xC000) >> 14];
        GpuColor col1 = debugColor[owner][(values[i] & 0x0C00) >> 10];
        GpuColor col2 = debugColor[owner][(values[i] & 0x00C0) >> 6];
        GpuColor col3 = debugColor[owner][(values[i] & 0x000C) >> 2];

        if (fgWeight != 0.0) {
            col0 = col0.mix(GpuColor(ptr[0]), fgWeight);
            col1 = col1.mix(GpuColor(ptr[1]), fgWeight);
            col2 = col2.mix(GpuColor(ptr[2]), fgWeight);
            col3 = col3.mix(GpuColor(ptr[3]), fgWeight);
        }

        ptr[0] = col0.rawValue;
        ptr[1] = col1.rawValue;
        ptr[2] = col2.rawValue;
        ptr[3] = col3.rawValue;
    }
}

void
DmaDebugger::vSyncHandler()
{
    // Only proceed if the debugger is enabled
    if (!config.enabled) return;

    // Clear old data in the next frame's VBLANK area
    u32 *ptr = denise.pixelEngine.frameBuffer->data;
    for (isize row = 0; row < VBLANK_CNT; row++) {
        for (isize col = 0; col <= LAST_PIXEL; col++) {
            ptr[row * HPIXELS + col] = PixelEngine::rgbaVBlank;
        }
    }
}
