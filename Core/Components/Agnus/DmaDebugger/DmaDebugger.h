// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DmaDebuggerTypes.h"
#include "FrameBufferTypes.h"
#include "SubComponent.h"
#include "Beamtraps.h"
#include "utl/color/Colors.h"
#include "Constants.h"
#include "utl/wrappers.h"

namespace vamiga {

class DmaDebugger final : public SubComponent {
    
    Descriptions descriptions = {{
        
        .type           = Class::DmaDebugger,
        .name           = "DmaDebugger",
        .description    = "DMA Debugger",
        .shell          = "dmadebugger"
    }};
    
    Options options = {
        
        Opt::XRAY_MODE,
        Opt::XRAY_OVERLAY,
        Opt::XRAY_OVERLAY_STYLE,
        Opt::XRAY_OVERLAY_OPACITY,
        Opt::XRAY_DMA_CHANNEL0,
        Opt::XRAY_DMA_CHANNEL1,
        Opt::XRAY_DMA_CHANNEL2,
        Opt::XRAY_DMA_CHANNEL3,
        Opt::XRAY_DMA_CHANNEL4,
        Opt::XRAY_DMA_CHANNEL5,
        Opt::XRAY_DMA_CHANNEL6,
        Opt::XRAY_DMA_CHANNEL7,
        Opt::XRAY_COLOR0,
        Opt::XRAY_COLOR1,
        Opt::XRAY_COLOR2,
        Opt::XRAY_COLOR3,
        Opt::XRAY_COLOR4,
        Opt::XRAY_COLOR5,
        Opt::XRAY_COLOR6,
        Opt::XRAY_COLOR7,
        Opt::XRAY_COLOR8,
        Opt::XRAY_COLOR9
    };
    
    // Current configuration
    DmaDebuggerConfig config = {};

public:

    // Result of the latest inspection
    utl::Backed<DmaDebuggerInfo> info;

private:

    // Enable bits
    bool visualize[BUS_COUNT] = {};
    
    // Colors used for highlighting DMA (derived from config.debugColor)
    RgbColor debugColor[BUS_COUNT][5] = {};
    
    // Local copies of the recorded bus data (recorded in the EOL handler)
    BusOwner busOwner[HPOS_CNT];
    u16 busAddr[HPOS_CNT];
    u16 busData[HPOS_CNT];
    
    // HSYNC handler information (recorded in the EOL handler)
    isize pixel0 = 0;
    
public:
    
    // Beamtraps
    Beamtraps beamtraps = Beamtraps(agnus);
    
    bool eolTrap = false;
    bool eofTrap = false;
    
    
    //
    // Initializing
    //
    
public:
    
    DmaDebugger(Amiga &ref);
    
    DmaDebugger& operator= (const DmaDebugger& other) {
        
        CLONE_ARRAY(visualize)
        CLONE_ARRAY(busData)
        CLONE_ARRAY(busOwner)
        CLONE(pixel0)
        CLONE(config)
        
        std::memcpy(debugColor, other.debugColor, sizeof(debugColor));
        
        return *this;
    }
    
    
    //
    // Methods from Serializable
    //
    
private:
    
    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);
 
    
    //
    // Methods from CoreComponent
    //
    
public:
    
    const Descriptions &getDescriptions() const override { return descriptions; }
    
private:
    
    void _dump(Category category, std::ostream &os) const override;
    
    
    //
    //  Analyzing
    //
    
public:
    
    DmaDebuggerInfo cacheInfo() const;

    
    //
    // Methods from Configurable
    //
    
public:
    
    const DmaDebuggerConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;
    
private:
    
    void getColor(DmaChannel channel, double *rgb) const;
    void setColor(BusOwner owner, u32 rgba);
    
    
    //
    // Running the debugger
    //
    
public:
    
    // Called by Agnus at the end of each scanline
    void eolHandler();
    
    // Called by Agnus at the beginning of the HSYNC area
    void hsyncHandler(isize vpos);
    
    // Cleans by Agnus at the end of each frame
    void vSyncHandler();
    
    // Called at the end of each frame
    void eofHandler();
    
private:
    
    /* Visualizes DMA usage for a certain range of DMA cycles. Paints only
     * the raw per-channel colors into 'dmaPtr' (the xray texture, see
     * PixelEngine::xrayTexture) -- never touches the emulator texture.
     * Opacity/display-mode blending no longer happens here: PixelEngine::
     * mergeXray applies that once for the whole frame, after both this and
     * XRayMode::XRAY_LAYERS (PixelEngine::hide) have finished building
     * their xray texture for every line.
     *
     * Dispatches once (per call, not per pixel) to the templated overload
     * below, matching the host's current HOST_TEX_FORMAT. Fixing the format
     * as a template parameter lets PixelEngine::toTexel<F> fold its format
     * switch away at compile time, so the per-pixel loop -- run across
     * every visible pixel of every scanline -- carries no runtime format
     * branching at all.
     */
    void computeOverlay(Texel *dmaPtr, isize first, isize last, BusOwner *own, u16 *val);

    template <TexelFormat F>
    void computeOverlay(Texel *dmaPtr, isize first, isize last, BusOwner *own, u16 *val);
};

}
