// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

SerialPort::SerialPort()
{
    setDescription("SerialPort");

    registerSnapshotItems(vector<SnapshotItem> {

        { &device,        sizeof(device),        PERSISTANT },
        { &port,          sizeof(port),          0 },
    });
}

void
SerialPort::_initialize()
{
    events = &amiga->agnus.events;
    uart = &amiga->paula.uart;
}

void
SerialPort::_powerOn()
{
    port = 0x1FFFFFE; 
}

void
SerialPort::_dump()
{
    plainmsg("    device: %d\n", device);
    plainmsg("      port: %X\n", port);
}

void
SerialPort::connectDevice(SerialPortDevice device)
{
    if (isSerialPortDevice(device)) {
        this->device = device;
    }
}

bool
SerialPort::getPin(int nr)
{
    assert(nr >= 1 && nr <= 25);

    bool result = GET_BIT(port, nr);

    debug("port = %X getPin(%d) = %d\n", port, result);
    return result;
}

void
SerialPort::setPin(int nr, bool value)
{
    assert(nr >= 1 && nr <= 25);

    SET_BIT(port, nr);
    debug("setPin(%d,%d) port = %X\n", nr, value, port);
}

void
SerialPort::setRXD(bool value)
{
    setPin(3, value);

    // Schedule the first reception event if this is the first bit
    if (!events->hasEventSec(RXD_SLOT)) {
        events->scheduleSecRel(RXD_SLOT, uart->rate() / 2, RXD_BIT);
    }
}
