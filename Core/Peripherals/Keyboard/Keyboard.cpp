// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Keyboard.h"
#include "Amiga.h"
#include "IOUtils.h"

namespace vamiga {

void
Keyboard::cacheInfo(KeyboardInfo &result) const
{
    {   SYNCHRONIZED
        
        result.state = state;
        result.shiftReg = shiftReg;
    }
}

void
Keyboard::_didReset(bool hard)
{    
    std::memset(keyDown, 0, sizeof(keyDown));
    state = KbState::SELFTEST;
    execute();
}

i64
Keyboard::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::KBD_ACCURACY:  return config.accurate;

        default:
            fatalError;
    }
}

void
Keyboard::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::KBD_ACCURACY:

            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
Keyboard::setOption(Opt option, i64 value)
{
    switch (option) {
            
        case Opt::KBD_ACCURACY:
            
            config.accurate = value;
            return;

        default:
            fatalError;
    }
}

void
Keyboard::_dump(Category category, std::ostream &os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        dumpConfig(os);
    }
    
    if (category == Category::State) {
        
        os << tab("State");
        os << KbStateEnum::key(state) << std::endl;
        os << tab("Shift register");
        os << hex(shiftReg) << std::endl;
        os << tab("SP LO cycle");
        os << dec(spLow) << std::endl;
        os << tab("SP HI cycle");
        os << dec(spHigh) << std::endl;
        
        os << tab("Type ahead buffer");
        os << "[ ";
        for (isize i = queue.begin(); i != queue.end(); i = queue.next(i)) {
            os << hex(queue.elements[i]) << " ";
        }
        os << " ]" << std::endl;
        
        isize count = 0;
        for (isize i = 0; i < 128; i++) count += (isize)keyDown[i];
        os << tab("Down");
        os << dec(count) << " keys" << std::endl;
    }
}

bool
Keyboard::isPressed(KeyCode keycode) const
{
    assert(keycode < 0x80);
    return keyDown[keycode];
}

void
Keyboard::press(KeyCode keycode)
{
    assert(keycode < 0x80);
    
    SYNCHRONIZED
    
    if (!keyDown[keycode] && !queue.isFull()) {
        
        trace(KBD_DEBUG, "Pressing Amiga key %02X\n", keycode);
        
        keyDown[keycode] = true;
        queue.write(keycode);
        wakeUp();
        
        // Check for reset key combination (CTRL + Amiga Left + Amiga Right)
        if (keyDown[0x63] && keyDown[0x66] && keyDown[0x67]) {
            msgQueue.put(Msg::CTRL_AMIGA_AMIGA);
        }
    }
}

void
Keyboard::release(KeyCode keycode)
{
    assert(keycode < 0x80);
    
    SYNCHRONIZED
    
    if (keyDown[keycode] && !queue.isFull()) {
        
        trace(KBD_DEBUG, "Releasing Amiga key %02X\n", keycode);
        
        keyDown[keycode] = false;
        queue.write(keycode | 0x80);
        wakeUp();
    }
}

void
Keyboard::toggle(KeyCode keycode)
{
    isPressed(keycode) ? release(keycode) : press(keycode);
}

void
Keyboard::releaseAll()
{
    for (KeyCode i = 0; i < 0x80; i++) {
        release(i);
    }
}

void
Keyboard::wakeUp()
{
    if (!agnus.hasEvent<SLOT_KBD>()) {
        
        trace(KBD_DEBUG, "Wake up\n");
        state = KbState::SEND;
        execute();
    }
}

void
Keyboard::abortTyping()
{
    debug(KEY_DEBUG, "abortTyping()\n");

    {   SYNCHRONIZED

        if (!pending.isEmpty()) {

            pending.clear();
            releaseAll();
        }
    }
}

void
Keyboard::setSPLine(bool value, Cycle cycle)
{
    trace(KBD_DEBUG, "setSPLine(%d)\n", value);

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
    isize diff = (isize)((spHigh - spLow) / 28);
    bool accept = diff >= 1;
    bool reject = diff > 0 && !accept;

    if (accept) {

        trace(KBD_DEBUG, "Accepting handshake (SP low for %ld usec)\n", diff);
        processHandshake();
    }

    if (reject) {

        trace(KBD_DEBUG, "REJECTING handshake (SP low for %ld usec)\n", diff);
    }
}

void
Keyboard::processHandshake()
{
    // Switch to the next state
    switch(state) {
            
        case KbState::SELFTEST:  state = KbState::STRM_ON;  break;
        case KbState::SYNC:      state = KbState::STRM_ON;  break;
        case KbState::STRM_ON:   state = KbState::STRM_OFF; break;
        case KbState::STRM_OFF:  state = KbState::SEND;     break;
        case KbState::SEND:                                 break;

        default:
            fatalError;
    }
    
    // Perform all state specific actions
    execute();
}

void
Keyboard::execute()
{
    SYNCHRONIZED
    
    switch(state) {
            
        case KbState::SELFTEST:
            
            trace(KBD_DEBUG, "KB_SELFTEST\n");
            
            // Await a handshake within the next second
            agnus.scheduleRel<SLOT_KBD>(SEC(1), KBD_TIMEOUT);
            break;
            
        case KbState::SYNC:
            
            trace(KBD_DEBUG, "KB_SYNC\n");
            sendSyncPulse();
            break;
            
        case KbState::STRM_ON:
            
            trace(KBD_DEBUG, "KB_STRM_ON\n");
            
            // Send the "Initiate power-up key stream" code ($FD)
            sendKeyCode(0xFD);
            break;
            
        case KbState::STRM_OFF:
            
            trace(KBD_DEBUG, "KB_STRM_OFF\n");
            
            // Send the "Terminate key stream" code ($FE)
            sendKeyCode(0xFE);
            break;
            
        case KbState::SEND:

            trace(KBD_DEBUG, "KB_SEND\n");

            // Send a key code if the buffer is filled
            if (!queue.isEmpty()) {
                sendKeyCode(queue.read());
            } else {
                agnus.cancel<SLOT_KBD>();
            }
            break;

        default:
            fatalError;
    }
}

void
Keyboard::sendKeyCode(u8 code)
{
    trace(KBD_DEBUG, "sendKeyCode(%d)\n", code);

    // Reorder and invert the key code bits (6-5-4-3-2-1-0-7)
    shiftReg = ~((code << 1) | (code >> 7)) & 0xFF;
    
    /* Start a watchdog timer to monitor the expected handshake
     *
     * "The keyboard processor sets the KDAT line about 20 microseconds before
     *  it pulls KCLK low. KCLK stays low for about 20 microseconds, then goes
     *  high again. The processor waits another 20 microseconds before changing
     *  KDAT. Therefore, the bit rate during transmission is about 60
     *  microseconds per bit" [HRM]
     * "If the handshake pulse does not arrive within 143 ms of the last clock
     *  of the transmission, the keyboard will assume that the computer is
     *  still waiting for the rest of the transmission and is therefore out
     *  of sync." [HRM]
     */
    if (config.accurate) {
        
        // Start with the transmission of the first shift register bit
        agnus.scheduleImm<SLOT_KBD>(KBD_DAT, 0);
        
    } else {

        // In simple keyboard mode, send the keycode over in one chunk
        ciaa.setKeyCode(shiftReg);
        agnus.scheduleRel<SLOT_KBD>(8*USEC(60) + MSEC(143), KBD_TIMEOUT);
    }
}

void
Keyboard::sendSyncPulse()
{
    /* "The keyboard will then attempt to restore sync by going into 'resync
     *  mode.' In this mode, the keyboard clocks out a 1 and waits for a
     *  handshake pulse. If none arrives within 143 ms, it clocks out another
     *  1 and waits again. This process will continue until a handshake pulse
     *  arrives."
     */
    trace(KBD_DEBUG, "sendSyncPulse\n");
    
    if (config.accurate) {
        
        agnus.scheduleImm<SLOT_KBD>(KBD_SYNC_DAT0);
        
    } else {
        
        // In simple keyboard mode, send a whole byte
        sendKeyCode(0xFF);
    }
}

void
Keyboard::processCommand(const Command &cmd)
{
    if (cmd.key.delay > 0) {

        trace(KEY_DEBUG, "%s: Delayed for %f sec\n", CmdEnum::key(cmd.type), cmd.key.delay);

        pending.insert(agnus.clock + SEC(cmd.key.delay),
                       Command(cmd.type, KeyCmd { .keycode = cmd.key.keycode }));
        agnus.scheduleImm<SLOT_KEY>(KEY_AUTO_TYPE);

    } else {

        trace(KEY_DEBUG, "%s\n", CmdEnum::key(cmd.type));

        switch (cmd.type) {

            case Cmd::KEY_PRESS:         press(cmd.key.keycode); break;
            case Cmd::KEY_RELEASE:       release(cmd.key.keycode); break;
            case Cmd::KEY_RELEASE_ALL:   releaseAll(); break;
            case Cmd::KEY_TOGGLE:        toggle(cmd.key.keycode); break;

            default:
                fatalError;
        }
    }
}

}
