// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(GAME_PAD_ACTION)
{
    PULL_UP = 0,    // Pull the joystick up
    PULL_DOWN,      // Pull the joystick down
    PULL_LEFT,      // Pull the joystick left
    PULL_RIGHT,     // Pull the joystick right
    PRESS_FIRE,     // Press the first joystick button
    PRESS_FIRE2,    // Press the second joystick button
    PRESS_FIRE3,    // Press the third joystick button
    PRESS_LEFT,     // Press the left mouse button
    PRESS_MIDDLE,   // Press the middle mouse button
    PRESS_RIGHT,    // Press the right mouse button
    RELEASE_X,      // Move back to neutral horizontally
    RELEASE_Y,      // Move back to neutral vertically
    RELEASE_XY,     // Move back to neutral
    RELEASE_FIRE,   // Release the first joystick button
    RELEASE_FIRE2,  // Release the second joystick button
    RELEASE_FIRE3,  // Release the third joystick button
    RELEASE_LEFT,   // Release the left mouse button
    RELEASE_MIDDLE, // Release the middle mouse button
    RELEASE_RIGHT   // Release the right mouse button
};
typedef GAME_PAD_ACTION GamePadAction;

#ifdef __cplusplus
struct GamePadActionEnum : vamiga::util::Reflection<GamePadActionEnum, GamePadAction>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = RELEASE_RIGHT;
    
    static const char *prefix() { return nullptr; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case PULL_UP:        return "PULL_UP";
            case PULL_DOWN:      return "PULL_DOWN";
            case PULL_LEFT:      return "PULL_LEFT";
            case PULL_RIGHT:     return "PULL_RIGHT";
            case PRESS_FIRE:     return "PRESS_FIRE";
            case PRESS_FIRE2:    return "PRESS_FIRE2";
            case PRESS_FIRE3:    return "PRESS_FIRE3";
            case PRESS_LEFT:     return "PRESS_LEFT";
            case PRESS_RIGHT:    return "PRESS_RIGHT";
            case RELEASE_X:      return "RELEASE_X";
            case RELEASE_Y:      return "RELEASE_Y";
            case RELEASE_XY:     return "RELEASE_XY";
            case RELEASE_FIRE:   return "RELEASE_FIRE";
            case RELEASE_FIRE2:  return "RELEASE_FIRE2";
            case RELEASE_FIRE3:  return "RELEASE_FIRE3";
            case RELEASE_LEFT:   return "RELEASE_LEFT";
            case RELEASE_RIGHT:  return "RELEASE_RIGHT";
        }
        return "???";
    }
};
#endif


enum_long(HID_ACTION)
{
    HID_A0,
    HID_A0_REV,
    HID_A1,
    HID_A1_REV,
    HID_A12,
    HID_A2,
    HID_A2_REV,
    HID_A20,
    HID_A3,
    HID_A3_REV,
    HID_A31,
    HID_A4,
    HID_A5,
    HID_A5_REV,
    HID_A6,
    HID_A6_REV,
    HID_A7,
    HID_A8,
    HID_B0,
    HID_B1,
    HID_B10,
    HID_B11,
    HID_B12,
    HID_B13,
    HID_B14,
    HID_B15,
    HID_B16,
    HID_B17,
    HID_B19,
    HID_B2,
    HID_B20,
    HID_B21,
    HID_B22,
    HID_B23,
    HID_B24,
    HID_B25,
    HID_B26,
    HID_B27,
    HID_B3,
    HID_B4,
    HID_B5,
    HID_B6,
    HID_B7,
    HID_B8,
    HID_B9
};
typedef HID_ACTION HIDAction;


//
// Structures
//

typedef struct
{
    bool autofire;
    bool autofireBursts;
    isize autofireBullets;
    isize autofireDelay;
}
JoystickConfig;

typedef struct
{
    bool button;
    isize axisX;
    isize axisY;
}
JoystickInfo;

typedef struct {

    const char *name;

    int vendorID;
    int productID;
    int version;

    HID_ACTION leftx;
    HID_ACTION rightx;
    HID_ACTION lefty;
    HID_ACTION righty;
    HID_ACTION button1;
    HID_ACTION button2;
}
GamePadTraits;
