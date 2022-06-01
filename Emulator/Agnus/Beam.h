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
#include "AmigaTypes.h"

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

    // The frame count
    i64 frame = 0;

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
        << frame
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

    isize numLines() const { return vCnt(); }
    isize lastLine() const { return vMax(); }

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

    Beam& operator+=(isize i);
    Beam operator+(const isize i) const;

    isize diff(isize v2, isize h2) const;

    /* Translates a cycle delta (DMA cycle relative to the current position)
     * to a beam position. The function only returns a precise position if
     * the result is located between the current position and the frame end.
     * If a negative delta is provided, location (INT32_MIN, INT32_MIN) is
     * returned. If the current position plus the provided delta is a beam
     * position in the next frame, location (INT32_MAX, INT32_MAX) is returned.
     */
    Beam translate(isize diff) const;
};
