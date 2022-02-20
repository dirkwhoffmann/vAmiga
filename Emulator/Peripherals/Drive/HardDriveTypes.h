// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

//
// Constants
//

#define MAX_HDF_SIZE MB(128)


//
// Structures
//

typedef struct
{
    bool attached;
    bool modified;
    struct { isize c; isize h; isize s; } head;
}
HardDriveInfo;
