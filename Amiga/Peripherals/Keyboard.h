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

class Keyboard : public HardwareComponent {
    
    typedef enum
    {
        KB_SEND_SYNC,
        KB_POWER_UP_KEY_STREAM,
        KB_TERMINATE_KEY_STREAM,
        KB_NORMAL_OPERATION
    }
    KeyboardState;
    
    public:
    
    // The keybord layout identifier
    long layout = 0;
    
    private:

    // The current state of the keyboard
    KeyboardState state;
    
    // The acknowledge signal sent from the Amiga side
    bool handshake;
    
    // Size of the keycode type-ahead buffer
    static const size_t bufferSize = 10;
    
    // The keycode type-ahead buffer
    uint8_t typeAheadBuffer[bufferSize];
    
    // Next free position in the type ahead buffer
    uint8_t bufferIndex;
    
    // Indicates if a key is currently held down (array index = raw key code).
    // DEPRECATED
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
     void _reset() override;
     void _dump() override;
    
    
    //
    // Talking to the Amiga
    //
    
public:
    
    // Sends a keycode to the Amiga
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
    
    
    //
    // Working with the type-ahead buffer
    //
    
    // Indicates if a keycode is present
    bool bufferHasData() { return bufferIndex != 0; }

    // Reads a keycode from the type-ahead buffer
    uint8_t readFromBuffer();
    
    // Writes a keycode into the type-ahead buffer
    void writeToBuffer(uint8_t keycode);
};

#endif
