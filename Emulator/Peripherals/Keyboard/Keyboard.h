// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "KeyboardTypes.h"
#include "AgnusTypes.h"
#include "SubComponent.h"
#include "RingBuffer.h"

namespace vamiga {

class Keyboard : public SubComponent {

    // Current configuration
    KeyboardConfig config;

    // The current keyboard state
    KeyboardState state;
    
    // Shift register storing the transmission bits
    u8 shiftReg;
    
    /* Time stamps recording an Amiga triggered change of the SP line. The SP
     * line is driven by the Amiga to transmit a handshake.
     */
    Cycle spLow;
    Cycle spHigh;

    // The keycode type-ahead buffer. The Amiga can hold up to 10 keycodes
    util::RingBuffer<KeyCode, 10> queue;

    // Remebers the keys that are currently held down
    bool keyDown[128];

    
    //
    // Initialization
    //
    
public:
    
    using SubComponent::SubComponent;
    
    
    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override { return "Keyboard"; }
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //

private:
    
    void _reset(bool hard) override;

    template <class T>
    void serialize(T& worker)
    {
        worker

        << state
        << shiftReg
        << spLow
        << spHigh
        << queue;

        if (util::isResetter(worker)) return;

        worker 

        << config.accurate;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring
    //
    
public:
    
    const KeyboardConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);

    
    //
    // Pressing and releasing keys
    //
    
public:

    bool keyIsPressed(KeyCode keycode) const;
    void pressKey(KeyCode keycode);
    void releaseKey(KeyCode keycode);
    void toggleKey(KeyCode keycode);
    void releaseAllKeys();

    void autoType(KeyCode keycode, Cycle duration = MSEC(100), Cycle delay = 0);
    
private:
    
    // Wake up the keyboard if it has gone idle
    void wakeUp();
    

    //
    // Talking to the Amiga
    //
    
public:

    /* Emulates a change on the SP line. This function is called whenever the
     * CIA switches the serial register from input mode to output mode or vice
     * versa. The SP line is controlled by the Amiga to signal a handshake.
     */
    void setSPLine(bool value, Cycle cycle);
    
    // Services a keyboard event
    void serviceKeyboardEvent(EventID id);

    // Services an auto typing event
    void serviceKeyEvent();
    
    
    //
    // Running the device
    //

private:

    // Processes a detected handshake
    void processHandshake();

    // Performs all actions according to the current state
    void execute();
    
    // Sends a keycode to the Amiga
    void sendKeyCode(u8 keyCode);

    // Sends a sync pulse to the Amiga
    void sendSyncPulse();
};

}
