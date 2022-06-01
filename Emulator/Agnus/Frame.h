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
    
    // The long frame flipflop
    bool lof;

    // The master clock at the beginning of this frame
    Cycle start;

    // The type of the first line in this frame
    LineType type;

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << nr
        // << lof
        << start
        << type;
    }
    
    Frame() : nr(0), lof(false), start(0), type(LINE_PAL) { }

    [[deprecated]] bool isLongFrame() const { return lof; }
    // [[deprecated]] bool isShortFrame() const { return !lof; }
    [[deprecated]] isize numLines() const;
    [[deprecated]] isize lastLine() const;

    // Advances one frame
    void next(bool laceBit, Cycle newStart, LineType newType)
    {
        nr++;
        start = newStart;
        type = newType;
        
        // Toggle the long frame flipflop in interlace mode
        if (laceBit) { lof = !lof; }
    }
};
