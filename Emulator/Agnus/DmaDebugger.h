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
#include "Bus.h"
#include "AmigaComponent.h"
#include "Colors.h"

class DmaDebugger : public AmigaComponent {

    // Current configuration
    DmaDebuggerConfig config;

    // Debug colors, derived from the debug color palette
    RgbColor debugColor[BUS_COUNT][5];


    //
    // Initializing
    //

public:

    DmaDebugger(Amiga &ref);

    const char *getDescription() const override { return "DmaDebugger"; }

    void _reset(bool hard) override { }


    //
    // Configuring
    //

public:
    
    const DmaDebuggerConfig &getConfig() const { return config; }

    i64 getConfigItem(Option option) const;
    i64 getConfigItem(Option option, long id) const;
    bool setConfigItem(Option option, i64 value) override;
    bool setConfigItem(Option option, long id, i64 value) override;

private:

    void getColor(BusOwner owner, double *rgb);
    void setColor(BusOwner owner, u32 rgba);
    /*
    void setColor(BusOwner owner, RgbColor color);
    void setColor(BusOwner owner, double r, double g, double b);
    */

    
    //
    // Analyzing
    //
    
public:

    // Returns the result of the most recent call to inspect()
    DMADebuggerInfo getInfo();

    
    //
    // Serializing
    //

private:

    isize _size() override { return 0; }
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
