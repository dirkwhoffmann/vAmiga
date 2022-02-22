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
// Structures
//

typedef struct
{
    Side side;
    Cylinder cylinder;
    Sector sector;
}
HardDriveHead;

typedef struct
{
    bool connected;
}
HardDriveConfig;

typedef struct
{
    bool attached;
    bool modified;
    struct { isize c; isize h; isize s; } head;
}
HardDriveInfo;
