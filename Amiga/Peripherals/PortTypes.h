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
    return value >= 0 && value <= SPD_LOOPBACK;
}

typedef enum : long
{
    CPD_NONE,
    CPD_MOUSE,
    CPD_JOYSTICK
}
ControlPortDevice;

inline bool isControlPortDevice(long value) {
    return value >= 0 && value <= CPD_JOYSTICK;
}

typedef enum
{
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,
    JOYSTICK_FIRE,
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
}
JoystickState;

inline bool isJoystickState(long value) {
    return value >= 0 && value <= MOUSE_BUTTON_RIGHT;
}

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
    RELEASE_FIRE,
    PRESS_MOUSE_LEFT,
    PRESS_MOUSE_RIGHT,
    RELEASE_MOUSE_LEFT,
    RELEASE_MOUSE_RIGHT
}
JoystickAction;

inline bool isJoystickAction(long value) {
    return value >= 0 && value <= RELEASE_MOUSE_RIGHT;
}

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
    SerialPortDevice device;
}
SerialPortConfig;

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
