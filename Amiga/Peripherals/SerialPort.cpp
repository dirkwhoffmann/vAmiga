// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

SerialPort::SerialPort(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("SerialPort");

    config.device = SPD_LOOPBACK;
}

void
SerialPort::setDevice(SerialPortDevice device)
{
    assert(isSerialPortDevice(device));

    config.device = device;
}

void
SerialPort::_reset()
{
   RESET_SNAPSHOT_ITEMS
   // port = 0x1FFFFFE;
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
    msg("    device: %d\n", config.device);
    msg("      port: %X\n", port);
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

bool
SerialPort::getPin(int nr)
{
    assert(nr >= 1 && nr <= 25);

    bool result = GET_BIT(port, nr);

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
    REPLACE_BIT(port, nr, value);

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
SerialPort::setPort(u32 mask, bool value)
{
    u32 oldPort = port;

    /* Emulate the loopback cable (if connected)
     *
     *     Connected pins: A: 2 - 3       (TXD - RXD)
     *                     B: 4 - 5 - 6   (RTS - CTS - DSR)
     *                     C: 8 - 20 - 22 (CD - DTR - RI)
     */
    if (config.device == SPD_LOOPBACK) {

        u32 maskA = TXD_MASK | RXD_MASK;
        u32 maskB = RTS_MASK | CTS_MASK | DSR_MASK;
        u32 maskC = CD_MASK | DTR_MASK | RI_MASK;

        if (mask & maskA) mask |= maskA;
        if (mask & maskB) mask |= maskB;
        if (mask & maskC) mask |= maskC;
    }

    // Change the port pins
    if (value) port |= mask; else port &= ~mask;

    // Let the UART know if RXD has changed
    if ((oldPort ^ port) & RXD_MASK) uart.rxdHasChanged(value);
}

