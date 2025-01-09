// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

namespace vamiga {

//
// Structures
//

typedef struct
{
    // Refresh rate of the host display
    isize refreshRate;
    
    // Audio sample rate of the host computer
    isize sampleRate;
    
    // Framebuffer dimensions
    isize frameBufferWidth;
    isize frameBufferHeight;
}
HostConfig;

}
