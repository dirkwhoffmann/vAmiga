// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _PORT_T_INC
#define _PORT_T_INC

//
// Enumerations
//

typedef enum : long
{
    CPD_NONE,
    CPD_MOUSE,
    CPD_JOYSTICK
}
ControlPortDevice;

inline bool isControlPortDevice(long value) {
    return value >= CPD_NONE && value <= CPD_JOYSTICK;
}

typedef enum : long
{
    SPD_NONE,
    SPD_LOOPBACK
}
SerialPortDevice;

inline bool isSerialPortDevice(long value) {
    return value >= SPD_NONE && value <= SPD_LOOPBACK;
}

#endif
