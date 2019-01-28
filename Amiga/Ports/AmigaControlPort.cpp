/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

AmigaControlPort::AmigaControlPort(int portNr)
{
    assert(portNr == 1 || portNr == 2);
    
    nr = portNr;
    setDescription("ControlPort");
}

void
AmigaControlPort::_powerOn()
{
    button = false;
    axisX = 0;
    axisY = 0;
}

void
AmigaControlPort::didLoadFromBuffer(uint8_t **buffer)
{
    // Discard any active joystick movements
    button = false;
    axisX = 0;
    axisY = 0;
}

void
AmigaControlPort::_dump()
{
    plainmsg("Button:  %s AxisX: %d AxisY: %d\n", button ? "YES" : "NO", axisX, axisY);
}

void
AmigaControlPort::scheduleNextShot()
{
    nextAutofireFrame = amiga->denise.frame + (int)(50.0 / (2 * autofireFrequency));
}

void
AmigaControlPort::execute()
{
    if (!autofire || autofireFrequency <= 0.0)
        return;
    
    // Wait until it's time to push or release fire
    if (amiga->denise.frame != nextAutofireFrame)
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
AmigaControlPort::trigger(JoystickEvent event)
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
AmigaControlPort::setAutofire(bool value)
{
    if (!(autofire = value)) {
        button = false;
    }
}

void
AmigaControlPort::setAutofireBullets(int value)
{
    autofireBullets = value;
    if (bulletCounter > 0) {
        bulletCounter = (autofireBullets < 0) ? UINT64_MAX : autofireBullets;
    }
}

void
AmigaControlPort::setXY(float x, float y)
{
    debug("Moving mouse to (%f,%f)\n", x, y);
}

void
AmigaControlPort::setLeftMouseButton(bool pressed)
{
    debug("Pressing left mouse button");
}

void
AmigaControlPort::setRightMouseButton(bool pressed)
{
    debug("Pressing right mouse button");
}
