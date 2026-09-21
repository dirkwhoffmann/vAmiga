// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "DmaDebugger.h"
#include "Amiga.h"
#include "utl/io.h"

namespace vamiga {

DmaDebugger::DmaDebugger(Amiga &ref) : SubComponent(ref)
{
    info.bind([this] { return cacheInfo(); } );
}

void
DmaDebugger::_dump(Category category, std::ostream &os) const
{
    auto print = [&]() {

        using namespace utl;

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
DmaDebugger::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::XRAY_MODE:        return (i64)config.mode;
        case Opt::XRAY_OVERLAY:     return config.overlay;
        case Opt::XRAY_OVERLAY_STYLE: return (i64)config.displayMode;
        case Opt::XRAY_OVERLAY_OPACITY: return config.opacity;

        case Opt::XRAY_DMA_CHANNEL0:    return config.visualize[0];
        case Opt::XRAY_DMA_CHANNEL1:    return config.visualize[1];
        case Opt::XRAY_DMA_CHANNEL2:    return config.visualize[2];
        case Opt::XRAY_DMA_CHANNEL3:    return config.visualize[3];
        case Opt::XRAY_DMA_CHANNEL4:    return config.visualize[4];
        case Opt::XRAY_DMA_CHANNEL5:    return config.visualize[5];
        case Opt::XRAY_DMA_CHANNEL6:    return config.visualize[6];
        case Opt::XRAY_DMA_CHANNEL7:    return config.visualize[7];

        case Opt::XRAY_COLOR0:      return config.debugColor[0];
        case Opt::XRAY_COLOR1:      return config.debugColor[1];
        case Opt::XRAY_COLOR2:      return config.debugColor[2];
        case Opt::XRAY_COLOR3:      return config.debugColor[3];
        case Opt::XRAY_COLOR4:      return config.debugColor[4];
        case Opt::XRAY_COLOR5:      return config.debugColor[5];
        case Opt::XRAY_COLOR6:      return config.debugColor[6];
        case Opt::XRAY_COLOR7:      return config.debugColor[7];
        case Opt::XRAY_COLOR8:      return config.debugColor[8];
        case Opt::XRAY_COLOR9:      return config.debugColor[9];

        default:
            fatalError;
    }
}

void
DmaDebugger::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::XRAY_MODE:
            if (!XRayModeEnum::isValid(value)) {
                throw CoreError(CoreError::OPT_INV_ARG, XRayModeEnum::keyList());
            }
            return;

        case Opt::XRAY_OVERLAY:

            return;

        case Opt::XRAY_OVERLAY_STYLE:
            if (!DmaDisplayModeEnum::isValid(value)) {
                throw CoreError(CoreError::OPT_INV_ARG, DmaDisplayModeEnum::keyList());
            }
            return;
            
        case Opt::XRAY_OVERLAY_OPACITY:         case Opt::XRAY_DMA_CHANNEL0:
        case Opt::XRAY_DMA_CHANNEL1:
        case Opt::XRAY_DMA_CHANNEL2:
        case Opt::XRAY_DMA_CHANNEL3:
        case Opt::XRAY_DMA_CHANNEL4:
        case Opt::XRAY_DMA_CHANNEL5:
        case Opt::XRAY_DMA_CHANNEL6:
        case Opt::XRAY_DMA_CHANNEL7:
        case Opt::XRAY_COLOR0:
        case Opt::XRAY_COLOR1:
        case Opt::XRAY_COLOR2:
        case Opt::XRAY_COLOR3:
        case Opt::XRAY_COLOR4:
        case Opt::XRAY_COLOR5:
        case Opt::XRAY_COLOR6:
        case Opt::XRAY_COLOR7:
        case Opt::XRAY_COLOR8:
        case Opt::XRAY_COLOR9:

            return;

        default:
            throw CoreError(CoreError::OPT_UNSUPPORTED);
    }
}

void
DmaDebugger::setOption(Opt option, i64 value)
{
    switch (option) {

        case Opt::XRAY_MODE:

            config.mode = (XRayMode)value;

            // Avoid showing stale visualization from the previous mode
            // (or real picture data in the xray texture) until the new
            // mode has painted its own pixels.
            pixelEngine.clearAll();

            msgQueue.put(Msg::DMA_DEBUG, value);
            return;

        case Opt::XRAY_OVERLAY:

            config.overlay = value;
            return;

        case Opt::XRAY_OVERLAY_STYLE:             
            config.displayMode = (DmaDisplayMode)value;
            return;

        case Opt::XRAY_OVERLAY_OPACITY: 
            config.opacity = (isize)value;
            return;

        case Opt::XRAY_DMA_CHANNEL0:

            config.visualize[0] = bool(value);
            visualize[BUS_COPPER] = value;
            return;

        case Opt::XRAY_DMA_CHANNEL1:

            config.visualize[1] = bool(value);
            visualize[BUS_BLITTER] = value;
            return;

        case Opt::XRAY_DMA_CHANNEL2:

            config.visualize[2] = bool(value);
            visualize[BUS_DISK] = value;
            return;

        case Opt::XRAY_DMA_CHANNEL3:

            config.visualize[3] = bool(value);
            visualize[BUS_AUD0] = value;
            visualize[BUS_AUD1] = value;
            visualize[BUS_AUD2] = value;
            visualize[BUS_AUD3] = value;
            return;

        case Opt::XRAY_DMA_CHANNEL4:

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

        case Opt::XRAY_DMA_CHANNEL5:

            config.visualize[5] = bool(value);
            visualize[BUS_BPL1] = value;
            visualize[BUS_BPL2] = value;
            visualize[BUS_BPL3] = value;
            visualize[BUS_BPL4] = value;
            visualize[BUS_BPL5] = value;
            visualize[BUS_BPL6] = value;
            return;

        case Opt::XRAY_DMA_CHANNEL6:

            config.visualize[6] = bool(value);
            visualize[BUS_CPU] = value;
            return;

        case Opt::XRAY_DMA_CHANNEL7:

            config.visualize[7] = bool(value);
            visualize[BUS_REFRESH] = value;
            return;

        case Opt::XRAY_COLOR0:

            config.debugColor[0] = u32(value);
            setColor(BusOwner::COPPER, (u32)value);
            return;

        case Opt::XRAY_COLOR1:

            config.debugColor[1] = u32(value);
            setColor(BusOwner::BLITTER, (u32)value);
            return;

        case Opt::XRAY_COLOR2:

            config.debugColor[2] = u32(value);
            setColor(BusOwner::DISK, (u32)value);
            return;

        case Opt::XRAY_COLOR3:

            config.debugColor[3] = u32(value);
            setColor(BusOwner::AUD0, (u32)value);
            setColor(BusOwner::AUD1, (u32)value);
            setColor(BusOwner::AUD2, (u32)value);
            setColor(BusOwner::AUD3, (u32)value);
            return;

        case Opt::XRAY_COLOR4:

            config.debugColor[4] = u32(value);
            setColor(BusOwner::SPRITE0, (u32)value);
            setColor(BusOwner::SPRITE1, (u32)value);
            setColor(BusOwner::SPRITE2, (u32)value);
            setColor(BusOwner::SPRITE3, (u32)value);
            setColor(BusOwner::SPRITE4, (u32)value);
            setColor(BusOwner::SPRITE5, (u32)value);
            setColor(BusOwner::SPRITE6, (u32)value);
            setColor(BusOwner::SPRITE7, (u32)value);
            return;

        case Opt::XRAY_COLOR5:

            config.debugColor[5] = u32(value);
            setColor(BusOwner::BPL1, (u32)value);
            setColor(BusOwner::BPL2, (u32)value);
            setColor(BusOwner::BPL3, (u32)value);
            setColor(BusOwner::BPL4, (u32)value);
            setColor(BusOwner::BPL5, (u32)value);
            setColor(BusOwner::BPL6, (u32)value);
            return;

        case Opt::XRAY_COLOR6:

            config.debugColor[6] = u32(value);
            setColor(BusOwner::CPU, (u32)value);
            return;

        case Opt::XRAY_COLOR7:

            config.debugColor[7] = u32(value);
            setColor(BusOwner::REFRESH, (u32)value);
            return;

        case Opt::XRAY_COLOR8:

            // No BusOwner backs this slot -- it's XRayMode::XRAY_LAYERS'
            // playfield 1 color (see PixelEngine::hide), not a DMA channel.
            config.debugColor[8] = u32(value);
            return;

        case Opt::XRAY_COLOR9:

            // Playfield 2 color (see PixelEngine::hide).
            config.debugColor[9] = u32(value);
            return;

        default:
            fatalError;
    }
}

DmaDebuggerInfo
DmaDebugger::cacheInfo() const
{
    DmaDebuggerInfo info;

    info.visualizeCopper = config.visualize[isize(DmaChannel::COPPER)];
    info.visualizeBlitter = config.visualize[isize(DmaChannel::BLITTER)];
    info.visualizeDisk = config.visualize[isize(DmaChannel::DISK)];
    info.visualizeAudio = config.visualize[isize(DmaChannel::AUDIO)];
    info.visualizeSprites = config.visualize[isize(DmaChannel::SPRITE)];
    info.visualizeBitplanes = config.visualize[isize(DmaChannel::BITPLANE)];
    info.visualizeCpu = config.visualize[isize(DmaChannel::CPU)];
    info.visualizeRefresh = config.visualize[isize(DmaChannel::REFRESH)];

    getColor(DmaChannel::COPPER, info.copperColor);
    getColor(DmaChannel::BLITTER, info.blitterColor);
    getColor(DmaChannel::DISK, info.diskColor);
    getColor(DmaChannel::AUDIO, info.audioColor);
    getColor(DmaChannel::SPRITE, info.spriteColor);
    getColor(DmaChannel::BITPLANE, info.bitplaneColor);
    getColor(DmaChannel::CPU, info.cpuColor);
    getColor(DmaChannel::REFRESH, info.refreshColor);

    return info;
}

void
DmaDebugger::getColor(DmaChannel channel, double *rgb) const
{
    DmaChannelEnum::validate(channel);

    RgbColor color = RgbColor(config.debugColor[isize(channel)]);
    rgb[0] = color.r;
    rgb[1] = color.g;
    rgb[2] = color.b;
}

void
DmaDebugger::setColor(BusOwner owner, u32 rgba)
{
    BusOwnerEnum::key(owner);

    // Compute the color variants used for drawing
    RgbColor color = RgbColor(rgba);
    debugColor[isize(owner)][0] = color.shade(0.3);
    debugColor[isize(owner)][1] = color.shade(0.1);
    debugColor[isize(owner)][2] = color.tint(0.1);
    debugColor[isize(owner)][3] = color.tint(0.3);
}

void
DmaDebugger::eolHandler()
{
    // Check if execution should be interrupted
    if (eolTrap) { eolTrap = false; amiga.setFlag(RL::EOL_REACHED); }
    
    if (config.mode == XRayMode::XRAY_DMA) {
        
        // Copy Agnus arrays before they get deleted
        std::memcpy(busOwner, agnus.busOwner, sizeof(agnus.busOwner));
        std::memcpy(busAddr, agnus.busAddr, sizeof(agnus.busAddr));
        std::memcpy(busData, agnus.busData, sizeof(agnus.busData));
        
        // Record some information for being picked up in the HSYNC handler
        pixel0 = agnus.pos.pixel(0);
    }
}

void
DmaDebugger::hsyncHandler(isize vpos)
{
    assert(agnus.pos.h == 0x12);

    if (config.mode == XRayMode::XRAY_DMA) {

        // Draw first chunk (data from previous DMA line)
        auto *dma1 = pixelEngine.xrayWorkingPtr(vpos);
        computeOverlay(dma1, HBLANK_MIN, HPOS_MAX, busOwner, busData);

        // Draw second chunk (data from current DMA line)
        // pos.pixel() counts super-hires pixels, dma1 counts Texels (hires)
        auto *dma2 = dma1 + agnus.pos.pixel(0) / 2;
        computeOverlay(dma2, 0, HBLANK_MIN - 1, agnus.busOwner, agnus.busData);
    }
}

void
DmaDebugger::computeOverlay(Texel *dmaPtr, isize first, isize last, BusOwner *own, u16 *val)
{
    // Dispatched once per call (not per pixel) -- see the class comment in
    // DmaDebugger.h. Each branch below calls a separate instantiation of
    // the templated overload, so the format is a compile-time constant for
    // the whole per-pixel loop inside it.
    switch (static_cast<TexelFormat>(host.getConfig().texFormat)) {

        case TexelFormat::ABGR: computeOverlay<TexelFormat::ABGR>(dmaPtr, first, last, own, val); return;
        case TexelFormat::ARGB: computeOverlay<TexelFormat::ARGB>(dmaPtr, first, last, own, val); return;

        default: // RGBA
            computeOverlay<TexelFormat::RGBA>(dmaPtr, first, last, own, val); return;
    }
}

template <TexelFormat F>
void
DmaDebugger::computeOverlay(Texel *dmaPtr, isize first, isize last, BusOwner *own, u16 *val)
{
    for (isize i = first; i <= last; i++, dmaPtr += 4) {

        auto owner = isize(own[i]);

        // Handle the easy case first: No foreground pixels
        if (!visualize[owner]) {

            // Nothing to show here -- see PixelEngine::mergeXray for how
            // (and whether) that ends up dimming the real picture.
            dmaPtr[0] = dmaPtr[1] = dmaPtr[2] = dmaPtr[3] = Texture::black;
            continue;
        }

        // Get RGBA values of foreground pixels
        GpuColor<F> col0 = debugColor[owner][(val[i] & 0xC000) >> 14];
        GpuColor<F> col1 = debugColor[owner][(val[i] & 0x0C00) >> 10];
        GpuColor<F> col2 = debugColor[owner][(val[i] & 0x00C0) >> 6];
        GpuColor<F> col3 = debugColor[owner][(val[i] & 0x000C) >> 2];

        // Paint the raw channel color into the xray texture alone -- see
        // PixelEngine::mergeXray for how (and whether) it ends up blended
        // into the real picture.
        dmaPtr[0] = PixelEngine::toTexel<F>(col0);
        dmaPtr[1] = PixelEngine::toTexel<F>(col1);
        dmaPtr[2] = PixelEngine::toTexel<F>(col2);
        dmaPtr[3] = PixelEngine::toTexel<F>(col3);
    }
}

void
DmaDebugger::vSyncHandler()
{
    // Only proceed if the debugger is enabled
    if (config.mode != XRayMode::XRAY_DMA) return;

    // Clear old data in the VBLANK area of the next frame
    auto cnt = agnus.isPAL() ? PAL::VBLANK_CNT : NTSC::VBLANK_CNT;
    for (isize row = 0; row < cnt; row++) {

        auto *ptr = denise.pixelEngine.workingPtr(row);
        auto *dma = denise.pixelEngine.xrayWorkingPtr(row);

        for (isize col = 0; col < HPIXELS; col++) {

            ptr[col] = Texture::vblank;
            dma[col] = Texture::black;
        }
    }
}

void
DmaDebugger::eofHandler()
{
    // Check if execution should be interrupted
    if (eofTrap) { eofTrap = false; amiga.setFlag(RL::EOF_REACHED); }
}

}
