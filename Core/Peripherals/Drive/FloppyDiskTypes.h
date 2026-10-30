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

#include "FileSystems/Amiga/FSTypes.h"
#include "DeviceTypes.h"

namespace vamiga {

using retro::vault::Diameter;
using retro::vault::Density;


//
// Structures
//

/// Properties of a floppy disk.
typedef struct
{
    retro::vault::amiga::FSFormat dos;      ///< File system format
    retro::vault::Diameter diameter;        ///< Disk diameter
    retro::vault::Density density;          ///< Disk density
    retro::vault::amiga::BootBlockType bootBlockType;///< Type of the boot block
    const char *bootBlockName;              ///< Name of the boot block
    bool hasVirus;                          ///< The boot block contains a known virus
}
FloppyDiskInfo;

}
