// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Beam.h"

namespace vamiga {

Beam&
Beam::operator+=(isize i)
{
    if (i < 0) { *this -= -i; return *this; }

    // Jump near the target frame
    auto cycles = cyclesPerFrames(4);
    frame += (i / cycles) * 4;
    i %= cycles;

    while (i > 0) {

        cycles = i < PAL::HPOS_MAX ? i : PAL::HPOS_MAX;
        i -= cycles;
        h += cycles;

        if (h >= hCnt()) {

            h -= hCnt();
            if (lolToggle) lol = !lol;

            if (++v >= vCnt()) {

                frame++;
                if (lofToggle) lof = !lof;
                v = 0;
            }
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

Beam&
Beam::operator-=(isize i)
{
    if (i < 0) { *this += -i; return *this; }

    // Jump near the target frame
    auto cycles = cyclesPerFrames(4);
    frame -= (i / cycles) * 4;
    i %= cycles;

    while (i > 0) {

        cycles = i < PAL::HPOS_MAX ? i : PAL::HPOS_MAX;
        i -= cycles;
        h -= cycles;

        if (h < 0) {

            if (lolToggle) lol = !lol;
            h += hCnt();

            if (--v < 0) {

                frame--;
                if (lofToggle) lof = !lof;
                v = vCnt();
            }
        }
    }

    return *this;
}

Beam
Beam::operator-(const isize i) const
{
    auto result = *this;
    result -= i;
    return result;
}

isize
Beam::diff(isize v2, isize h2) const
{
    isize result = 0;
    isize threshold = 2 * cyclesPerFrame(FrameType::PAL_LF);

    auto b = *this;

    // If (v2,h2) is in the same line, but already passed, make sure to wrap
    // over to the next frame.
    if (b.v == v2 && h2 < b.h) {

        b = b + PAL::HPOS_CNT;
        result += PAL::HPOS_CNT;
    }

    while (b.v != v2) {

        b = b + PAL::HPOS_CNT;
        result += PAL::HPOS_CNT;

        if (result > threshold) {

            // The specified beam position will never match
            return -1;
        }
    }
    result += h2 - b.h;

    assert(result >= 0);
    return result;
}

FrameType
Beam::predictFrameType() const
{
    // PAL
    if (type == TV::PAL) {
        return lof ? FrameType::PAL_LF : FrameType::PAL_SF;
    }

    // NTSC
    if (IS_EVEN(v) && lol) {
        return lof ? FrameType::NTSC_LF_LL : FrameType::NTSC_SF_LL;
    } else {
        return lof ? FrameType::NTSC_LF_SL : FrameType::NTSC_SF_SL;
    }
}

FrameType
Beam::predictNextFrameType(FrameType type, bool toggle)
{
    switch (type) {

        case FrameType::PAL_LF:
            return toggle ? FrameType::PAL_SF : FrameType::PAL_LF;

        case FrameType::PAL_SF:
            return toggle ? FrameType::PAL_LF : FrameType::PAL_SF;

        case FrameType::NTSC_LF_LL:
            return toggle ? FrameType::NTSC_SF_SL : FrameType::NTSC_LF_SL;

        case FrameType::NTSC_LF_SL:
            return toggle ? FrameType::NTSC_SF_LL : FrameType::NTSC_LF_LL;

        case FrameType::NTSC_SF_LL:
            return toggle ? FrameType::NTSC_LF_LL : FrameType::NTSC_SF_LL;

        case FrameType::NTSC_SF_SL:
            return toggle ? FrameType::NTSC_LF_SL : FrameType::NTSC_SF_SL;

        default:
            fatalError;
    }
}

FrameType
Beam::predictNextFrameType() const
{
    return predictNextFrameType(predictFrameType(), lofToggle);
}

isize
Beam::cyclesPerFrame(FrameType type)
{
    switch (type) {

        case FrameType::PAL_LF:
            return PAL::VPOS_CNT_LF * PAL::HPOS_CNT;

        case FrameType::PAL_SF:
            return PAL::VPOS_CNT_SF * PAL::HPOS_CNT;

        case FrameType::NTSC_LF_LL:
            return 132 * NTSC::HPOS_CNT_LL + 131 * NTSC::HPOS_CNT_SL;

        case FrameType::NTSC_LF_SL:
            return 132 * NTSC::HPOS_CNT_SL + 131 * NTSC::HPOS_CNT_LL;

        case FrameType::NTSC_SF_LL:
        case FrameType::NTSC_SF_SL:
            return 131 * NTSC::HPOS_CNT_SL + 131 * NTSC::HPOS_CNT_LL;

        default:
            fatalError;
    }
}

isize
Beam::cyclesPerFrame() const
{
    return cyclesPerFrame(predictFrameType());
}

isize
Beam::cyclesPerFrames(isize count, FrameType type, bool toggle)
{
    isize result = 0;

    for (isize i = 0; i < count; i++) {

        result += cyclesPerFrame(type);
        type = predictNextFrameType(type, toggle);
    }

    return result;
}

isize
Beam::cyclesPerFrames(isize count) const
{
    return cyclesPerFrames(count, predictFrameType(), lofToggle);
}

Pixel
Beam::pixel(isize hpos) const
{
    if (hpos >= HBLANK_MIN) {

        // Every texture line starts with the HBLANK area
        return 4 * (hpos - HBLANK_MIN);
        
    } else {

        // Everything left to the HBLANK area belongs to the previous line
        return 4 * (hpos - HBLANK_MIN + hLatched);
    }
}

void
Beam::eol()
{
    // Remember and reset the horizontal coordinate
    hLatched = h;

    // Advance to the next line
    h = 0;
    if (++v > vMax()) eof();

    // Toggle the line length if toggling is enabled
    if (lolToggle) lol = !lol;
}

void
Beam::eof()
{
    // Remember and reset the vertical coordinate
    vLatched = v;

    // Advance to the next frame
    v = 0;
    frame++;

    // Toggle the frame length if toggling is enabled
    if (lofToggle) lof = !lof;
}

void
Beam::switchMode(TV format)
{
    switch (format) {

        case TV::PAL:

            type = TV::PAL;
            lol = false;
            lolToggle = false;
            vLatched = PAL::VPOS_MAX_LF;
            break;

        case TV::NTSC:

            type = TV::NTSC;
            lol = false;
            lolToggle = true;
            vLatched = NTSC::VPOS_MAX_LF;
            break;

        default:
            fatalError;
    }
}

}
