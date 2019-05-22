// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _JOYSTICK_INC
#define _JOYSTICK_INC

#include "HardwareComponent.h"

class Joystick : public HardwareComponent {

    // Quick-access references
    class Agnus *agnus;

    // The control port this joystick is connected to (1 or 2)
    int nr;
    
    // Button state
    bool button = false;
    
    // Horizontal joystick position (-1 = left, 1 = right, 0 = released)
    int axisX = 0;
    
    // Vertical joystick position (-1 = up, 1 = down, 0 = released)
    int axisY = 0;
    
    // Indicates whether multi-shot mode is enabled
    bool autofire = false;
    
    // Number of bullets per gun volley
    int autofireBullets = -3;
    
    // Autofire frequency in Hz
    float autofireFrequency = 2.5;
    
    // Bullet counter used in multi-fire mode
    int64_t bulletCounter = 0;
    
    // Next frame to auto-press or auto-release the fire button
    Frame nextAutofireFrame = 0;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Joystick(int nr);
    
    
    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _dump() override;
    void didLoadFromBuffer(uint8_t **buffer) override;
    
    
    //
    // Accessing properties
    //
    
public:
    
    // Auto-fire
    bool getAutofire() { return autofire; }
    void setAutofire(bool value);
    
    // Number of bullets per gun volley (a negative value means infinite)
    int getAutofireBullets() { return autofireBullets; }
    void setAutofireBullets(int value);
    
    // Autofire frequency
    float getAutofireFrequency() { return autofireFrequency; }
    void setAutofireFrequency(float value) { autofireFrequency = value; }

private:
    
    // Updates variable nextAutofireFrame
    void scheduleNextShot();

    
    //
    // Using the device
    //
    
public:

    // Callback handler for function ControlPort::joydat()
    uint16_t joydat();

    // Callback handler for function ControlPort::ciapa()
    uint8_t ciapa();
    
    // Triggers a joystick event
    void trigger(JoystickEvent event);

    /* Execution function for this control port
     * This method needs to be invoked at the end of each frame to make the
     * auto-fire mechanism work.
     */
    void execute();
};

#endif
