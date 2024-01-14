// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"


//
// Enumerations
//

enum_long(CPD)
{
    CPD_NONE,
    CPD_MOUSE,
    CPD_JOYSTICK
};
typedef CPD ControlPortDevice;

#ifdef __cplusplus
struct ControlPortDeviceEnum : util::Reflection<ControlPortDeviceEnum, ControlPortDevice>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = CPD_JOYSTICK;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "CPD"; }
    static const char *key(ControlPortDevice value)
    {
        switch (value) {
                
            case CPD_NONE:      return "NONE";
            case CPD_MOUSE:     return "MOUSE";
            case CPD_JOYSTICK:  return "JOYSTICK";
        }
        return "???";
    }
};
#endif

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
