// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "Keyboard.h"
#include "Amiga.h"
#include "Emulator.h"
#include "utl/io.h"

namespace vamiga {

Keyboard::Keyboard(Amiga& ref) : SubComponent(ref)
{
    info.bind([this] { return cacheInfo(); } );
};

KeyboardInfo
Keyboard::cacheInfo() const
{
    KeyboardInfo info;

    info.state = state;
    info.shiftReg = shiftReg;

    return info;
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
            throw CoreError(CoreError::OPT_UNSUPPORTED);
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
    using namespace utl;

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
        
        logmsg(LOG_KBD, "Pressing Amiga key %02X\n", keycode);
        
        keyDown[keycode] = true;
        queue.write(keycode);
        wakeUp();

        // Check for reset key combination (CTRL + Amiga Left + Amiga Right)
        if (keyDown[0x63] && keyDown[0x66] && keyDown[0x67]) {
            msgQueue.put(Msg::CTRL_AMIGA_AMIGA);
        }

        msgQueue.put(Msg::KB_PRESS, keycode);
    }
}

void
Keyboard::release(KeyCode keycode)
{
    assert(keycode < 0x80);

    SYNCHRONIZED

    // Only proceed if the key is currently pressed and unlocked
    if (keyDown[keycode] && !isLocked(keycode) && !queue.isFull()) {

        logmsg(LOG_KBD, "Releasing Amiga key %02X\n", keycode);

        keyDown[keycode] = false;
        queue.write(keycode | 0x80);
        wakeUp();

        msgQueue.put(Msg::KB_RELEASE, keycode);
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

bool
Keyboard::isLocked(KeyCode keycode) const
{
    assert(keycode < 0x80);
    return locked[keycode];
}

void
Keyboard::lock(KeyCode keycode)
{
    assert(keycode < 0x80);

    SYNCHRONIZED

    if (!locked[keycode]) {

        locked[keycode] = true;
        msgQueue.put(Msg::KB_LOCK, keycode);
    }
}

void
Keyboard::unlock(KeyCode keycode)
{
    assert(keycode < 0x80);

    SYNCHRONIZED

    if (locked[keycode]) {

        locked[keycode] = false;
        msgQueue.put(Msg::KB_UNLOCK, keycode);
    }
}

void
Keyboard::unlockAll()
{
    for (KeyCode i = 0; i < 0x80; i++) {
        unlock(i);
    }
}

void
Keyboard::wakeUp()
{
    if (!agnus.hasEvent<SLOT_KBD>()) {
        
        logmsg(LOG_KBD, "Wake up\n");
        state = KbState::SEND;
        execute();
    }
}

optional<std::pair<KeyCode, bool>>
Keyboard::keyFor(char c)
{
    // Keys of the main block on a US keyboard (unshifted, shifted)
    static const struct { KeyCode code; char plain; char shifted; } keys[] = {

        { 0x00, '`', '~' }, { 0x01, '1', '!' }, { 0x02, '2', '@' }, { 0x03, '3', '#' },
        { 0x04, '4', '$' }, { 0x05, '5', '%' }, { 0x06, '6', '^' }, { 0x07, '7', '&' },
        { 0x08, '8', '*' }, { 0x09, '9', '(' }, { 0x0A, '0', ')' }, { 0x0B, '-', '_' },
        { 0x0C, '=', '+' }, { 0x0D, '\\', '|' },
        { 0x10, 'q', 'Q' }, { 0x11, 'w', 'W' }, { 0x12, 'e', 'E' }, { 0x13, 'r', 'R' },
        { 0x14, 't', 'T' }, { 0x15, 'y', 'Y' }, { 0x16, 'u', 'U' }, { 0x17, 'i', 'I' },
        { 0x18, 'o', 'O' }, { 0x19, 'p', 'P' }, { 0x1A, '[', '{' }, { 0x1B, ']', '}' },
        { 0x20, 'a', 'A' }, { 0x21, 's', 'S' }, { 0x22, 'd', 'D' }, { 0x23, 'f', 'F' },
        { 0x24, 'g', 'G' }, { 0x25, 'h', 'H' }, { 0x26, 'j', 'J' }, { 0x27, 'k', 'K' },
        { 0x28, 'l', 'L' }, { 0x29, ';', ':' }, { 0x2A, '\'', '"' },
        { 0x31, 'z', 'Z' }, { 0x32, 'x', 'X' }, { 0x33, 'c', 'C' }, { 0x34, 'v', 'V' },
        { 0x35, 'b', 'B' }, { 0x36, 'n', 'N' }, { 0x37, 'm', 'M' }, { 0x38, ',', '<' },
        { 0x39, '.', '>' }, { 0x3A, '/', '?' }
    };

    switch (c) {

        case ' ':   return std::pair { KeyCode(0x40), false };
        case '\b':  return std::pair { KeyCode(0x41), false };
        case '\t':  return std::pair { KeyCode(0x42), false };
        case '\n':  return std::pair { KeyCode(0x44), false };

        default:
            break;
    }

    for (auto &key : keys) {

        if (c == key.plain) return std::pair { key.code, false };
        if (c == key.shifted) return std::pair { key.code, true };
    }
    return { };
}

void
Keyboard::autoType(const string &text)
{
    constexpr KeyCode shift = 0x60;
    constexpr double delta = 0.02;

    double delay = 0.0;

    auto put = [&](Cmd type, KeyCode code) {

        emulator.put(Command(type, KeyCmd { .keycode = code, .delay = delay }));
        delay += delta;
    };

    for (char c : text) {

        if (c == '\r') continue;

        auto key = keyFor(c);
        if (!key) continue;

        if (key->second) put(Cmd::KEY_PRESS, shift);
        put(Cmd::KEY_PRESS, key->first);
        put(Cmd::KEY_RELEASE, key->first);
        if (key->second) put(Cmd::KEY_RELEASE, shift);
    }
}

void
Keyboard::abortTyping()
{
    logmsg(LOG_KEY, "abortTyping()\n");

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
    logmsg(LOG_KBD, "setSPLine(%d)\n", value);

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

        logmsg(LOG_KBD, "Accepting handshake (SP low for %td usec)\n", diff);
        processHandshake();
    }

    if (reject) {

        logmsg(LOG_KBD, "REJECTING handshake (SP low for %td usec)\n", diff);
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
            
            logmsg(LOG_KBD, "KB_SELFTEST\n");
            
            // Await a handshake within the next second
            agnus.scheduleRel<SLOT_KBD>(SEC(1), KBD_TIMEOUT);
            break;
            
        case KbState::SYNC:
            
            logmsg(LOG_KBD, "KB_SYNC\n");
            sendSyncPulse();
            break;
            
        case KbState::STRM_ON:
            
            logmsg(LOG_KBD, "KB_STRM_ON\n");
            
            // Send the "Initiate power-up key stream" code ($FD)
            sendKeyCode(0xFD);
            break;
            
        case KbState::STRM_OFF:
            
            logmsg(LOG_KBD, "KB_STRM_OFF\n");
            
            // Send the "Terminate key stream" code ($FE)
            sendKeyCode(0xFE);
            break;
            
        case KbState::SEND:

            logmsg(LOG_KBD, "KB_SEND\n");

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
    logmsg(LOG_KBD, "sendKeyCode(%d)\n", code);

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
    logmsg(LOG_KBD, "sendSyncPulse\n");
    
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

        logmsg(LOG_KEY, "%s: Delayed for %f sec\n", CmdEnum::key(cmd.type), cmd.key.delay);

        if (pending.isFull()) {

            logmsg(LOG_WARN, "%s: Too many pending key events. Dropped.\n", CmdEnum::key(cmd.type));
            return;
        }
        pending.insert(agnus.clock + SEC(cmd.key.delay),
                       Command(cmd.type, KeyCmd { .keycode = cmd.key.keycode }));
        agnus.scheduleImm<SLOT_KEY>(KEY_AUTO_TYPE);

    } else {

        logmsg(LOG_KEY, "%s\n", CmdEnum::key(cmd.type));

        switch (cmd.type) {

            case Cmd::KEY_PRESS:         press(cmd.key.keycode); break;
            case Cmd::KEY_RELEASE:       release(cmd.key.keycode); break;
            case Cmd::KEY_RELEASE_ALL:   releaseAll(); break;
            case Cmd::KEY_TOGGLE:        toggle(cmd.key.keycode); break;
            case Cmd::KEY_LOCK:          lock(cmd.key.keycode); break;
            case Cmd::KEY_UNLOCK:        unlock(cmd.key.keycode); break;

            default:
                fatalError;
        }
    }
}

}
