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
#include "Colors.h"
#include "Constants.h"

namespace vamiga {

class DmaDebugger final : public SubComponent, public Inspectable<DmaDebuggerInfo> {
    
    Descriptions descriptions = {{
        
        .type           = Class::DmaDebugger,
        .name           = "DmaDebugger",
        .description    = "DMA Debugger",
        .shell          = "dmadebugger"
    }};
    
    Options options = {
        
        Opt::DMA_DEBUG_ENABLE,
        Opt::DMA_DEBUG_MODE,
        Opt::DMA_DEBUG_OPACITY,
        Opt::DMA_DEBUG_CHANNEL0,
        Opt::DMA_DEBUG_CHANNEL1,
        Opt::DMA_DEBUG_CHANNEL2,
        Opt::DMA_DEBUG_CHANNEL3,
        Opt::DMA_DEBUG_CHANNEL4,
        Opt::DMA_DEBUG_CHANNEL5,
        Opt::DMA_DEBUG_CHANNEL6,
        Opt::DMA_DEBUG_CHANNEL7,
        Opt::DMA_DEBUG_COLOR0,
        Opt::DMA_DEBUG_COLOR1,
        Opt::DMA_DEBUG_COLOR2,
        Opt::DMA_DEBUG_COLOR3,
        Opt::DMA_DEBUG_COLOR4,
        Opt::DMA_DEBUG_COLOR5,
        Opt::DMA_DEBUG_COLOR6,
        Opt::DMA_DEBUG_COLOR7
    };
    
    // Current configuration
    DmaDebuggerConfig config = {};
    
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
    // Methods from Inspectable
    //
    
public:
    
    void cacheInfo(DmaDebuggerInfo &result) const override;
    
    
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
    
    // Visualizes DMA usage for a certain range of DMA cycles
    void computeOverlay(Texel *ptr, isize first, isize last, BusOwner *own, u16 *val);
};

}
