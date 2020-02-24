// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

ControlPort::ControlPort(int nr, Amiga& ref) : AmigaComponent(ref)
{
    assert(nr == 1 || nr == 2);
    
    this->nr = nr;
    setDescription(nr == 1 ? "ControlPort1" : "ControlPort2");    
}

void
ControlPort::_inspect()
{
    pthread_mutex_lock(&lock);

    info.joydat = joydat();

    // Extract pin values from joydat value
    bool x0 = GET_BIT(info.joydat, 0);
    bool x1 = GET_BIT(info.joydat, 1);
    bool y0 = GET_BIT(info.joydat, 8);
    bool y1 = GET_BIT(info.joydat, 9);
    info.m0v = y0 ^ !y1;
    info.m0h = x0 ^ !x1;
    info.m1v = !y1;
    info.m1h = !x1;

    info.potgo = paula.potgo;
    info.potgor = paula.peekPOTGOR();
    info.potdat = (nr == 1) ? paula.peekPOTxDAT<0>() : paula.peekPOTxDAT<1>();

    pthread_mutex_unlock(&lock);
}

void
ControlPort::_dump()
{
    msg("         device: %d\n", device);
    msg("  mouseCounterX: %d\n", mouseCounterX);
    msg("  mouseCounterY: %d\n", mouseCounterY);
}

ControlPortInfo
ControlPort::getInfo()
{
    ControlPortInfo result;

    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);

    return result;
}

u16
ControlPort::joydat()
{
    assert(nr == 1 || nr == 2);
    assert(isControlPortDevice(device));

    switch (device) {

        case CPD_NONE:

            return 0;

        case CPD_MOUSE:

            mouseCounterX += mouse.getDeltaX();
            mouseCounterY += mouse.getDeltaY();
            return HI_LO(mouseCounterY & 0xFF, mouseCounterX & 0xFF);

        case CPD_JOYSTICK:

            return nr == 1 ? joystick1.joydat() : joystick2.joydat();
    }
}

/*
u8
ControlPort::ciapa()
{
    switch (device) {

        case CPD_NONE:

            return 0xFF;

        case CPD_MOUSE:

            if (mouse.leftButton) {
                return (nr == 1) ? 0xBF : 0x7F;
            } else {
                return 0xFF;
            }

        case CPD_JOYSTICK:

            return nr == 1 ? joystick1.ciapa() : joystick2.ciapa();
    }
}
*/

void
ControlPort::pokeJOYTEST(u16 value)
{
    mouseCounterY &= ~0b11111100;
    mouseCounterY |= HI_BYTE(value) & 0b11111100;

    mouseCounterX &= ~0b11111100;
    mouseCounterX |= LO_BYTE(value) & 0b11111100;
}

void
ControlPort::changePotgo(u16 &potgo)
{
    if (device == CPD_MOUSE) mouse.changePotgo(nr, potgo);
}

void
ControlPort::changePra(u8 &pra)
{
    if (device == CPD_MOUSE) {
        mouse.changePra(nr, pra);
        return;
    }
    if (device == CPD_JOYSTICK) {
        nr == 1 ? joystick1.changePra(pra) : joystick2.changePra(pra);
        return;
    }
}

void
ControlPort::connectDevice(ControlPortDevice device)
{
    if (isControlPortDevice(device)) this->device = device;
}
