// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DMA_DEBUGGER_INC
#define _DMA_DEBUGGER_INC

#include "HardwareComponent.h"

class DmaDebugger : public HardwareComponent {

private:

    // Indicates if DMA debugging is turned on or off
    bool enabled = true;

    // Color storage
    RgbColor debugColor[BUS_OWNER_COUNT][4];


    //
    // Constructing and destructing
    //

public:

    DmaDebugger();


    //
    // Methods from HardwareComponent
    //

    /*
private:

    void _powerOn() override;
     */

    //
    // Configuring the device
    //

public:

    // Turns DMA debugging on or off
    bool isEnabled() { return enabled; }
    void setEnabled(bool value) { enabled = value; }

    // Enables or disables the visual effects for a certain DMA source
    bool isVisualized(BusOwner owner);
    void setVisualized(BusOwner owner, bool value);

    // Gets or sets a debug color
    uint32_t getColor(BusOwner owner);
    void setColor(BusOwner owner, RgbColor color);

    // Gets or sets the opacity of the superimposed visual effect
    float getOpacity();
    void setOpacity(float value);


    //
    // Running the debugger
    //

    // Superimposes the debug output onto the current rasterline
    void computeOverlay();
};

#endif

