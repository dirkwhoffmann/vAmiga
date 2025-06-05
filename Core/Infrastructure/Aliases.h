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

//
// Type aliases
//

namespace vamiga {

typedef i64 Cycle;            // Master cycle units
typedef i64 CPUCycle;         // CPU cycle units
typedef i64 CIACycle;         // CIA cycle units
typedef i64 DMACycle;         // DMA cycle units

// Pixels
typedef isize Pixel;

// Block devices
typedef isize Cylinder;
typedef isize Head;
typedef isize Sector;
typedef isize Track;

// Keyboard
typedef u8 KeyCode;

// File Systems
typedef u32 Block;

}
