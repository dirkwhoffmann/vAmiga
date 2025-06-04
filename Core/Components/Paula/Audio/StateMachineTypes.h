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

//
// Structures
//

typedef struct
{
    isize state;
    bool dma;
    u16 audlenLatch;
    u16 audlen;
    u16 audperLatch;
    i32 audper;
    u16 audvolLatch;
    u16 audvol;
    u16 auddat;
}
StateMachineInfo;

}
