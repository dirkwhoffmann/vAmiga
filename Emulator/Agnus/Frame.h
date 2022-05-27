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
    isize numLines() const { return lof ? 313 : 312; }
    isize lastLine() const { return lof ? 312 : 311; }
    
    bool wasLongFrame() const { return prevlof; }
    bool wasShortFrame() const { return !prevlof; }
    isize prevNumLines() const { return prevlof ? 313 : 312; }
    isize prevLastLine() const { return prevlof ? 312 : 311; }

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
    Cycle posToCycle(isize v, isize h)
    {
        isize cycles = v * HPOS_CNT_PAL + h;

        switch (type) {

            case LINE_PAL:          break;
            case LINE_NTSC_SHORT:   cycles += v / 2; break;
            case LINE_NTSC_LONG:    cycles += (v + 1) / 2; break;

            default:
                fatalError;
        }

        return start + DMA_CYCLES(cycles);
    }
};
