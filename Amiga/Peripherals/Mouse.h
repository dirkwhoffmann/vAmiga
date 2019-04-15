// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _MOUSE_INC
#define _MOUSE_INC

#include "HardwareComponent.h"

class Mouse : public HardwareComponent {
    
    // The port the mouse is connected to (0 = unconnected)
    uint8_t port = 0;
    
    // Mouse button states
    bool leftButton;
    bool rightButton;
    
    // The current mouse position
    int64_t mouseX;
    int64_t mouseY;
    
    /* The target mouse position
     * In order to achieve a smooth mouse movement, a new mouse coordinate is
     * not written directly into mouseX and mouseY. Instead, these variables
     * are set. In execute(), mouseX and mouseY are shifted smoothly towards
     * the target positions.
     */
    int64_t targetX;
    int64_t targetY;
    
    // Dividers applied to raw coordinates in setXY()
    int dividerX = 256;
    int dividerY = 256;
    
    // Mouse movement in pixels per execution step
    int64_t shiftX = 31;
    int64_t shiftY = 31;
    
    
    //
    // Constructing and destructing
    //
    
    public:
    
    Mouse();
    
    
    //
    // Methods from HardwareComponent
    //
    
    private:
    
    void _powerOn() override;
    void _dump() override;
    
    public:
    
    
    //
    // Operating the mouse
    //
    
    // Connects the mouse to the specified port.
    void connectToPort(unsigned nr) { assert(nr <= 2); port = nr; }
    
    // Disconnects the mouse.
    void disconnect() { port = 0; }
    
    // Emulates a mouse movement event.
    void setXY(int64_t x, int64_t y);
    
    // Emulates a mouse button event.
    void setLeftButton(bool value) { leftButton = value; }
    void setRightButton(bool value) { rightButton = value; }
    
    // Returns the mouse position as it appears in the JOYxDAT register.
    uint8_t joydat() { return HI_LO(mouseY & 0xFF, mouseX & 0xFF); }
    
    // Performs periodic actions for this device.
    void execute();
};

#endif

