// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/Reflection.h"

namespace vamiga {

typedef struct
{
    u16 serper;
    isize baudRate;
    u16 receiveBuffer;
    u16 receiveShiftReg;
    u16 transmitBuffer;
    u16 transmitShiftReg;
}
UARTInfo;

}
