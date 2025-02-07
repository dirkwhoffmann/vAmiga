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
// Structures
//

typedef struct
{
    bool pullUpResistors;
    bool shakeDetection;
    isize velocity;
}
MouseConfig;

}
