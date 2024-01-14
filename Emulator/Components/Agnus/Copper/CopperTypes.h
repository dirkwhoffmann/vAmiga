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

//
// Structures
//

typedef struct
{
    isize copList;
    u32   copList1Start;
    u32   copList1End;
    u32   copList2Start;
    u32   copList2End;
    bool  active;
    bool  cdang;
    u32   coppc0;
    u32   cop1lc;
    u32   cop2lc;
    u16   cop1ins;
    u16   cop2ins;
}
CopperInfo;
