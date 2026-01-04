// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FileSystems/AmigaFS/FSTypes.h"
#include "DeviceTypes.h"

namespace vamiga {

using retro::vault::device::Diameter;
using retro::vault::device::Density;
using retro::vault::amigafs::BootBlockType;
using retro::vault::amigafs::FSFormat;

//
// Structures
//

typedef struct
{
    FSFormat dos;
    Diameter diameter;
    Density density;
    BootBlockType bootBlockType;
    const char *bootBlockName;
    bool hasVirus;
}
FloppyDiskInfo;

}
