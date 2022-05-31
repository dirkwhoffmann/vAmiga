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
#include "Reflection.h"

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
    static const char *key(LineType value)
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
        assert(i >= 0 && i <= HPOS_CNT_PAL);

        auto result = *this;
        assert(result.hCnt() == hCnt());

        result.h = h + i;
        if (result.h >= hCnt()) {

            result.v++;
            result.h -= hCnt();

            if (type == LINE_NTSC_LONG) result.type = LINE_NTSC_SHORT;
            if (type == LINE_NTSC_SHORT) result.type = LINE_NTSC_LONG;
        }

        return result;
    }

    isize diff(isize v2, isize h2) const
    {
        assert(v2 > v || (v2 == v && h2 >= h));

        isize result = 0;

        auto b = *this;
        while (b.v < v2) {
            b = b + HPOS_CNT_PAL;
            result += HPOS_CNT_PAL;
        }
        result += h2 - b.h;

        assert(result >= 0);
        return result;
    }
};
