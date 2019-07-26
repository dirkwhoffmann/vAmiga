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

    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {

        { &mouseCounterX, sizeof(mouseCounterX), 0 },
        { &mouseCounterY, sizeof(mouseCounterY), 0 },
        { &potX,          sizeof(potX),          0 },
        { &potY,          sizeof(potY),          0 }
    });

    
}

void
ControlPort::_inspect()
{
    pthread_mutex_lock(&lock);

    /* The port pin values are not stored in plain text. We can easily
     * reverse-engineer them out of the JOYDAT register value though.
     */
    uint16_t dat = joydat();
    bool x0 = !!GET_BIT(dat, 0);
    bool x1 = !!GET_BIT(dat, 1);
    bool y0 = !!GET_BIT(dat, 8);
    bool y1 = !!GET_BIT(dat, 9);

    info.m0v = y0 ^ !y1;
    info.m0h = x0 ^ !x1;
    info.m1v = !y1;
    info.m1h = !x1;

    info.potx = 0; // TODO
    info.poty = 0; // TODO

    pthread_mutex_unlock(&lock);
}

void
ControlPort::_dump()
{
    plainmsg("         device: %d\n", device);
    plainmsg("  mouseCounterX: %d\n", mouseCounterX);
    plainmsg("  mouseCounterY: %d\n", mouseCounterY);
}

size_t
ControlPort::_save(uint8_t *buffer)
{
    SerWriter writer(buffer);

    applyToPersistentItems(writer);
    applyToResetItems(writer);

    debug(SNAP_DEBUG, "Serialized to %d bytes\n", writer.ptr - buffer);
    return writer.ptr - buffer;
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

uint16_t
ControlPort::potgor()
{
    if (device == CPD_MOUSE)
        if (amiga->mouse.rightButton)
            return (nr == 1) ? 0xFBFF : 0xBFFF;

    return 0xFFFF;
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
