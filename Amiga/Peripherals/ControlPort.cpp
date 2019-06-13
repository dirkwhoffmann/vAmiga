// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

ControlPort::ControlPort(int nr)
{
    assert(nr == 1 || nr == 2);
    
    this->nr = nr;
    setDescription(nr == 1 ? "ControlPort1" : "ControlPort2");
}

uint16_t
ControlPort::potgor()
{
    switch (device) {
            
        case CPD_MOUSE:
            return amiga->mouse.rightButton ? 0xFBFF : 0xFFFF;
            
        default:
            return 0xFFFF;
    }
}

uint16_t
ControlPort::joydat()
{
    assert(nr == 1 || nr == 2);
    assert(isControlPortDevice(device));

    switch (device) {

        case CPD_NONE:
            return 0;

        case CPD_MOUSE:

            mouseCounterX += amiga->mouse.getDeltaX();
            mouseCounterY += amiga->mouse.getDeltaY();
            // assert(HI_LO(mouseCounterY & 0xFF, mouseCounterX & 0xFF) == amiga->mouse.getXY());
            return HI_LO(mouseCounterY & 0xFF, mouseCounterX & 0xFF);

        case CPD_JOYSTICK:
            return nr == 1 ? amiga->joystick1.joydat() : amiga->joystick2.joydat();
    }
}

uint8_t
ControlPort::ciapa()
{
    switch (device) {

        case CPD_NONE:

            return 0xFF;

        case CPD_MOUSE:

            if (amiga->mouse.leftButton) {
                return (nr == 1) ? 0xBF : 0x7F;
            } else {
                return 0xFF;
            }

        case CPD_JOYSTICK:

            return nr == 1 ? amiga->joystick1.ciapa() : amiga->joystick2.ciapa();
    }
}

void
ControlPort::pokeJOYTEST(uint16_t value)
{
    mouseCounterY &= ~0b11111100;
    mouseCounterY |= HI_BYTE(value) & 0b11111100;

    mouseCounterX &= ~0b11111100;
    mouseCounterX |= LO_BYTE(value) & 0b11111100;
}

void
ControlPort::connectDevice(ControlPortDevice device)
{
    if (isControlPortDevice(device)) {
        this->device = device;
    }
}
