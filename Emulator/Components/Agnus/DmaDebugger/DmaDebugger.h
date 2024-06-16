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
#include "Colors.h"
#include "Constants.h"

namespace vamiga {

class DmaDebugger : public SubComponent {

    Descriptions descriptions = {{

        .name           = "dmadebugger",
        .description    = "DMA Debugger"
    }};

    ConfigOptions options = {

        OPT_DMA_DEBUG_ENABLE,
        OPT_DMA_DEBUG_MODE,
        OPT_DMA_DEBUG_OPACITY,
        OPT_DMA_DEBUG_CHANNEL0,
        OPT_DMA_DEBUG_CHANNEL1,
        OPT_DMA_DEBUG_CHANNEL2,
        OPT_DMA_DEBUG_CHANNEL3,
        OPT_DMA_DEBUG_CHANNEL4,
        OPT_DMA_DEBUG_CHANNEL5,
        OPT_DMA_DEBUG_CHANNEL6,
        OPT_DMA_DEBUG_CHANNEL7,
        OPT_DMA_DEBUG_COLOR0,
        OPT_DMA_DEBUG_COLOR1,
        OPT_DMA_DEBUG_COLOR2,
        OPT_DMA_DEBUG_COLOR3,
        OPT_DMA_DEBUG_COLOR4,
        OPT_DMA_DEBUG_COLOR5,
        OPT_DMA_DEBUG_COLOR6,
        OPT_DMA_DEBUG_COLOR7
    };

    // Current configuration
    DmaDebuggerConfig config = {};

    // Enable bits
    bool visualize[BUS_COUNT] = {};
    
    // Colors used for highlighting DMA (derived from config.debugColor)
    RgbColor debugColor[BUS_COUNT][5] = {};

    // A local copy of the bus value table (recorded in the EOL handler)
    u16 busValue[HPOS_CNT];

    // A local copy of the bus owner table (recorded in the EOL handler)
    BusOwner busOwner[HPOS_CNT];

    // HSYNC handler information (recorded in the EOL handler)
    isize pixel0 = 0;


    //
    // Initializing
    //

public:

    DmaDebugger(Amiga &ref);

    
    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream& os) const override { }

    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _reset(bool hard) override { }

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:
    
    const DmaDebuggerConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void setOption(Option option, i64 value) override;

private:

    void getColor(DmaChannel channel, double *rgb);
    void setColor(BusOwner owner, u32 rgba);

    
    //
    // Analyzing
    //
    
public:

    // Returns the result of the most recent call to inspect()
    DmaDebuggerInfo getInfo();

    
    //
    // Serializing
    //

private:

    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }
    

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
