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
    bool pullUpResistors;                   ///< Emulate pull-up resistors
    bool shakeDetection;                    ///< Detect a shaken mouse
    isize velocity;                         ///< Mouse velocity
}
MouseConfig;

}
