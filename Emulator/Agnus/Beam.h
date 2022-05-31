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
    LINE_NTSC           // 227 or 228 DMA cycles
};
typedef LINE_TYPE LineType;

#ifdef __cplusplus
struct LineTypeEnum : util::Reflection<LineTypeEnum, LineType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = LINE_NTSC;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "LINE"; }
    static const char *key(LineType value)
    {
        switch (value) {

            case LINE_PAL:  return "PAL";
            case LINE_NTSC: return "NTSC";
        }
        return "???";
    }
};
#endif

struct Beam
{
    // The vertical and horizontal beam position
    isize v = 0;
    isize h = 0;

    // Long frame flipflop
    bool lof = false;
    bool lofToggle = false;

    // Long line fliflop
    bool lol = false;
    bool lolToggle = false;

    // The type of the current line
    LineType type;

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << v
        << h
        << lof
        << lofToggle
        << lol
        << lolToggle
        << type;
    }

    isize hCnt() const { return lol ? 228 : 227; }
    isize hMax() const { return lol ? 227 : 226; }
    isize vCnt() const { return type == LINE_PAL ? vCntPal() : vCntNtsc(); }
    isize vMax() const { return type == LINE_PAL ? vMaxPal() : vMaxNtsc(); }
    isize vMaxPal() const { return lof ? 312 : 311; }
    isize vMaxNtsc() const { return lof ? 262 : 261; }
    isize vCntPal() const { return lof ? 313 : 312; }
    isize vCntNtsc() const { return lof ? 263 : 262; }

    bool operator==(const Beam& beam) const
    {
        return v == beam.v && h == beam.h;
    }

    bool operator!=(const Beam& beam) const
    {
        return v != beam.v || h != beam.h;
    }

    bool operator>(const Beam& beam) const
    {
        return v > beam.v || (v == beam.v && h > beam.h);
    }

    bool operator<(const Beam& beam) const
    {
        return v < beam.v || (v == beam.v && h < beam.h);
    }

    bool operator>=(const Beam& beam) const
    {
        return *this == beam || *this > beam;
    }

    bool operator<=(const Beam& beam) const
    {
        return *this == beam || *this < beam;
    }

    Beam& operator+=(isize i)
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

    Beam operator+(const isize i) const
    {
        auto result = *this;

        result += i;

        return result;
        /*
        assert(i >= 0 && i <= HPOS_CNT_PAL);

        auto result = *this;

        result.h = h + i;
        if (result.h >= hCnt()) {

            result.h -= hCnt();
            result.v += 1;
            if (lolToggle) result.lol = !result.lol;

            if (result.v >= vCnt()) {

                result.v = 0;
                if (lofToggle) result.lof = !result.lof;
            }
        }

        return result;
        */
    }

    isize diff(isize v2, isize h2) const
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
};
