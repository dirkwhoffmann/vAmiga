// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/types.h"

namespace vamiga {

using namespace utl;

typedef struct
{
    isize partitions;
    isize drivers;
    bool hasRDB;
}
HDFInfo;

}
