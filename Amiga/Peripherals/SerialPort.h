// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _SERIAL_PORT_INC
#define _SERIAL_PORT_INC

#include "HardwareComponent.h"

class SerialPort : public HardwareComponent {

    //
    // References (for quickly accessing other components)
    //

    class EventHandler *events;
    class UART *uart;


    //
    // Variables
    //

    // The connected device
    SerialPortDevice device = SPD_LOOPBACK;

    // The current values of the port pins
    uint32_t port;

    
    //
    // Constructing and destructing
    //

public:

    SerialPort();


    //
    // Methods from HardwareComponent
    //

private:

    void _initialize() override;
    void _powerOn() override;
    void _dump() override;


    //
    // Connecting peripherals
    //

public:

    // Connects or disconnects a device
    void connectDevice(SerialPortDevice device);


    //
    // Transmitting and receiving data
    //

private:

    // Reads the current value of a certain port pin
    bool getPin(int nr);

    // Modifies the value of a certain port pin
    void setPin(int nr, bool value);

public:

    bool getTXD() { return getPin(2); }
    bool getRXD() { return getPin(3); }
    bool getRTS() { return getPin(4); }
    bool getCTS() { return getPin(5); }
    bool getDSR() { return getPin(6); }
    bool getCD() { return getPin(8); }
    bool getDTR() { return getPin(20); }

    void setTXD(bool value) { setPin(2, value); }
    void setRXD(bool value);
    void setRTS(bool value) { setPin(4, value); }
    void setCTS(bool value) { setPin(5, value); }
    void setDSR(bool value) { setPin(6, value); }
    void setCD(bool value) { setPin(8, value); }
    void setDTR(bool value) { setPin(20, value); }
};

#endif

