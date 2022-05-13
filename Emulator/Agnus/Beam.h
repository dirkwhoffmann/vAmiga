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
    isize newh; // Issue #681

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << v
        << newh;
    }
    
    Beam(isize v, isize h) : v(v), newh(h) { }
    Beam(isize cycle = 0) : Beam(cycle / HPOS_CNT, cycle % HPOS_CNT) { }

    bool operator==(const Beam& beam) const
    {
        return v == beam.v && newh == beam.newh;
    }

    bool operator!=(const Beam& beam) const
    {
        return v != beam.v || newh != beam.newh;
    }

    Beam& operator+=(const Beam& beam)
    {
        v += beam.v;
        newh += beam.newh;

        if (newh >= HPOS_CNT) { newh -= HPOS_CNT; v++; }
        else if (newh < 0)    { newh += HPOS_CNT; v--; }

        return *this;
    }

    Beam operator+(const Beam& beam) const
    {
        auto vv = v + beam.v;
        auto hh = newh + beam.newh;

        if (hh >= HPOS_CNT) { hh -= HPOS_CNT; vv++; }
        else if (hh < 0)    { hh += HPOS_CNT; vv--; }

        return Beam(vv, hh);
    }

    Beam operator+(const isize i) const
    {
        return *this + Beam(i);
    }

    isize operator-(const Beam& beam) const
    {
        return (v * HPOS_CNT + newh) - (beam.v * HPOS_CNT + beam.newh);
    }

    Beam& operator++()
    {
        if (++newh > HPOS_MAX) { v++; newh = 0; }
        return *this;
    }

    Beam& operator--()
    {
        if (--newh < 0) { v--; newh = HPOS_MAX; }
        return *this;
    }
};
