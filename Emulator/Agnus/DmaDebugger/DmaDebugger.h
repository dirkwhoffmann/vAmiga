// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DmaDebuggerTypes.h"
#include "SubComponent.h"
#include "Colors.h"

class DmaDebugger : public SubComponent {

    // Current configuration
    DmaDebuggerConfig config = {};

    // Enable bits
    bool visualize[BUS_COUNT] = {};
    
    // Colors used for highlighting DMA (derived from config.debugColor)
    RgbColor debugColor[BUS_COUNT][5] = {};


    //
    // Initializing
    //

public:

    DmaDebugger(Amiga &ref);

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "DmaDebugger"; }
    void _dump(Category category, std::ostream& os) const override { }

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { }


    //
    // Configuring
    //

public:
    
    const DmaDebuggerConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    i64 getConfigItem(Option option, long id) const;
    void setConfigItem(Option option, i64 value);
    void setConfigItem(Option option, long id, i64 value);

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
    
    // Superimposes the debug output onto the current rasterline
    void computeOverlay();

    // Cleans up some texture data at the end of each frame
    void vSyncHandler();
};
