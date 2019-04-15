// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CONTROL_PORT_INC
#define _CONTROL_PORT_INC

#include "HardwareComponent.h"

class ControlPort : public HardwareComponent {

private:
    
    // Represented control port (1 or 2)
    int nr;
    
    // true, if a mouse is connected to this port
    bool hasMouse = false;
    
    // True, if button is pressed.
    bool button = false;
    
    /* Horizontal joystick position
     * Valid valued are -1 (LEFT), 1 (RIGHT), or 0 (RELEASED)
     */
    int axisX = 0;
    
    /* Vertical joystick position
     * Valid valued are -1 (UP), 1 (DOWN), or 0 (RELEASED)
     */
    int axisY = 0;
    
    // True if multi-shot mode in enabled
    bool autofire = false;
    
    // Number of bullets per gun volley
    int autofireBullets = -3;
    
    // Autofire frequency in Hz
    float autofireFrequency = 2.5;
    
    // Bullet counter used in multi-fire mode
    uint64_t bulletCounter = 0;
    
    // Next frame to auto-press or auto-release the fire button
    uint64_t nextAutofireFrame = 0;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    ControlPort(int p);
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _dump() override;
    void didLoadFromBuffer(uint8_t **buffer) override;
    
    
    //
    // Joystick
    //
    
public:
    
    // Triggers a joystick event
    void trigger(JoystickEvent event);
    
    // Returns true if auto-fire mode is enabled.
    bool getAutofire() { return autofire; }
    
    // Enables or disables autofire.
    void setAutofire(bool value);
    
    /* Returns the number of bullets per gun volley.
     * A negative value represents infinity.
     */
    int getAutofireBullets() { return autofireBullets; }
    
    /* Sets the number of bullets per gun volley.
     * A negative value represents infinity.
     */
    void setAutofireBullets(int value);
    
    // Returns the autofire frequency.
    float getAutofireFrequency() { return autofireFrequency; }
    
    // Sets the autofire frequency.
    void setAutofireFrequency(float value) { autofireFrequency = value; }
    
    // Updates variable nextAutofireFrame
    void scheduleNextShot();
    
    /* Execution function for this control port
     * This method is invoked at the end of each frame. It is needed
     * needed to implement the autofire functionality, only.
     */
    void execute();
    
    
    //
    // Mouse
    //
 
public:
    
    // Returns true if a mouse is connected
    bool mouseIsConnected() { return hasMouse; }
    
    // Connects or disconnects a mouse
    void connectMouse(bool value) { hasMouse = value; }
    
    // Returns the potentiometer X value
    // uint8_t potX();
    
    // Returns the potentiometer Y value
    // uint8_t potY();

    // Moves the mouse to a new location
    // void setXY(float x, float y);
    
    // Presses a mouse button
    // void setLeftMouseButton(bool pressed);
    // void setRightMouseButton(bool pressed);
};

#endif

