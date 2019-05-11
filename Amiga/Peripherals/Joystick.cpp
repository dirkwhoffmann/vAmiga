/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Joystick::Joystick(int nr)
{
    assert(nr == 1 || nr == 2);
    
    this->nr = nr;
    setDescription(nr == 1 ? "Joystick1" : "Joystick2");
}

void
Joystick::_powerOn()
{
    button = false;
    axisX = 0;
    axisY = 0;
}

void
Joystick::_dump()
{
    plainmsg("Button:  %s AxisX: %d AxisY: %d\n", button ? "YES" : "NO", axisX, axisY);
}

void
Joystick::didLoadFromBuffer(uint8_t **buffer)
{
    // Discard any active joystick movements
    button = false;
    axisX = 0;
    axisY = 0;
}

void
Joystick::setAutofire(bool value)
{
    autofire = value;
    
    // Release button immediately if autofire-mode is switches off
    if (value == false) button = false;
}

void
Joystick::setAutofireBullets(int value)
{
    autofireBullets = value;
    
    // Update the bullet counter if we're currently firing
    if (bulletCounter > 0) {
        bulletCounter = (autofireBullets < 0) ? UINT64_MAX : autofireBullets;
    }
}

void
Joystick::scheduleNextShot()
{
    nextAutofireFrame = _agnus->frame + (int)(50.0 / (2 * autofireFrequency));
}

void
Joystick::trigger(JoystickEvent event)
{
    switch (event) {
            
        case PULL_UP:    axisY = -1; break;
        case PULL_DOWN:  axisY =  1; break;
        case PULL_LEFT:  axisX = -1; break;
        case PULL_RIGHT: axisX =  1; break;
            
        case RELEASE_X:  axisX =  0; break;
        case RELEASE_Y:  axisY =  0; break;
        case RELEASE_XY: axisX = axisY = 0; break;
            
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
            
        case RELEASE_FIRE:
            if (!autofire) button = false;
            break;
            
        default:
            assert(0);
    }
}

void
Joystick::execute()
{
    if (!autofire || autofireFrequency <= 0.0)
        return;
    
    // Wait until it's time to push or release fire
    if (_agnus->frame != nextAutofireFrame)
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

