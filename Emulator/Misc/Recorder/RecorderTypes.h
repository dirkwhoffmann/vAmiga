// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

namespace vamiga {

//
// Enumerations
//



//
// Structures
//

typedef struct
{
    // Video paramters
    isize frameRate;
    isize bitRate;
    isize sampleRate;
    
    // Recorded texture area
    struct { isize x1; isize y1; isize x2; isize y2; } cutout;
}
RecorderConfig;

}
