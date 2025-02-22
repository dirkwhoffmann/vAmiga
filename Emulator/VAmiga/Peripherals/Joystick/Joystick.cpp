// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "Joystick.h"
#include "Amiga.h"
#include "IOUtils.h"

namespace vamiga {

Joystick::Joystick(Amiga& ref, ControlPort& pref) : SubComponent(ref, pref.objid), port(pref)
{

};

void
Joystick::cacheInfo(JoystickInfo &result) const
{
    {   SYNCHRONIZED

        result.button = button;
        result.axisX = axisX;
        result.axisY = axisY;
    }
}

i64
Joystick::getOption(Opt option) const
{
    switch (option) {

        case Opt::JOY_AUTOFIRE:            return (i64)config.autofire;
        case Opt::JOY_AUTOFIRE_BURSTS:     return (i64)config.autofireBursts;
        case Opt::JOY_AUTOFIRE_BULLETS:    return (i64)config.autofireBullets;
        case Opt::JOY_AUTOFIRE_DELAY:      return (i64)config.autofireDelay;

        default:
            fatalError;
    }
}

void
Joystick::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::JOY_AUTOFIRE:
        case Opt::JOY_AUTOFIRE_BURSTS:
        case Opt::JOY_AUTOFIRE_BULLETS:
        case Opt::JOY_AUTOFIRE_DELAY:

            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
Joystick::setOption(Opt option, i64 value)
{
    switch (option) {

        case Opt::JOY_AUTOFIRE:

            config.autofire = bool(value);
            return;

        case Opt::JOY_AUTOFIRE_BURSTS:

            config.autofireBursts = bool(value);
            return;

        case Opt::JOY_AUTOFIRE_BULLETS:

            config.autofireBullets = isize(value);
            return;

        case Opt::JOY_AUTOFIRE_DELAY:

            config.autofireDelay = isize(value);
            return;

        default:
            fatalError;
    }
}

void
Joystick::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {

        dumpConfig(os);
    }

    if (category == Category::State) {

        os << tab("Button 1 pressed") << bol(button) << std::endl;
        os << tab("Button 2 pressed") << bol(button2) << std::endl;
        os << tab("Button 3 pressed") << bol(button3) << std::endl;
        os << tab("X axis") << dec(axisX) << std::endl;
        os << tab("Y axis") << dec(axisY) << std::endl;
    }
}

void
Joystick::_didLoad()
{
    // Discard any active joystick movements
    button = false;
    axisX = 0;
    axisY = 0;
}

void
Joystick::setButton(bool value)
{
    trace(PRT_DEBUG, "Button = %d\n", value);
    button = value;
}

bool
Joystick::isAutofiring()
{
    return bulletCounter > 0;
}

void
Joystick::startAutofire()
{
    trace(PRT_DEBUG, "startAutofire()\n");

    // Load magazine
    reload(config.autofireBursts ? config.autofireBullets : INT_MAX);

    // Fire the first shot
    setButton(true);

    // Schedule the release event
    nextAutofireReleaseFrame = agnus.pos.frame + config.autofireDelay;
}

void
Joystick::stopAutofire()
{
    trace(PRT_DEBUG, "stopAutofire()\n");

    // Release button and empty the bullet counter
    setButton(false);
    bulletCounter = 0;

    // Clear all events
    nextAutofireFrame = nextAutofireReleaseFrame = 0;
}

void
Joystick::reload(isize bullets)
{
    bulletCounter = bullets;
}

void
Joystick::changePotgo(u16 &potgo) const
{
    u16 maskR = port.isPort1() ? 0x0400 : 0x4000;
    u16 maskM = port.isPort1() ? 0x0100 : 0x1000;

    if (button2) {
        potgo &= ~maskR;
    } else {
        potgo |= maskR;
    }

    if (button3) {
        potgo &= ~maskM;
    } else { 
        potgo |= maskM;
    }
}

void
Joystick::changePra(u8 &pra) const
{
    u16 mask = port.isPort1() ? 0x40 : 0x80;

    if (button) pra &= ~mask;
}

u16
Joystick::joydat() const
{
    // debug("joydat\n");

    u16 result = 0;

    /* 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
     * Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 X7 X6 X5 X4 X3 X2 X1 X0
     *
     *      Left: Y1 = 1
     *     Right: X1 = 1
     *        Up: Y0 xor Y1 = 1
     *      Down: X0 xor X1 = 1
     */

    if (axisX == -1) result |= 0x0300;
    else if (axisX == 1) result |= 0x0003;

    if (axisY == -1) result ^= 0x0100;
    else if (axisY == 1) result ^= 0x0001;

    return result;
}

u8
Joystick::ciapa() const
{
    return button ? (port.isPort1() ? 0xBF : 0x7F) : 0xFF;
}

void
Joystick::trigger(GamePadAction event)
{
    assert_enum(GamePadAction, event);

    debug(PRT_DEBUG, "trigger(%s)\n", GamePadActionEnum::key(event));

    switch (event) {

        case GamePadAction::PULL_UP:       axisY = -1; break;
        case GamePadAction::PULL_DOWN:     axisY =  1; break;
        case GamePadAction::PULL_LEFT:     axisX = -1; break;
        case GamePadAction::PULL_RIGHT:    axisX =  1; break;
        case GamePadAction::PRESS_FIRE2:   button2 = true; break;
        case GamePadAction::PRESS_FIRE3:   button3 = true; break;
        case GamePadAction::RELEASE_X:     axisX =  0; break;
        case GamePadAction::RELEASE_Y:     axisY =  0; break;
        case GamePadAction::RELEASE_XY:    axisX = axisY = 0; break;
        case GamePadAction::RELEASE_FIRE2: button2 = false; break;
        case GamePadAction::RELEASE_FIRE3: button3 = false; break;

        case GamePadAction::PRESS_FIRE:

            // If autofire is enabled...
            if (config.autofire) {

                // ...check if we are currently firing.
                if (isAutofiring()) {

                    // If yes, the required action depends on the autofire mode.
                    if (config.autofireBursts) {

                        // In burst mode, reload the magazine.
                        reload(config.autofireBullets);

                    } else {

                        // Otherwise, stop firing.
                        stopAutofire();
                    }

                } else {

                    // We are currently not firing. Initiate the first shot.
                    startAutofire();
                }

            } else {

                setButton(true);
            }
            break;

        case GamePadAction::RELEASE_FIRE:

            if (!config.autofire) setButton(false);
            break;

        default:
            break;
    }
    port.setDevice(ControlPortDevice::JOYSTICK);
}

void
Joystick::eofHandler()
{
    if (isAutofiring()) {

        if (agnus.pos.frame == nextAutofireFrame) {

            setButton(true);
            nextAutofireReleaseFrame = nextAutofireFrame + config.autofireDelay;
        }

        if (agnus.pos.frame == nextAutofireReleaseFrame) {

            setButton(false);
            if (--bulletCounter > 0) {
                nextAutofireFrame = nextAutofireReleaseFrame + config.autofireDelay;
            } else {
                stopAutofire();
            }
        }
    }
}

}
