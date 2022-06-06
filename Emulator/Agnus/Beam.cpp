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
    if (i < 0) { *this -= -i; return *this; }

    // Jump near the target frame
    auto cycles = cyclesPerFrames(4);
    frame += (i / cycles) * 4;
    i %= cycles;

    while (i > 0) {

        cycles = i < HPOS_MAX_PAL ? i : HPOS_MAX_PAL;
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

        cycles = i < HPOS_MAX_PAL ? i : HPOS_MAX_PAL;
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

FrameType
Beam::predictFrameType() const
{
    // PAL
    if (type == PAL) {
        return lof ? FRAME_PAL_LF : FRAME_PAL_SF;
    }

    // NTSC
    if (IS_EVEN(v) && lol) {
        return lof ? FRAME_NTSC_LF_LL : FRAME_NTSC_SF_LL;
    } else {
        return lof ? FRAME_NTSC_LF_SL : FRAME_NTSC_SF_SL;
    }
}

FrameType
Beam::predictNextFrameType(FrameType type, bool toggle)
{
    switch (type) {

        case FRAME_PAL_LF:
            return toggle ? FRAME_PAL_SF : FRAME_PAL_LF;

        case FRAME_PAL_SF:
            return toggle ? FRAME_PAL_LF : FRAME_PAL_SF;

        case FRAME_NTSC_LF_LL:
            return toggle ? FRAME_NTSC_SF_SL : FRAME_NTSC_LF_SL;

        case FRAME_NTSC_LF_SL:
            return toggle ? FRAME_NTSC_SF_LL : FRAME_NTSC_LF_LL;

        case FRAME_NTSC_SF_LL:
            return toggle ? FRAME_NTSC_LF_LL : FRAME_NTSC_SF_LL;

        case FRAME_NTSC_SF_SL:
            return toggle ? FRAME_NTSC_LF_SL : FRAME_NTSC_SF_SL;

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

        case FRAME_PAL_LF:
            return VPOS_CNT_PAL_LF * HPOS_CNT_PAL;

        case FRAME_PAL_SF:
            return VPOS_CNT_PAL_SF * HPOS_CNT_PAL;

        case FRAME_NTSC_LF_LL:
            return 132 * HPOS_CNT_NTSC_LL + 131 * HPOS_CNT_NTSC_SL;

        case FRAME_NTSC_LF_SL:
            return 132 * HPOS_CNT_NTSC_SL + 131 * HPOS_CNT_NTSC_LL;

        case FRAME_NTSC_SF_LL:
        case FRAME_NTSC_SF_SL:
            return 131 * HPOS_CNT_NTSC_SL + 131 * HPOS_CNT_NTSC_LL;

        default:
            fatalError;
    }
}

isize
Beam::cyclesPerFrame() const
{
    return cyclesPerFrame(type);
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
    return cyclesPerFrames(count, type, lofToggle);
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
Beam::switchMode(VideoFormat format)
{
    switch (format) {

        case PAL:

            type = PAL;
            lol = false;
            lolToggle = false;
            vLatched = VPOS_MAX_PAL_LF;
            break;

        case NTSC:

            type = NTSC;
            lol = false;
            lolToggle = true;
            vLatched = VPOS_MAX_NTSC_LF;
            break;

        default:
            fatalError;
    }
}

