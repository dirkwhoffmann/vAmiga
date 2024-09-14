// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DmaDebugger.h"
#include "Amiga.h"

namespace vamiga {

DmaDebugger::DmaDebugger(Amiga &ref) : SubComponent(ref)
{
    
}

void 
DmaDebugger::_dump(Category category, std::ostream& os) const
{
    auto print = [&]() {

        using namespace util;

        for (int i = 0; i < beamtraps.elements(); i++) {

            auto bp = *beamtraps.guardNr(i);
            auto v = std::to_string(HI_WORD(bp.addr));
            auto h = std::to_string(LO_WORD(bp.addr));
            os << tab("Beamtrap " + std::to_string(i));
            os << "(" + v + "," + h + ")";

            if (!bp.enabled) os << " (Disabled)";
            else if (bp.ignore) os << " (Disabled for " << dec(bp.ignore) << " hits)";
            os << std::endl;
        }
    };

    if (category == Category::Config) {
        
        dumpConfig(os);
    }

    if (category == Category::Beamtraps) {

        if (beamtraps.elements()) {
            print();
        } else {
            os << "No beamtraps set" << std::endl;
        }
    }
}

i64
DmaDebugger::getOption(Option option) const
{
    switch (option) {
            
        case OPT_DMA_DEBUG_ENABLE:      return config.enabled;
        case OPT_DMA_DEBUG_MODE:        return config.displayMode;
        case OPT_DMA_DEBUG_OPACITY:     return config.opacity;

        case OPT_DMA_DEBUG_CHANNEL0:    return config.visualize[0];
        case OPT_DMA_DEBUG_CHANNEL1:    return config.visualize[1];
        case OPT_DMA_DEBUG_CHANNEL2:    return config.visualize[2];
        case OPT_DMA_DEBUG_CHANNEL3:    return config.visualize[3];
        case OPT_DMA_DEBUG_CHANNEL4:    return config.visualize[4];
        case OPT_DMA_DEBUG_CHANNEL5:    return config.visualize[5];
        case OPT_DMA_DEBUG_CHANNEL6:    return config.visualize[6];
        case OPT_DMA_DEBUG_CHANNEL7:    return config.visualize[7];

        case OPT_DMA_DEBUG_COLOR0:      return config.debugColor[0];
        case OPT_DMA_DEBUG_COLOR1:      return config.debugColor[1];
        case OPT_DMA_DEBUG_COLOR2:      return config.debugColor[2];
        case OPT_DMA_DEBUG_COLOR3:      return config.debugColor[3];
        case OPT_DMA_DEBUG_COLOR4:      return config.debugColor[4];
        case OPT_DMA_DEBUG_COLOR5:      return config.debugColor[5];
        case OPT_DMA_DEBUG_COLOR6:      return config.debugColor[6];
        case OPT_DMA_DEBUG_COLOR7:      return config.debugColor[7];

        default:
            fatalError;
    }
}

void
DmaDebugger::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_DMA_DEBUG_ENABLE:

            return;

        case OPT_DMA_DEBUG_MODE:

            if (!DmaDisplayModeEnum::isValid(value)) {
                throw Error(VAERROR_OPT_INV_ARG, DmaDisplayModeEnum::keyList());
            }
            return;
            
        case OPT_DMA_DEBUG_OPACITY:
        case OPT_DMA_DEBUG_CHANNEL0:
        case OPT_DMA_DEBUG_CHANNEL1:
        case OPT_DMA_DEBUG_CHANNEL2:
        case OPT_DMA_DEBUG_CHANNEL3:
        case OPT_DMA_DEBUG_CHANNEL4:
        case OPT_DMA_DEBUG_CHANNEL5:
        case OPT_DMA_DEBUG_CHANNEL6:
        case OPT_DMA_DEBUG_CHANNEL7:
        case OPT_DMA_DEBUG_COLOR0:
        case OPT_DMA_DEBUG_COLOR1:
        case OPT_DMA_DEBUG_COLOR2:
        case OPT_DMA_DEBUG_COLOR3:
        case OPT_DMA_DEBUG_COLOR4:
        case OPT_DMA_DEBUG_COLOR5:
        case OPT_DMA_DEBUG_COLOR6:
        case OPT_DMA_DEBUG_COLOR7:

            return;

        default:
            throw(VAERROR_OPT_UNSUPPORTED);
    }
}

void
DmaDebugger::setOption(Option option, i64 value)
{
    switch (option) {

        case OPT_DMA_DEBUG_ENABLE:

            config.enabled = value;
            msgQueue.put(MSG_DMA_DEBUG, value);
            return;
            
        case OPT_DMA_DEBUG_MODE:
            
            config.displayMode = (DmaDisplayMode)value;
            return;

        case OPT_DMA_DEBUG_OPACITY:

            config.opacity = (isize)value;
            return;

        case OPT_DMA_DEBUG_CHANNEL0:

            config.visualize[0] = bool(value);
            visualize[BUS_COPPER] = value;
            return;

        case OPT_DMA_DEBUG_CHANNEL1:

            config.visualize[1] = bool(value);
            visualize[BUS_BLITTER] = value;
            return;

        case OPT_DMA_DEBUG_CHANNEL2:

            config.visualize[2] = bool(value);
            visualize[BUS_DISK] = value;
            return;

        case OPT_DMA_DEBUG_CHANNEL3:

            config.visualize[3] = bool(value);
            visualize[BUS_AUD0] = value;
            visualize[BUS_AUD1] = value;
            visualize[BUS_AUD2] = value;
            visualize[BUS_AUD3] = value;
            return;

        case OPT_DMA_DEBUG_CHANNEL4:

            config.visualize[4] = bool(value);
            visualize[BUS_SPRITE0] = value;
            visualize[BUS_SPRITE1] = value;
            visualize[BUS_SPRITE2] = value;
            visualize[BUS_SPRITE3] = value;
            visualize[BUS_SPRITE4] = value;
            visualize[BUS_SPRITE5] = value;
            visualize[BUS_SPRITE6] = value;
            visualize[BUS_SPRITE7] = value;
            return;

        case OPT_DMA_DEBUG_CHANNEL5:

            config.visualize[5] = bool(value);
            visualize[BUS_BPL1] = value;
            visualize[BUS_BPL2] = value;
            visualize[BUS_BPL3] = value;
            visualize[BUS_BPL4] = value;
            visualize[BUS_BPL5] = value;
            visualize[BUS_BPL6] = value;
            return;

        case OPT_DMA_DEBUG_CHANNEL6:

            config.visualize[6] = bool(value);
            visualize[BUS_CPU] = value;
            return;

        case OPT_DMA_DEBUG_CHANNEL7:

            config.visualize[7] = bool(value);
            visualize[BUS_REFRESH] = value;
            return;

        case OPT_DMA_DEBUG_COLOR0:

            config.debugColor[0] = u32(value);
            setColor(BUS_COPPER, (u32)value);
            return;

        case OPT_DMA_DEBUG_COLOR1:

            config.debugColor[1] = u32(value);
            setColor(BUS_BLITTER, (u32)value);
            return;

        case OPT_DMA_DEBUG_COLOR2:

            config.debugColor[2] = u32(value);
            setColor(BUS_DISK, (u32)value);
            return;

        case OPT_DMA_DEBUG_COLOR3:

            config.debugColor[3] = u32(value);
            setColor(BUS_AUD0, (u32)value);
            setColor(BUS_AUD1, (u32)value);
            setColor(BUS_AUD2, (u32)value);
            setColor(BUS_AUD3, (u32)value);
            return;

        case OPT_DMA_DEBUG_COLOR4:

            config.debugColor[4] = u32(value);
            setColor(BUS_SPRITE0, (u32)value);
            setColor(BUS_SPRITE1, (u32)value);
            setColor(BUS_SPRITE2, (u32)value);
            setColor(BUS_SPRITE3, (u32)value);
            setColor(BUS_SPRITE4, (u32)value);
            setColor(BUS_SPRITE5, (u32)value);
            setColor(BUS_SPRITE6, (u32)value);
            setColor(BUS_SPRITE7, (u32)value);
            return;

        case OPT_DMA_DEBUG_COLOR5:

            config.debugColor[5] = u32(value);
            setColor(BUS_BPL1, (u32)value);
            setColor(BUS_BPL2, (u32)value);
            setColor(BUS_BPL3, (u32)value);
            setColor(BUS_BPL4, (u32)value);
            setColor(BUS_BPL5, (u32)value);
            setColor(BUS_BPL6, (u32)value);
            return;

        case OPT_DMA_DEBUG_COLOR6:

            config.debugColor[6] = u32(value);
            setColor(BUS_CPU, (u32)value);
            return;

        case OPT_DMA_DEBUG_COLOR7:

            config.debugColor[7] = u32(value);
            setColor(BUS_REFRESH, (u32)value);
            return;
            
        default:
            fatalError;
    }
}

void
DmaDebugger::cacheInfo(DmaDebuggerInfo &result) const
{
    {   SYNCHRONIZED
        
        result.visualizeCopper = config.visualize[DMA_CHANNEL_COPPER];
        result.visualizeBlitter = config.visualize[DMA_CHANNEL_BLITTER];
        result.visualizeDisk = config.visualize[DMA_CHANNEL_DISK];
        result.visualizeAudio = config.visualize[DMA_CHANNEL_AUDIO];
        result.visualizeSprites = config.visualize[DMA_CHANNEL_SPRITE];
        result.visualizeBitplanes = config.visualize[DMA_CHANNEL_BITPLANE];
        result.visualizeCpu = config.visualize[DMA_CHANNEL_CPU];
        result.visualizeRefresh = config.visualize[DMA_CHANNEL_REFRESH];
        
        getColor(DMA_CHANNEL_COPPER, result.copperColor);
        getColor(DMA_CHANNEL_BLITTER, result.blitterColor);
        getColor(DMA_CHANNEL_DISK, result.diskColor);
        getColor(DMA_CHANNEL_AUDIO, result.audioColor);
        getColor(DMA_CHANNEL_SPRITE, result.spriteColor);
        getColor(DMA_CHANNEL_BITPLANE, result.bitplaneColor);
        getColor(DMA_CHANNEL_CPU, result.cpuColor);
        getColor(DMA_CHANNEL_REFRESH, result.refreshColor);
    }
}

void
DmaDebugger::getColor(DmaChannel channel, double *rgb) const
{
    assert_enum(DmaChannel, channel);
    
    RgbColor color = RgbColor(config.debugColor[channel]);
    rgb[0] = color.r;
    rgb[1] = color.g;
    rgb[2] = color.b;
}

void
DmaDebugger::setColor(BusOwner owner, u32 rgba)
{
    assert_enum(BusOwner, owner);

    // Compute the color variants used for drawing
    RgbColor color = RgbColor(rgba);
    debugColor[owner][0] = color.shade(0.3);
    debugColor[owner][1] = color.shade(0.1);
    debugColor[owner][2] = color.tint(0.1);
    debugColor[owner][3] = color.tint(0.3);
}

void
DmaDebugger::eolHandler()
{
    // Only proceed if DMA debugging has been turned on
    if (!config.enabled) return;

    // Copy Agnus arrays before they get deleted
    std::memcpy(busValue, agnus.busValue, sizeof(agnus.busValue));
    std::memcpy(busOwner, agnus.busOwner, sizeof(agnus.busOwner));

    // Record some information for being picked up in the HSYNC handler
    pixel0 = agnus.pos.pixel(0);
}

void
DmaDebugger::hsyncHandler(isize vpos)
{
    assert(agnus.pos.h == 0x12);

    // Only proceed if DMA debugging has been turned on
    if (!config.enabled) return;

    // Draw first chunk (data from previous DMA line)
    auto *ptr1 = pixelEngine.workingPtr(vpos);
    computeOverlay(ptr1, HBLANK_MIN, HPOS_MAX, busOwner, busValue);

    // Draw second chunk (data from current DMA line)
    auto *ptr2 = ptr1 + agnus.pos.pixel(0);
    computeOverlay(ptr2, 0, HBLANK_MIN - 1, agnus.busOwner, agnus.busValue);
}

void
DmaDebugger::computeOverlay(Texel *ptr, isize first, isize last, BusOwner *own, u16 *val)
{
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

        default:
            fatalError;

    }

    for (isize i = first; i <= last; i++, ptr += 4) {

        BusOwner owner = own[i];
        // u32 *ptr32 = (u32 *)ptr;

        // Handle the easy case first: No foreground pixels
        if (!visualize[owner]) {

            if (bgWeight != 0.0) {

                ptr[0] = TEXEL(GpuColor(ptr[0]).shade(bgWeight).rawValue);
                ptr[1] = TEXEL(GpuColor(ptr[1]).shade(bgWeight).rawValue);
                ptr[2] = TEXEL(GpuColor(ptr[2]).shade(bgWeight).rawValue);
                ptr[3] = TEXEL(GpuColor(ptr[3]).shade(bgWeight).rawValue);
            }
            continue;
        }

        // Get RGBA values of foreground pixels
        GpuColor col0 = debugColor[owner][(val[i] & 0xC000) >> 14];
        GpuColor col1 = debugColor[owner][(val[i] & 0x0C00) >> 10];
        GpuColor col2 = debugColor[owner][(val[i] & 0x00C0) >> 6];
        GpuColor col3 = debugColor[owner][(val[i] & 0x000C) >> 2];

        if (fgWeight != 0.0) {

            col0 = col0.mix(GpuColor(ptr[0]), fgWeight);
            col1 = col1.mix(GpuColor(ptr[2]), fgWeight);
            col2 = col2.mix(GpuColor(ptr[4]), fgWeight);
            col3 = col3.mix(GpuColor(ptr[6]), fgWeight);
        }

        ptr[0] = TEXEL(col0.rawValue);
        ptr[1] = TEXEL(col1.rawValue);
        ptr[2] = TEXEL(col2.rawValue);
        ptr[3] = TEXEL(col3.rawValue);
    }
}

void
DmaDebugger::vSyncHandler()
{
    // Only proceed if the debugger is enabled
    if (!config.enabled) return;

    // Clear old data in the VBLANK area of the next frame
    for (isize row = 0; row < VBLANK_CNT; row++) {

        auto *ptr = denise.pixelEngine.workingPtr(row);
        for (isize col = 0; col < HPIXELS; col++) {

            ptr[col] = FrameBuffer::vblank;
        }
    }
}

void
DmaDebugger::eofHandler()
{

}

}
