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
    
    // The value of the frame flipflop in the previous frame
    bool prevlof;

    // The master clock at the beginning of this frame
    Cycle start;

    // The type of the first line in this frame
    LineType type;

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << nr
        << lof
        << prevlof
        << start
        << type;
    }
    
    Frame() : nr(0), lof(false), prevlof(false), start(0), type(LINE_PAL) { }

    bool isLongFrame() const { return lof; }
    bool isShortFrame() const { return !lof; }
    isize numLines() const;
    isize lastLine() const;
    
    bool wasLongFrame() const { return prevlof; }
    bool wasShortFrame() const { return !prevlof; }
    isize prevNumLines() const;
    isize prevLastLine() const;

    // Advances one frame
    void next(bool laceBit, Cycle newStart, LineType newType)
    {
        nr++;
        prevlof = lof;
        start = newStart;
        type = newType;
        
        // Toggle the long frame flipflop in interlace mode
        if (laceBit) { lof = !lof; }
    }

    // Computes the master cycle for a position in the current frame
    Cycle posToCycle(isize v, isize h) const;

    // Computes the number of cycles between two beam positions
    Cycle diff(isize v1, isize h1, isize v2, isize h2) const;
};
