/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

ControlPort::ControlPort(int portNr)
{
    assert(portNr == 1 || portNr == 2);
    
    nr = portNr;
    setDescription(nr == 1 ? "ControlPort1" : "ControlPort2");
}

void
ControlPort::_powerOn()
{
    button = false;
    axisX = 0;
    axisY = 0;
}

void
ControlPort::_dump()
{
    plainmsg("Button:  %s AxisX: %d AxisY: %d\n", button ? "YES" : "NO", axisX, axisY);
}

void
ControlPort::didLoadFromBuffer(uint8_t **buffer)
{
    // Discard any active joystick movements
    button = false;
    axisX = 0;
    axisY = 0;
}

uint16_t
ControlPort::potgor()
{
    uint16_t result = 0xFFFF;
    
    if (hasMouse) {
        if (amiga->mouse.rightButton) {
            CLR_BIT(result, 10);
        }
    }
    
    return result;
}

uint16_t
ControlPort::joydat()
{
    return hasMouse ? amiga->mouse.getXY() : 0;
}

uint8_t
ControlPort::ciapa()
{
    assert(nr == 1 || nr == 2);
    
    uint16_t result = 0xFF;
    
    if (hasMouse) {
        if (amiga->mouse.leftButton) {
            CLR_BIT(result, (nr == 1) ? 6 : 7);
        }
    }
 
    return result;
}

void
ControlPort::scheduleNextShot()
{
    nextAutofireFrame = amiga->agnus.frame + (int)(50.0 / (2 * autofireFrequency));
}

void
ControlPort::execute()
{
    if (!autofire || autofireFrequency <= 0.0)
        return;
    
    // Wait until it's time to push or release fire
    if (amiga->agnus.frame != nextAutofireFrame)
        return;
    
    // Are there any bullets left?
    if (bulletCounter) {
        if (button) {
            button = false;
            bulletCounter--;
        } else {
            button = true;
        }
        scheduleNextShot();
    }
}

void
ControlPort::trigger(JoystickEvent event)
{
    switch (event) {
            
        case PULL_UP:
            axisY = -1;
            break;
        case PULL_DOWN:
            axisY = 1;
            break;
        case PULL_LEFT:
            axisX = -1;
            break;
        case PULL_RIGHT:
            axisX = 1;
            break;
        case PRESS_FIRE:
            
            if (autofire) {
                if (bulletCounter) {
                    // Cease fire
                    bulletCounter = 0;
                    button = false;
                } else {
                    // Load magazine
                    bulletCounter = (autofireBullets < 0) ? UINT64_MAX : autofireBullets;
                    button = true;
                    scheduleNextShot();
                }
            } else {
                button = true;
            }
            break;
        case RELEASE_X:
            axisX = 0;
            break;
        case RELEASE_Y:
            axisY = 0;
            break;
        case RELEASE_XY:
            axisX = 0;
            axisY = 0;
            break;
        case RELEASE_FIRE:
            if (!autofire)
                button = false;
            break;
            
        default:
            assert(0);
    }
}

void
ControlPort::setAutofire(bool value)
{
    if (!(autofire = value)) {
        button = false;
    }
}

void
ControlPort::setAutofireBullets(int value)
{
    autofireBullets = value;
    if (bulletCounter > 0) {
        bulletCounter = (autofireBullets < 0) ? UINT64_MAX : autofireBullets;
    }
}

/*
void
ControlPort::setXY(float x, float y)
{
    if (hasMouse) {
        debug("Moving mouse to (%f,%f)\n", x, y);
    }
}

void
ControlPort::setLeftMouseButton(bool pressed)
{
    if (hasMouse) {
        debug("Pressing left mouse button");
    }
}

void
ControlPort::setRightMouseButton(bool pressed)
{
    if (hasMouse) {
        debug("Pressing right mouse button");
    }
}
*/
