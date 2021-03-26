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

enum_i8(BUS_OWNER)
{
    BUS_NONE,
    BUS_CPU,
    BUS_REFRESH,
    BUS_DISK,
    BUS_AUDIO,
    BUS_BPL1,
    BUS_BPL2,
    BUS_BPL3,
    BUS_BPL4,
    BUS_BPL5,
    BUS_BPL6,
    BUS_SPRITE0,
    BUS_SPRITE1,
    BUS_SPRITE2,
    BUS_SPRITE3,
    BUS_SPRITE4,
    BUS_SPRITE5,
    BUS_SPRITE6,
    BUS_SPRITE7,
    BUS_COPPER,
    BUS_BLITTER,
    BUS_COUNT
};
typedef BUS_OWNER BusOwner;
