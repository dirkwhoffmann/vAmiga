// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Roms/RomManager.h"

namespace vamiga {

/* The Rom vocabulary -- RomTraits, RomType, RomVendor and the CRC32 constants
 * naming every Amiga Rom the database knows -- is shared by all cores and lives
 * in rvlib's RomManager. It used to be defined here.
 */
using namespace retro::vault;

}
