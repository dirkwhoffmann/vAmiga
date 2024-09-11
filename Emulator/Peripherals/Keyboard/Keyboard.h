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
#include "CmdQueue.h"
#include "RingBuffer.h"

namespace vamiga {

class Keyboard final : public SubComponent {

    Descriptions descriptions = {{

        .type           = KeyboardClass,
        .name           = "Keyboard",
        .description    = "Keyboard",
        .shell          = "keyboard"
    }};

    ConfigOptions options = {

        OPT_KBD_ACCURACY
    };

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

    // Delayed keyboard commands (used, e.g., for auto-typing)
    util::SortedRingBuffer<Cmd, 1024> pending;


    //
    // Methods
    //
    
public:
    
    using SubComponent::SubComponent;
    
    Keyboard& operator= (const Keyboard& other) {

        CLONE(state)
        CLONE(shiftReg)
        CLONE(spLow)
        CLONE(spHigh)
        CLONE(queue)
        CLONE(pending)

        CLONE(config)

        return *this;
    }

    
    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //

private:
    
    template <class T>
    void serialize(T& worker)
    {
        worker

        << state
        << shiftReg
        << spLow
        << spHigh
        << queue;

        if (isResetter(worker)) return;

        worker

        << config.accurate;

    } SERIALIZERS(serialize);

    void _didReset(bool hard) override;
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:
    
    const KeyboardConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option option, i64 value) override;

    
    //
    // Pressing and releasing keys
    //
    
public:

    // Checks whether a certain key is pressed
    bool isPressed(KeyCode keycode) const;

    // Presses or releases a key
    void press(KeyCode keycode);
    void release(KeyCode keycode);
    void toggle(KeyCode keycode);
    void releaseAll();

    // Auto-types a string
    void autoType(const string &text);

    // Discards all pending key events
    void abortAutoTyping();
    
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


    //
    // Processing commands and events
    //

public:

    // Processes a command from the command queue
    void processCommand(const Cmd &cmd);
};

}
