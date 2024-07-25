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

class Joystick : public SubComponent, public Inspectable<JoystickInfo> {

    Descriptions descriptions = {
        {
            .type           = COMP_JOYSTICK,
            .name           = "Joystick1",
            .description    = "Joystick in Port 1",
            .shell          = "joystick 1"
        },
        {
            .type           = COMP_JOYSTICK,
            .name           = "Joystick2",
            .description    = "Joystick in Port 2",
            .shell          = "joystick 2"
        }
    };

    ConfigOptions options = {

        OPT_JOY_AUTOFIRE,
        OPT_JOY_AUTOFIRE_BURSTS,
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
    isize axisX = 0;

    // Vertical joystick position (-1 = up, 1 = down, 0 = released)
    isize axisY = 0;

    // Bullet counter used in autofire mode
    isize bulletCounter = 0;

    // Next frame to auto-press or auto-release the fire button
    i64 nextAutofireFrame = 0;
    i64 nextAutofireReleaseFrame = 0;

    
    //
    // Initializing
    //
    
public:
    
    Joystick(Amiga& ref, ControlPort& pref);

    Joystick& operator= (const Joystick& other) {

        CLONE(button)
        CLONE(axisX)
        CLONE(axisY)

        CLONE(config)

        return *this;
    }

    
    //
    // Methods from Serializable
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


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from Inspectable
    //

private:

    void cacheInfo(JoystickInfo &result) const override;


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
    
    // Triggers a joystick event
    void trigger(GamePadAction event);

    // To be called after each frame
    void eofHandler();
    
private:

    // Sets the button state
    void setButton(bool value);

    // Checks whether autofiring is active
    bool isAutofiring();

    // Starts or stops autofire mode
    void startAutofire();
    void stopAutofire();

    // Reloads the autofire magazine
    void reload();
    void reload(isize bullets);
    
    // Updates variable nextAutofireFrame
    void scheduleNextShot();
};

}
