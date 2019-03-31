// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _KEYBOARD_INC
#define _KEYBOARD_INC

#include "HardwareComponent.h"

typedef enum
{
    KB_SEND_SYNC = 0,
    KB_POWER_UP_KEY_STREAM,
    KB_TERMINATE_KEY_STREAM,
    KB_NORMAL_OPERATION
}
KeyboardState;

class Keyboard : public HardwareComponent {
    
private:

    // Current state of the keyboard
    KeyboardState state;
    
    // Acknowledge signal sent from the Amiga side
    bool handshake;
        
    // Indicates if a key is currently held down (array index = raw key code).
    bool keyDown[128];
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Keyboard();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
     void _powerOn() override;
     /*
     void _powerOff() override;
     void _reset() override;
     void _ping() override;
     */
     void _dump() override;
    
    //
    // FAKE METHODS FOR THE VISUAL PROTOTYPE (TEMPORARY)
    //
    
public:
    
    /* Sends a keycode to the Amiga
     */
    void sendKeyCode(uint8_t keyCode); 
    
    /* Receives a handshake from the Amiga
     * This function is called whenever the CIA puts the serial register into
     * output mode.
     */
    void emulateHandshake() { handshake = true; }
    
    /* The keyboard execution function
     * This function is called periodically by the hsync handler with a period
     * of approx. 1 msec.
     */
    void execute();
    
    
    bool keyIsPressed(long keycode);
    void pressKey(long keycode);
    void releaseKey(long keycode);
    void releaseAllKeys();
};

#endif
