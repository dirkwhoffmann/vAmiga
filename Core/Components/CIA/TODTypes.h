// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {
//
// Structures
//

/// State of a time-of-day counter.
typedef struct
{
    u32 value;                              ///< Current counter value
    u32 latch;                              ///< Latched counter value
    u32 alarm;                              ///< Alarm value
}
TODInfo;

}
