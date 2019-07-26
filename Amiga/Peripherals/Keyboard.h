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
     * The original keyboard stores 10 keycodes in it's buffer.
     */
    static const size_t bufferSize = 10;
    
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
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & layout;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & state
        & handshake
        & typeAheadBuffer
        & bufferIndex;
    }
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _reset() override;
    void _dump() override;
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS };
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS };

    
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
    
    bool bufferIsEmpty() { return bufferIndex == 0; }
    bool bufferIsFull() { return bufferIndex == bufferSize; }

    // Reads a keycode from the type-ahead buffer.
    uint8_t readFromBuffer();
    
    // Writes a keycode into the type-ahead buffer.
    void writeToBuffer(uint8_t keycode);
};

#endif
