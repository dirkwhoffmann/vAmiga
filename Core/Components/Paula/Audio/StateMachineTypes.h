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
