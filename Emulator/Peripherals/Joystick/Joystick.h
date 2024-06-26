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

    Descriptions descriptions = {
        {
            .name           = "Joystick1",
            .description    = "Joystick in Port 1",
            .shell          = "joystick 1"
        },
        {
            .name           = "Joystick2",
            .description    = "Joystick in Port 2",
            .shell          = "joystick 2"
        }
    };

    ConfigOptions options = {

        OPT_JOY_AUTOFIRE,
        OPT_JOY_AUTOFIRE_BULLETS,
        OPT_JOY_AUTOFIRE_DELAY
    };

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
    
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //

private:
    
    template <class T>
    void serialize(T& worker)
    {
        if (isResetter(worker)) {

            worker

            << button
            << button2
            << button3
            << axisX
            << axisY;
        }

    } SERIALIZERS(serialize);

    void _didLoad() override;

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:

    const JoystickConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void setOption(Option option, i64 value) override;


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
