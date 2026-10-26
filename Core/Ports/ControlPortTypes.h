// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"

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

struct ControlPortDeviceEnum : utl::Reflectable<ControlPortDeviceEnum, ControlPortDevice>
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
    bool m0v;                               ///< Mouse quadrature signal V0
    bool m0h;                               ///< Mouse quadrature signal H0
    bool m1v;                               ///< Mouse quadrature signal V1
    bool m1h;                               ///< Mouse quadrature signal H1
    u16 joydat;                             ///< Joystick/mouse data register (JOYxDAT)
    u16 potgo;                              ///< Potentiometer control register (POTGO)
    u16 potgor;                             ///< Potentiometer port data read register (POTGOR)
    u16 potdat;                             ///< Potentiometer counter data register (POTxDAT)
}
ControlPortInfo;

}
