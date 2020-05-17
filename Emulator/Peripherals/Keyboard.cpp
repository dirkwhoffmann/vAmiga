// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Keyboard::Keyboard(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("Keyboard");
}

void
Keyboard::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS

    memset(keyDown, 0, sizeof(keyDown));
    
    state = KB_SELFTEST;
    execute();
}

void
Keyboard::_dump()
{
    msg("Type ahead buffer: ");
    for (unsigned i = 0; i < bufferIndex; i++) {
        msg("%02X ", typeAheadBuffer[i]);
    }
    msg("\n");
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

    if (!keyDown[keycode] && !bufferIsFull()) {

        debug(KBD_DEBUG, "Pressing Amiga key %02X\n", keycode);

        keyDown[keycode] = true;
        writeToBuffer(keycode);
        
        // Check for reset key combination (CTRL - Amiga Left - Amiga Right)
        if (keyDown[0x63] && keyDown[0x66] && keyDown[0x67]) {
            amiga.putMessage(MSG_CTRL_AMIGA_AMIGA);
        }
    }
}

void
Keyboard::releaseKey(long keycode)
{
    assert(keycode < 0x80);

    if (keyDown[keycode] && !bufferIsFull()) {

        debug(KBD_DEBUG, "Releasing Amiga key %02X\n", keycode);

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

u8
Keyboard::readFromBuffer()
{
    assert(!bufferIsEmpty());

    u8 result = typeAheadBuffer[0];

    bufferIndex--;
    for (unsigned i = 0; i < bufferIndex; i++) {
        typeAheadBuffer[i] = typeAheadBuffer[i+1];
    }

    return result;
}

void
Keyboard::writeToBuffer(u8 keycode)
{
    assert(!bufferIsFull());

    typeAheadBuffer[bufferIndex] = keycode;
    bufferIndex++;

    // Wake up the keyboard if it has gone idle
    if (!agnus.hasEvent<KBD_SLOT>()) {
        debug(KBD_DEBUG, "Wake up\n");
        state = KB_SEND;
        execute();
    }
}

void
Keyboard::setSPLine(bool value, Cycle cycle)
{
    debug(KBD_DEBUG, "setSPLine(%d)\n", value);

    if (value) {
        if (spHigh <= spLow) spHigh = cycle;
    } else {
        if (spLow <= spHigh) spLow = cycle;
    }

    // Handshake detection logic

    /* "The handshake is issued by the processor pulsing the SP line low for a
     *  minimum of 75 microseconds." [HRM 2nd edition]
     *
     * "This handshake is issued by the processor pulsing the SP line low then
     *  high. While some keyboards can detect a 1 microsecond handshake pulse,
     *  the pulse must be at least 85 microseconds for operation with all
     *  models of Amiga keyboards." [HRM 3rd editon]
     */
    int diff = (spHigh - spLow) / 28;
    bool accept = diff >= 45;
    bool reject = diff > 0 && !accept;

    if (accept) {

        debug(KBD_DEBUG, "Accepting handshake (SP low for %d usec)\n", diff);
        processHandshake();
    }

    if (reject) {

        debug(KBD_DEBUG, "REJECTING handshake (SP low for %d usec)\n", diff);
    }
}

void
Keyboard::serviceKeyboardEvent(EventID id)
{
    switch(id) {
            
        case KBD_TIMEOUT:
            
            debug(KBD_DEBUG, "KBD_TIMEOUT\n");
            
            // A timeout has occured. Try to resynchronize with the Amiga.
            state = KB_SYNC;
            execute();
            break;
            
        default:
            assert(false);
            break;
    }
}

/*
void
Keyboard::serviceKeyboardEvent(EventID id)
{
    switch(id) {

        case KBD_SELFTEST:

            debug(KBD_DEBUG, "KBD_SELFTEST\n");

            // Continue with KBD_STRM_ON after receiving a handshake
            agnus.scheduleInc<KBD_SLOT>(SEC(1), KBD_TIMEOUT, KBD_STRM_ON);
            break;

        case KBD_SYNC:

            debug(KBD_DEBUG, "KBD_SYNC\n");

            // Send a SYNC byte
            sendKeyCode(0xFF);

            // Continue with KBD_STRM_ON after receiving a handshake
            agnus.scheduleInc<KBD_SLOT>(8 * MSEC(145), KBD_TIMEOUT, KBD_STRM_ON);
            break;

        case KBD_STRM_ON:

            debug(KBD_DEBUG, "KBD_STRM_ON\n");

            // Send the "Initiate power-up key stream" code ($FD)
            sendKeyCode(0xFD);

            // Continue with KBD_STRM_OFF after receiving a handshake
            agnus.scheduleInc<KBD_SLOT>(MSEC(145), KBD_TIMEOUT, KBD_STRM_OFF);
            break;

        case KBD_STRM_OFF:

            debug(KBD_DEBUG, "KBD_STRM_OFF\n");

            // Send the "Terminate key stream" code ($FE)
            sendKeyCode(0xFE);

            // Continue with KBD_STRM_OFF after receiving a handshake
            agnus.scheduleInc<KBD_SLOT>(MSEC(145), KBD_TIMEOUT, KBD_SEND);
            break;

        case KBD_SEND:

            debug(KBD_DEBUG, "KBD_SEND\n");

            // Send a key code if the buffer is filled
            if (!bufferIsEmpty()) sendKeyCode(readFromBuffer());

            // Check if there are more keys to send
            if (!bufferIsEmpty()) {

                // Continue in this state after receiving a handshake
                agnus.scheduleRel<KBD_SLOT>(9*MSEC(145), KBD_TIMEOUT, KBD_SEND);

            } else {

                // Go idle
                agnus.rescheduleAbs<KBD_SLOT>(NEVER);

            }
            break;

        case KBD_TIMEOUT:

            debug(KBD_DEBUG, "KBD_TIMEOUT\n");

            // We've received a time-out. Reinitiate the SYNC sequence
            agnus.scheduleInc<KBD_SLOT>(DMA_CYCLES(1), KBD_SYNC);
            break;

        default:
            assert(false);
    }
}
*/

void
Keyboard::processHandshake()
{
    switch(state) {
            
        case KB_SELFTEST:
        case KB_SYNC:
            
            state = KB_STRM_ON;
            execute();
            break;
            
        case KB_STRM_ON:
            
            state = KB_STRM_OFF;
            execute();
            break;
            
        case KB_STRM_OFF:
        case KB_SEND:
            
            state = KB_SEND;
            execute();
            break;
    }
}

void
Keyboard::execute()
{
    switch(state) {
            
        case KB_SELFTEST:
            
            debug(KBD_DEBUG, "KB_SELFTEST\n");
            
            // Start a watchdog timer for the next handshake
            agnus.scheduleRel<KBD_SLOT>(SEC(1), KBD_TIMEOUT);
            break;
            
        case KB_SYNC:
            
            debug(KBD_DEBUG, "KB_SYNC\n");
            
            // Send a SYNC byte
            sendKeyCode(0xFF);
            
            // Start a watchdog timer for the next handshake
            agnus.scheduleRel<KBD_SLOT>(8 * MSEC(145), KBD_TIMEOUT);
            break;
            
        case KB_STRM_ON:
            
            debug(KBD_DEBUG, "KB_STRM_ON\n");
            
            // Send the "Initiate power-up key stream" code ($FD)
            sendKeyCode(0xFD);
            
            // Start a watchdog timer for the next handshake
            agnus.scheduleInc<KBD_SLOT>(MSEC(145), KBD_TIMEOUT);
            break;
            
        case KB_STRM_OFF:
            
            debug(KBD_DEBUG, "KB_STRM_OFF\n");
            
            // Send the "Terminate key stream" code ($FE)
            sendKeyCode(0xFE);
            
            // Start a watchdog timer for the next handshake
            agnus.scheduleInc<KBD_SLOT>(MSEC(145), KBD_TIMEOUT);
            break;
            
        case KB_SEND:
            
            debug(KBD_DEBUG, "KB_SEND\n");
            
            // Send a key code if the buffer is filled
            if (!bufferIsEmpty()) sendKeyCode(readFromBuffer());
            
            // Check if there are more keys to send
            if (!bufferIsEmpty()) {
                
                // Start a watchdog timer for the next handshake
                agnus.scheduleRel<KBD_SLOT>(9*MSEC(145), KBD_TIMEOUT);
                
            } else {
                
                // Go idle
                agnus.cancel<KBD_SLOT>();
                
            }
            break;
    }
}

void
Keyboard::sendKeyCode(u8 keyCode)
{
    debug(KBD_DEBUG, "sendKeyCode(%d)\n", keyCode);

    // Reorder and invert the key code bits (6-5-4-3-2-1-0-7)
    keyCode  = ~((keyCode << 1) | (keyCode >> 7)) & 0xFF;

    // Send it over to CIA A
    ciaa.setKeyCode(keyCode);
}
