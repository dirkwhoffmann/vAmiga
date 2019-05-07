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
    
    /* The acknowledge signal sent from the Amiga side.
     * When a keycode has been sent to the Amiga, the keyboard waits for a
     * handshake signal before transmitting any more keycodes. The handshake
     * is transmitted via the SP line of CIA A.
     */
    bool handshake;
    
    /* Size of the keycode type-ahead buffer
     * The original keyboard has as type-ahead buffer of size 10. We allow
     * a slightly bigger number.
     */
    static const size_t bufferSize = 16;
    
    // The keycode type-ahead buffer
    uint8_t typeAheadBuffer[bufferSize];
    
    // Next free position in the type ahead buffer
    uint8_t bufferIndex;
    
    // Remebers the keys that are currently held down
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
    // Pressing and releasing keys
    //
    
public:

    bool keyIsPressed(long keycode);
    void pressKey(long keycode);
    void releaseKey(long keycode);
    void releaseAllKeys();
    
    
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
    
    
    //
    // Managing the type-ahead buffer
    //
    
    // Returns true if the buffer contains at least one keycode.
    bool bufferHasData() { return bufferIndex != 0; }

    // Reads a keycode from the type-ahead buffer.
    uint8_t readFromBuffer();
    
    // Writes a keycode into the type-ahead buffer.
    void writeToBuffer(uint8_t keycode);
};

#endif
