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

/// Configuration options of the video port.
typedef struct
{
    bool whiteNoise;                        ///< Show white noise while the machine is off
}

VideoPortConfig;

/// State of the video port.
typedef struct
{
    i64 latestGrabbedFrame;                 ///< Number of the most recent frame that was grabbed by the host
}
VideoPortInfo;

/// Statistics collected by the video port.
typedef struct
{
    isize droppedFrames;                    ///< Number of frames that were not grabbed in time
}
VideoPortStats;

}
