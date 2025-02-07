// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga/Foundation/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class ControlPortDevice
{
    NONE,
    MOUSE,
    JOYSTICK
};

struct ControlPortDeviceEnum : Reflection<ControlPortDeviceEnum, ControlPortDevice>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(ControlPortDevice::JOYSTICK);
    
    static const char *_key(ControlPortDevice value)
    {
        switch (value) {
                
            case ControlPortDevice::NONE:      return "NONE";
            case ControlPortDevice::MOUSE:     return "MOUSE";
            case ControlPortDevice::JOYSTICK:  return "JOYSTICK";
        }
        return "???";
    }
    static const char *help(ControlPortDevice value)
    {
        switch (value) {
                
            case ControlPortDevice::NONE:      return "No device";
            case ControlPortDevice::MOUSE:     return "Amiga Mouse";
            case ControlPortDevice::JOYSTICK:  return "Joystick";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    bool m0v;
    bool m0h;
    bool m1v;
    bool m1h;
    u16 joydat;
    u16 potgo;
    u16 potgor;
    u16 potdat;
}
ControlPortInfo;

}
