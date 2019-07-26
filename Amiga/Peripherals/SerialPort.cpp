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
    uart = &amiga->paula.uart;
}

void
SerialPort::_powerOn()
{
    port = 0x1FFFFFE; 
}

void
SerialPort::_inspect()
{
    pthread_mutex_lock(&lock);

    info.port = port; 
    info.txd = getTXD();
    info.rxd = getRXD();
    info.rts = getRTS();
    info.cts = getCTS();
    info.dsr = getDSR();
    info.cd = getCD();
    info.dtr = getDTR();

    pthread_mutex_unlock(&lock);
}

void
SerialPort::_dump()
{
    plainmsg("    device: %d\n", device);
    plainmsg("      port: %X\n", port);
}

size_t
SerialPort::_save(uint8_t *buffer)
{
    SerWriter writer(buffer);

    applyToPersistentItems(writer);
    applyToResetItems(writer);

    debug(SNAP_DEBUG, "Serialized to %d bytes\n", writer.ptr - buffer);
    return writer.ptr - buffer;
}

SerialPortInfo
SerialPort::getInfo()
{
    SerialPortInfo result;

    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);

    return result;
}

void
SerialPort::connectDevice(SerialPortDevice device)
{
    if (isSerialPortDevice(device)) {
        debug("connectDevice(%d)\n");
        this->device = device;
    } else {
        assert(false);
    }

}

bool
SerialPort::getPin(int nr)
{
    assert(nr >= 1 && nr <= 25);

    bool result = !!GET_BIT(port, nr);

    debug(2, "getPin(%d) = %d port = %X\n", nr, result, port);
    return result;
}

void
SerialPort::setPin(int nr, bool value)
{
    debug(2, "setPin(%d,%d)\n", nr, value);
    assert(nr >= 1 && nr <= 25);

    setPort(1 << nr, value);

    /*
    if (GET_BIT(port, nr) != value) {
        setPort(1 << nr, value);
    }
    */

    /*
    // Set the new pin value
    WRITE_BIT(port, nr, value);

    // Perform special actions
    switch (nr) {

        case 2: // TXD

            // Emulate the loopback cable (if connected)
            if (device == SPD_LOOPBACK) {
                setRXD(value);
            }
            break;

        case 3: // RXD

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

        case 20: // DTR

            // Emulate the loopback cable (if connected)
            if (device == SPD_LOOPBACK) {
                setCD(value);
            }
            break;
    }
    */
}

void
SerialPort::setPort(uint32_t mask, bool value)
{
    uint32_t oldPort = port;

    /* Emulate a loopback cable if connected
     *
     *     Connected pins: A: 2 - 3       (TXD - RXD)
     *                     B: 4 - 5 - 6   (RTS - CTS - DSR)
     *                     C: 8 - 20 - 22 (CD - DTR - RI)
     */
    if (device == SPD_LOOPBACK) {

        uint32_t maskA = TXD_MASK | RXD_MASK;
        uint32_t maskB = RTS_MASK | CTS_MASK | DSR_MASK;
        uint32_t maskC = CD_MASK | DTR_MASK | RI_MASK;

        if (mask & maskA) mask |= maskA;
        if (mask & maskB) mask |= maskB;
        if (mask & maskC) mask |= maskC;
    }

    // Change the port pins
    if (value) port |= mask; else port &= ~mask;

    // Let the UART know if RXD has changed
    if ((oldPort ^ port) & RXD_MASK) uart->rxdHasChanged(value);
}

