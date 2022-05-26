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
#include "Constants.h"

struct Beam
{
    // Counters for the vertical and horizontal beam position
    isize v;
    isize h;

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << v
        << h;
    }

    Beam() : v(0), h(0) { }
    Beam(isize v, isize h) : v(v), h(h) { }

    bool operator==(const Beam& beam) const
    {
        return v == beam.v && h == beam.h;
    }

    bool operator!=(const Beam& beam) const
    {
        return v != beam.v || h != beam.h;
    }

    Beam operator+(const isize i) const
    {
        assert(usize(i) < HPOS_CNT);

        auto vv = v;
        auto hh = h + i;

        if (hh >= HPOS_CNT) { hh -= HPOS_CNT; vv++; }
        else if (hh < 0)    { hh += HPOS_CNT; vv--; }

        return Beam(vv, hh);
    }

    isize operator-(const Beam& beam) const
    {
        return (v * HPOS_CNT + h) - (beam.v * HPOS_CNT + beam.h);
    }
};
