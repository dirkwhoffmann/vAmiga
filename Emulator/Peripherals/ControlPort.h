// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

//
// Enumerations
//

enum_long(PortNr)
{
    PORT_1 = 1,
    PORT_2 = 2
};

enum_long(CPD)
{
    CPD_NONE,
    CPD_MOUSE,
    CPD_JOYSTICK,
    
    CPD_COUNT
};
typedef CPD ControlPortDevice;

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
