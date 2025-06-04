// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class HIDEvent
{
    AXIS,
    BUTTON,
    DPAD_UP,
    DPAD_DOWN,
    DPAD_RIGHT,
    DPAD_LEFT,
    HATSWITCH,
};

enum class GamePadAction : long
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

struct GamePadActionEnum : Reflection<GamePadActionEnum, GamePadAction>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(GamePadAction::RELEASE_RIGHT);
    
    static const char *_key(GamePadAction value)
    {
        switch (value) {
                
            case GamePadAction::PULL_UP:        return "PULL_UP";
            case GamePadAction::PULL_DOWN:      return "PULL_DOWN";
            case GamePadAction::PULL_LEFT:      return "PULL_LEFT";
            case GamePadAction::PULL_RIGHT:     return "PULL_RIGHT";
            case GamePadAction::PRESS_FIRE:     return "PRESS_FIRE";
            case GamePadAction::PRESS_FIRE2:    return "PRESS_FIRE2";
            case GamePadAction::PRESS_FIRE3:    return "PRESS_FIRE3";
            case GamePadAction::PRESS_LEFT:     return "PRESS_LEFT";
            case GamePadAction::PRESS_MIDDLE:   return "PRESS_MIDDLE";
            case GamePadAction::PRESS_RIGHT:    return "PRESS_RIGHT";
            case GamePadAction::RELEASE_X:      return "RELEASE_X";
            case GamePadAction::RELEASE_Y:      return "RELEASE_Y";
            case GamePadAction::RELEASE_XY:     return "RELEASE_XY";
            case GamePadAction::RELEASE_FIRE:   return "RELEASE_FIRE";
            case GamePadAction::RELEASE_FIRE2:  return "RELEASE_FIRE2";
            case GamePadAction::RELEASE_FIRE3:  return "RELEASE_FIRE3";
            case GamePadAction::RELEASE_LEFT:   return "RELEASE_LEFT";
            case GamePadAction::RELEASE_MIDDLE: return "RELEASE_MIDDLE";
            case GamePadAction::RELEASE_RIGHT:  return "RELEASE_RIGHT";
        }
        return "???";
    }
    static const char *help(GamePadAction value)
    {
        return "";
    }
};


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

}
