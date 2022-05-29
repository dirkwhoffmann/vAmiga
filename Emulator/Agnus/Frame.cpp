// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Frame.h"

isize
Frame::numLines() const
{
    switch (type) {

        case LINE_PAL:  return lof ? 313 : 312;
        default:        return lof ? 263 : 262;
    }
}

isize
Frame::lastLine() const
{
    switch (type) {

        case LINE_PAL:  return lof ? 312 : 311;
        default:        return lof ? 262 : 261;
    }
}

isize
Frame::prevNumLines() const
{
    switch (type) {

        case LINE_PAL:  return prevlof ? 313 : 312;
        default:        return prevlof ? 263 : 262;
    }
}

isize
Frame::prevLastLine() const
{
    switch (type) {

        case LINE_PAL:  return prevlof ? 312 : 311;
        default:        return prevlof ? 262 : 261;
    }
}

Cycle
Frame::posToCycle(isize v, isize h) const
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

Cycle
Frame::diff(isize v1, isize h1, isize v2, isize h2) const
{
    assert(v1 >= v2);
    assert(v1 != v2|| h1 >= h2);

    auto count1 = posToCycle(v1, h1);
    auto count2 = posToCycle(v2, h2);
    assert(count1 >= count2);

    if (type == LINE_PAL) {
        assert(count1 - count2 ==
               DMA_CYCLES((v1 * HPOS_CNT_PAL + h1) - (v2 * HPOS_CNT_PAL + h2)));
    }

    return count1 - count2;
}
