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


    //
    // Constructing and destructing
    //

public:

    DmaDebugger();


    //
    // Methods from HardwareComponent
    //

private:

    /*
    void _powerOn() override;


    //
    // Configuring the device
    //

public:

    // Turns DMA debugging on or off
    bool isEnabled();
    void setEnabled(bool value);

    // Enables or disables the visual effects for a certain DMA source
    bool isVisualized(DmaSource source);
    void setVisualized(DmaSource source, bool value);

    // Gets or sets a debug color
    uint32_t getColor(DmaSource source);
    void setColor(DmaSource source, uint32_t color);

    // Gets or sets the opacity of the superimposed visual effect
    float getOpacity();
    void setOpacity(float value);


    //
    // Managing the color cache
    //

    // Superimposes the debug output onto the current rasterline
    void overlayDmaUsage(int *buffer);
    */
};

#endif

