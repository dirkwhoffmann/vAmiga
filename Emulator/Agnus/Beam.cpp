// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Beam.h"

Beam&
Beam::operator+=(isize i)
{
    assert(i >= 0 && i <= HPOS_CNT_PAL);

    h = h + i;
    if (h >= hCnt()) {

        h -= hCnt();
        v += 1;
        if (lolToggle) lol = !lol;

        if (v >= vCnt()) {

            v = 0;
            if (lofToggle) lof = !lof;
        }
    }

    return *this;
}

Beam
Beam::operator+(const isize i) const
{
    auto result = *this;
    result += i;
    return result;
}

isize
Beam::diff(isize v2, isize h2) const
{
    assert(v2 > v || (v2 == v && h2 >= h));

    isize result = 0;

    auto b = *this;
    while (b.v != v2) {
        b = b + HPOS_CNT_PAL;
        result += HPOS_CNT_PAL;
    }
    result += h2 - b.h;

    assert(result >= 0);
    return result;
}

Beam
Beam::translate(DMACycle diff) const
{
    auto result = *this;

    // Bail out if the cycle has already been passed
    if (diff < 0) {

        result.v = result.h = INT32_MIN;
        return result;
    }

    while (diff >= HPOS_CNT_PAL) {

        diff -= HPOS_CNT_PAL;
        auto newPos = result + HPOS_CNT_PAL;

        // Bail out if the position is in the next frame
        if (newPos < result) {

            result.v = result.h = INT32_MAX;
            return result;
        }

        result = newPos;
    }
    result += diff;

    return result;
}
