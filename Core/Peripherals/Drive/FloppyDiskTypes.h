// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystems/Amiga/FSTypes.h"
#include "DeviceTypes.h"

namespace vamiga {

using retro::vault::Diameter;
using retro::vault::Density;


//
// Structures
//

typedef struct
{
    retro::vault::amiga::FSFormat dos;
    retro::vault::Diameter diameter;
    retro::vault::Density density;
    retro::vault::amiga::BootBlockType bootBlockType;
    const char *bootBlockName;
    bool hasVirus;
}
FloppyDiskInfo;

}
