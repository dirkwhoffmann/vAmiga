/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Joystick::Joystick()
{
    setDescription("Joystick");
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
    
    // Update bullet counter if we're currently firing
    if (bulletCounter > 0) {
        bulletCounter = (autofireBullets < 0) ? UINT64_MAX : autofireBullets;
    }
}
