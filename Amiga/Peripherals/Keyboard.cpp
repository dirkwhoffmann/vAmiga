// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Keyboard::Keyboard()
{
    setDescription("Keyboard");
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &layout,          sizeof(layout),          PERSISTANT },
        
        { &state,           sizeof(state),           0 },
        { &handshake,       sizeof(handshake),       0 },
        { &handshake,       sizeof(handshake),       0 },
        { &typeAheadBuffer, sizeof(typeAheadBuffer), BYTE_ARRAY },
        { &bufferIndex,     sizeof(bufferIndex),     0 },
    });
}

void
Keyboard::_powerOn()
{
    memset(keyDown, 0, sizeof(keyDown));
}

void
Keyboard::_reset()
{
    
}

void
Keyboard::_dump()
{
    plainmsg("Type ahead buffer: ");
    for (unsigned i = 0; i < bufferIndex; i++) {
        plainmsg("%02X ", typeAheadBuffer[i]);
    }
    plainmsg("\n");
}

void
Keyboard::sendKeyCode(uint8_t keyCode)
{
    // Reorder and invert the key code bits (6-5-4-3-2-1-0-7)
    keyCode  = ~((keyCode << 1) | (keyCode >> 7)) & 0xFF;
    
    // Send it off to CIA A
    amiga->ciaA.setKeyCode(keyCode);
}

void
Keyboard::execute()
{
    // REMOVE ASAP
    // handshake = true;
    
    switch (state) {
            
        case KB_SEND_SYNC:
            
            if (handshake) {
                sendKeyCode(0xFF);
                state = KB_POWER_UP_KEY_STREAM;
            }
            break;
            
        case KB_POWER_UP_KEY_STREAM:
            
            if (handshake) {
                debug(2, "Sending KB_POWER_UP_KEY_STREAM\n");
                sendKeyCode(0xFD);
                state = KB_TERMINATE_KEY_STREAM;
                handshake = false;
            }
            break;
            
        case KB_TERMINATE_KEY_STREAM:
            
            if (handshake) {
                debug(2, "Sending KB_TERMINATE_KEY_STREAM\n");
                sendKeyCode(0xFE);
                state = KB_NORMAL_OPERATION;
                handshake = false;
            }
            break;
            
        case KB_NORMAL_OPERATION:
            
            if (handshake && bufferHasData()) {
                sendKeyCode(readFromBuffer());
                handshake = false;
            }
            break;
            
        default:
            assert(false);
    }
}

bool
Keyboard::keyIsPressed(long keycode)
{
    assert(keycode < 0x80);
    return keyDown[keycode];
}

void
Keyboard::pressKey(long keycode)
{
    assert(keycode < 0x80);
        
    if (!keyDown[keycode]) {
        
        debug("Pressing Amiga key %02X\n", keycode);
        
        keyDown[keycode] = true;
        writeToBuffer(keycode);
    }
}

void
Keyboard::releaseKey(long keycode)
{
    assert(keycode < 0x80);

    if (keyDown[keycode]) {
        
        debug("Releasing Amiga key %02X\n", keycode);
        
        keyDown[keycode] = false;
        writeToBuffer(keycode | 0x80);
    }
}

void
Keyboard::releaseAllKeys()
{
    for (unsigned i = 0; i < 0x80; i++) {
        releaseKey(i);
    }
}

uint8_t
Keyboard::readFromBuffer()
{
    assert(bufferHasData());
    
    uint8_t result = typeAheadBuffer[0];
    
    bufferIndex--;
    for (unsigned i = 0; i < bufferIndex; i++) {
        typeAheadBuffer[i] = typeAheadBuffer[i+1];
    }
    
    return result;
}

void
Keyboard::writeToBuffer(uint8_t keycode)
{
    if (bufferIndex < bufferSize) {
        typeAheadBuffer[bufferIndex] = keycode;
        bufferIndex++;
    }
}
