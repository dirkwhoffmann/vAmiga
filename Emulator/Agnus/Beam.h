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

enum_long(LINE_TYPE)
{
    LINE_PAL,           // 227 DMA cycles
    LINE_NTSC_SHORT,    // 227 DMA cycles
    LINE_NTSC_LONG      // 228 DMA cycles
};
typedef LINE_TYPE LineType;

#ifdef __cplusplus
struct LineTypeEnum : util::Reflection<LineTypeEnum, LineType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = LINE_NTSC_LONG;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "LINE"; }
    static const char *key(AgnusRevision value)
    {
        switch (value) {

            case LINE_PAL:          return "PAL";
            case LINE_NTSC_SHORT:   return "NTSC_SHORT";
            case LINE_NTSC_LONG:    return "NTSC_LONG";
        }
        return "???";
    }
};
#endif

struct Beam
{
    // The vertical and horizontal beam position
    isize v;
    isize h;

    // The type of the current line
    LineType type;

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << v
        << h
        << type;
    }

    Beam() : v(0), h(0), type(LINE_PAL) { }
    Beam(isize v, isize h) : v(v), h(h), type(LINE_PAL) { }

    isize hCnt() const { return type == LINE_NTSC_LONG ? 228 : 227; }
    isize hMax() const { return type == LINE_NTSC_LONG ? 227 : 226; }

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
        assert(i >= 0 && i < HPOS_CNT_PAL);

        auto vv = v;
        auto hh = h + i;

        if (hh >= hCnt()) { hh -= hCnt(); vv++; }

        return Beam(vv, hh);
    }

    // Returns the number of elapsed cycles between two beam positions
    isize operator-(const Beam& beam) const
    {
        assert(v >= beam.v);
        assert(v != beam.v || h >= beam.h);

        isize count1 = v * HPOS_CNT_PAL + h + ll();
        isize count2 = beam.v * HPOS_CNT_PAL + beam.h + beam.ll();

        if (type == LINE_PAL) {
            assert(count1 - count2 ==
                   (v * HPOS_CNT_PAL + h) - (beam.v * HPOS_CNT_PAL + beam.h));
        }

        return count1 - count2;
    }

    // Returns the number of long lines above the current vertical position
    isize ll() const {

        switch (type) {

            case LINE_PAL:          return 0;
            case LINE_NTSC_SHORT:   return IS_EVEN(v) ? (v / 2) : (v + 1) / 2;
            case LINE_NTSC_LONG:    return IS_EVEN(v) ? (v + 1) / 2 : (v / 2);

            default:
                fatalError;
        }
    }
};

/* REMOVE ASAP:
0 L 0 (v + 1) / 2
1 S 1
2 L 1
3 S 2
4 L 2
5 S 3

0 S 0 (v / 2)
1 L 0
2 S 1
3 L 1
4 S 2
5 L 2
*/
