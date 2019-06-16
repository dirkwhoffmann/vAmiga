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
    SPD_NONE,
    SPD_LOOPBACK
}
SerialPortDevice;

inline bool isSerialPortDevice(long value) {
    return value >= SPD_NONE && value <= SPD_LOOPBACK;
}

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

typedef enum
{
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,
    JOYSTICK_FIRE
}
JoystickDirection;

typedef enum
{
    PULL_UP,
    PULL_DOWN,
    PULL_LEFT,
    PULL_RIGHT,
    PRESS_FIRE,
    RELEASE_X,
    RELEASE_Y,
    RELEASE_XY,
    RELEASE_FIRE
}
JoystickEvent;

//
// Structures
//

typedef struct
{
    bool m0v;
    bool m0h;
    bool m1v;
    bool m1h;
    uint16_t potx;
    uint16_t poty;
}
ControlPortInfo;

typedef struct
{
    uint32_t port;

    bool txd;
    bool rxd;
    bool rts;
    bool cts;
    bool dsr;
    bool cd;
    bool dtr;
}
SerialPortInfo;

#endif
