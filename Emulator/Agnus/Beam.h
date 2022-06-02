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

enum_long(FRAME_TYPE)
{
    FRAME_PAL_LF,       // PAL long frame
    FRAME_PAL_SF,       // PAL short frame
    FRAME_NTSC_LF_LL,   // NTSC long frame starting with a long line
    FRAME_NTSC_LF_SL,   // NTSC long frame starting with a short line
    FRAME_NTSC_SF_LL,   // NTSC short frame starting with a long line
    FRAME_NTSC_SF_SL    // NTSC short frame starting with a short line
};
typedef FRAME_TYPE FrameType;

#ifdef __cplusplus
struct FrameTypeEnum : util::Reflection<FrameTypeEnum, FrameType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FRAME_NTSC_SF_SL;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "FRAME"; }
    static const char *key(FrameType value)
    {
        switch (value) {

            case FRAME_PAL_LF:      return "PAL_LF";
            case FRAME_PAL_SF:      return "PAL_SF";
            case FRAME_NTSC_LF_LL:  return "NTSC_LF_LL";
            case FRAME_NTSC_LF_SL:  return "NTSC_LF_SL";
            case FRAME_NTSC_SF_LL:  return "NTSC_SF_LL";
            case FRAME_NTSC_SF_SL:  return "NTSC_SF_SL";
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
    VideoFormat type;

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
    isize vCnt() const { return type == PAL ? vCntPal() : vCntNtsc(); }
    isize vMax() const { return type == PAL ? vMaxPal() : vMaxNtsc(); }
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
    Beam translate(Cycle diff) const;

    // Predicts the type of the current frame
    FrameType predictFrameType() const;

    // Predicts the type of the next frame
    static isize predictNextFrameType(FrameType type, bool toggle);
    FrameType predictNextFrameType() const {
        return predictNextFrameType(predictFrameType(), lofToggle); }

    // Returns the number of DMA cycles executed in a single frame
    static isize cyclesPerFrame(FrameType type);
    isize cyclesPerFrame() const { return cyclesPerFrame(type); }

    // Returns the number of DMA cycles executed in two consecutive frames
    static isize cyclesPerFramePair(FrameType type, bool toggle);
    isize cyclesPerFramePair() const { return cyclesPerFramePair(type, lofToggle); }

};
