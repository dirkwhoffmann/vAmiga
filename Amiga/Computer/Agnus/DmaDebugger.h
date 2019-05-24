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
#include "Colors.h"

class DmaDebugger : public HardwareComponent {

private:

    // Indicates if DMA debugging is turned on or off
    bool enabled = true;

    // Indicates if a certain DMA channel should be visualized
    bool visualize[BUS_OWNER_COUNT]; 

    // DMA debugging colors
    RgbColor debugColor[BUS_OWNER_COUNT][4];

    // Selected color index for all DMA channels
    uint8_t colorIndex[BUS_OWNER_COUNT];

    // Opacity of debug overlay
    double opacity = 0.5;


    //
    // Constructing and destructing
    //

public:

    DmaDebugger();

    // Returns the current settings
    DMADebuggerInfo getInfo();


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
    RgbColor getColor(BusOwner owner);
    void switchColor(BusOwner owner);
    void setColor(BusOwner owner, uint8_t nr);
    void setColor(BusOwner owner, RgbColor color);
    void setColor(BusOwner owner, double r, double g, double b);

    // Gets or sets the opacity of the superimposed visual effect
    double getOpacity();
    void setOpacity(double value);


    //
    // Running the debugger
    //

    // Superimposes the debug output onto the current rasterline
    void computeOverlay();
};

#endif

