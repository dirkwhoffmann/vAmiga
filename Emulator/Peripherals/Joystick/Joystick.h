// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "JoystickTypes.h"
#include "SubComponent.h"

namespace vamiga {

class Joystick : public SubComponent {

    // Reference to control port this device belongs to
    ControlPort &port;

    // Current configuration
    JoystickConfig config = {};

    // Button state
    bool button = false;
    bool button2 = false;
    bool button3 = false;

    // Horizontal joystick position (-1 = left, 1 = right, 0 = released)
    int axisX = 0;
    
    // Vertical joystick position (-1 = up, 1 = down, 0 = released)
    int axisY = 0;

    // Bullet counter used in multi-fire mode
    i64 bulletCounter = 0;
    
    // Next frame to auto-press or auto-release the fire button
    i64 nextAutofireFrame = 0;
    
    
    //
    // Initializing
    //
    
public:
    
    Joystick(Amiga& ref, ControlPort& pref);

    
    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override;
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //

private:
    
    void _reset(bool hard) override;

    template <class T>
    void serialize(T& worker)
    {
        
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    
    
    //
    // Configuring
    //
    
public:

    const JoystickConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);


    //
    // Using the device
    //
    
public:

    // Modifies the POTGOR bits according to the current button state
    void changePotgo(u16 &potgo) const;

    // Modifies the PRA bits of CIA A according to the current button state
    void changePra(u8 &pra) const;

    // Callback handler for function ControlPort::joydat()
    u16 joydat() const;

    // Callback handler for function ControlPort::ciapa()
    u8 ciapa() const;
    
    // Triggers a gamepad event
    void trigger(GamePadAction event);

    // To be called after each frame
    void eofHandler();
    
private:

    // Reloads the autofire magazine
    void reload();
    
    // Updates variable nextAutofireFrame
    void scheduleNextShot();
};

}
