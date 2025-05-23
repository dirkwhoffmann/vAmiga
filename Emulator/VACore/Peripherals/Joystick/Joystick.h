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

class Joystick final : public SubComponent, public Inspectable<JoystickInfo> {

    Descriptions descriptions = {
        {
            .type           = Class::Joystick,
            .name           = "Joystick1",
            .description    = "Joystick in Port 1",
            .shell          = "joystick1"
        },
        {
            .type           = Class::Joystick,
            .name           = "Joystick2",
            .description    = "Joystick in Port 2",
            .shell          = "joystick2"
        }
    };

    ConfigOptions options = {

        Opt::JOY_AUTOFIRE,
        Opt::JOY_AUTOFIRE_BURSTS,
        Opt::JOY_AUTOFIRE_BULLETS,
        Opt::JOY_AUTOFIRE_DELAY
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

        CLONE(config)

        CLONE(button)
        CLONE(button2)
        CLONE(button3)
        CLONE(axisX)
        CLONE(axisY)
        CLONE(bulletCounter)
        CLONE(nextAutofireFrame)
        CLONE(nextAutofireReleaseFrame)

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
            << axisY
            << bulletCounter
            << nextAutofireFrame
            << nextAutofireReleaseFrame;

        } else {

            worker

            << config.autofire
            << config.autofireBursts
            << config.autofireBullets
            << config.autofireDelay;
        }

    } SERIALIZERS(serialize, override);

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
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;


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
    void reload(isize bullets);
};

}
