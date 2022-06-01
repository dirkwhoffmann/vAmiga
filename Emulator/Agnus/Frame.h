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
#include "Macros.h"
#include "Beam.h"

struct Frame
{
    // Frame count
    i64 nr;

    // The type of the first line in this frame
    // [[deprecated]] LineType type;

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << nr;
    }
    
    Frame() : nr(0) { }

    // Advances one frame
    void next()
    {
        nr++;
    }
};
