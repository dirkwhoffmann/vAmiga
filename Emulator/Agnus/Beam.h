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
    
    Beam(isize v, isize h) : v(v), h(h) { }
    Beam(isize cycle = 0) : Beam(cycle / HPOS_CNT, cycle % HPOS_CNT) { }

    bool operator==(const Beam& beam) const
    {
        return v == beam.v && h == beam.h;
    }

    bool operator!=(const Beam& beam) const
    {
        return v != beam.v || h != beam.h;
    }

    Beam& operator+=(const Beam& beam)
    {
        v += beam.v;
        h += beam.h;

        if (h >= HPOS_CNT) { h -= HPOS_CNT; v++; }
        else if (h < 0)    { h += HPOS_CNT; v--; }

        return *this;
    }

    Beam operator+(const Beam& beam) const
    {
        auto newv = v + beam.v;
        auto newh = h + beam.h;

        if (newh >= HPOS_CNT) { newh -= HPOS_CNT; newv++; }
        else if (newh < 0)    { newh += HPOS_CNT; newv--; }

        return Beam(newv, newh);
    }

    Beam operator+(const isize i) const
    {
        return *this + Beam(i);
    }

    isize operator-(const Beam& beam) const
    {
        return (v * HPOS_CNT + h) - (beam.v * HPOS_CNT + beam.h);
    }

    Beam& operator++()
    {
        if (++h > HPOS_MAX) { v++; h = 0; }
        return *this;
    }

    Beam& operator--()
    {
        if (--h < 0) { v--; h = HPOS_MAX; }
        return *this;
    }
};
