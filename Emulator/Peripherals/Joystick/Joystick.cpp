// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Joystick.h"
#include "Amiga.h"
#include "IOUtils.h"

namespace vamiga {

Joystick::Joystick(Amiga& ref, ControlPort& pref) : SubComponent(ref), port(pref)
{

};

const char *
Joystick::getDescription() const
{
    return port.isPort1() ? "Joystick1" : "Joystick2";
}

void
Joystick::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    // Discard any active joystick movements
    button = false;
    axisX = 0;
    axisY = 0;
}

void
Joystick::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    std::vector <Option> options = {
        
        OPT_AUTOFIRE,
        OPT_AUTOFIRE_BULLETS,
        OPT_AUTOFIRE_DELAY
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
}

i64
Joystick::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_AUTOFIRE:            return (i64)config.autofire;
        case OPT_AUTOFIRE_BULLETS:    return (i64)config.autofireBullets;
        case OPT_AUTOFIRE_DELAY:      return (i64)config.autofireDelay;

        default:
            fatalError;
    }
}

void
Joystick::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_AUTOFIRE:
            
            config.autofire = bool(value);

            // Release button immediately if autofire-mode is switches off
            if (value == false) button = false;

            return;

        case OPT_AUTOFIRE_BULLETS:
            
            config.autofireBullets = isize(value);
            
            // Update the bullet counter if we're currently firing
            if (bulletCounter > 0) reload();

            return;

        case OPT_AUTOFIRE_DELAY:
            
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
    
    if (category == Category::Inspection) {
        
        os << tab("Button 1 pressed") << bol(button) << std::endl;
        os << tab("Button 2 pressed") << bol(button2) << std::endl;
        os << tab("Button 3 pressed") << bol(button3) << std::endl;
        os << tab("X axis") << dec(axisX) << std::endl;
        os << tab("Y axis") << dec(axisY) << std::endl;
    }
}

isize
Joystick::didLoadFromBuffer(const u8 *buffer)
{
    // Discard any active joystick movements
    button = false;
    axisX = 0;
    axisY = 0;

    return 0;
}

void
Joystick::reload()
{
    bulletCounter = (config.autofireBullets < 0) ? INT64_MAX : config.autofireBullets;
}

void
Joystick::scheduleNextShot()
{
    nextAutofireFrame = agnus.pos.frame + config.autofireDelay;
}

void
Joystick::changePotgo(u16 &potgo) const
{
    u16 maskR = port.isPort1() ? 0x0400 : 0x4000;
    u16 maskM = port.isPort1() ? 0x0100 : 0x1000;

    if (button2) {
        potgo &= ~maskR;
    } else { // if (config.pullUpResistors) {  TODO: MOVE config.pullUpResistors to Port
        potgo |= maskR;
    }

    if (button3) {
        potgo &= ~maskM;
    } else { // if (config.pullUpResistors) {  TODO: MOVE config.pullUpResistors to Port
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
            
        case PULL_UP:       axisY = -1; break;
        case PULL_DOWN:     axisY =  1; break;
        case PULL_LEFT:     axisX = -1; break;
        case PULL_RIGHT:    axisX =  1; break;
        case PRESS_FIRE2:   button2 = true; break;
        case PRESS_FIRE3:   button3 = true; break;
        case RELEASE_X:     axisX =  0; break;
        case RELEASE_Y:     axisY =  0; break;
        case RELEASE_XY:    axisX = axisY = 0; break;
        case RELEASE_FIRE2: button2 = false; break;
        case RELEASE_FIRE3: button3 = false; break;


        case PRESS_FIRE:
            if (config.autofire) {
                if (bulletCounter) {
                    
                    // Cease fire
                    bulletCounter = 0;
                    button = false;
                    
                } else {

                    // Load magazine
                    button = true;
                    reload();
                    scheduleNextShot();
                }
                
            } else {
                button = true;
            }
            break;

        case RELEASE_FIRE:
            if (!config.autofire) button = false;
            break;
            
        default:
            break;
    }
    port.setDevice(CPD_JOYSTICK);
}

void
Joystick::eofHandler()
{
    // Only proceed if auto fire is enabled
    if (!config.autofire || config.autofireDelay < 0) return;

    // Only proceed if a trigger frame has been reached
    if (agnus.pos.frame != nextAutofireFrame) return;

    // Only proceed if there are bullets left
    if (bulletCounter == 0) return;
    
    if (button) {
        button = false;
        bulletCounter--;
    } else {
        button = true;
    }
    scheduleNextShot();
}

}
