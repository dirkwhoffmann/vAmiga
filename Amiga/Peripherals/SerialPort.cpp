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

    bool result = !!GET_BIT(port, nr);

    debug(SER_DEBUG, "getPin(%2d) = %d port = %X\n", nr, result, port);
    return result;
}

void
SerialPort::setPin(int nr, bool value)
{
    debug(SER_DEBUG, "setPin(%2d,%d)\n", nr, value);
    assert(nr >= 1 && nr <= 25);

    // Only continue if the pin value changes
    if (GET_BIT(port, nr) == value) return;

    // Set the new pin value
    WRITE_BIT(port, nr, value);

    // Perform special actions
    switch (nr) {

        case 3: // RXD

            // Emulate the loopback cable (if connected)
            if (device == SPD_LOOPBACK) {
                setRXD(value);
            }

            // Let the UART know about the change
            uart->rxdHasChanged(value);
            break;

        case 4: // RTS

            // Emulate the loopback cable (if connected)
            if (device == SPD_LOOPBACK) {
                setCTS(value);
                setDSR(value);
            }
            break;
    }
}
