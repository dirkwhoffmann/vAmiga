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

#include "AmigaComponent.h"

class Keyboard : public AmigaComponent {

    // Current configuration
     KeyboardConfig config;

public:
    
    /* The keybord layout identifier.
     * This variable is set and read by the GUI, only.
     */
    long layout = 0;
    
private:

    /* Time stamps recording an Amiga triggered change of the SP line.
     * The SP line is driven by the Amiga to transmit a handshake.
     */
    Cycle spLow;
    Cycle spHigh;

    // The keycode type-ahead buffer (10 keycodes on an original Amiga)
    static const size_t bufferSize = 10;
    u8 typeAheadBuffer[bufferSize];
    
    // Next free position in the type ahead buffer
    u8 bufferIndex;
    
    // Remebers the keys that are currently held down
    bool keyDown[128];

    
    //
    // Constructing and destructing
    //
    
public:
    
    Keyboard(Amiga& ref);
    

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

        & spLow
        & spHigh
        & typeAheadBuffer
        & bufferIndex;
    }


    //
    // Configuring
    //

    KeyboardConfig getConfig() { return config; }


    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _reset() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Pressing and releasing keys
    //
    
public:

    bool keyIsPressed(long keycode);
    void pressKey(long keycode);
    void releaseKey(long keycode);
    void releaseAllKeys();


    //
    // Managing the type-ahead buffer
    //

private:

    bool bufferIsEmpty() { return bufferIndex == 0; }
    bool bufferIsFull() { return bufferIndex == bufferSize; }

    // Reads a keycode from the type-ahead buffer.
    u8 readFromBuffer();

    // Writes a keycode into the type-ahead buffer.
    void writeToBuffer(u8 keycode);

    
    //
    // Talking to the Amiga
    //
    
public:

    /* Emulates a handshake from the Amiga
     * This function is called whenever the CIA switches the serial register
     * between from input mode to output mode or vice versa.
     */
    void setSPLine(bool value, Cycle cycle);

    // Services a keyboard event
    void serviceKeyboardEvent(EventID id);

private:

    // Sends a keycode to the Amiga
    void sendKeyCode(u8 keyCode);
};

#endif
