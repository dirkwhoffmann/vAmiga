// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BeamTypes.h"
#include "AmigaTypes.h"
#include "Constants.h"
#include "Serializable.h"

namespace vamiga {

struct Beam : SerializableStruct
{
    // The vertical and horizontal beam position
    isize v = 0;
    isize h = 0;

    // Latched coordinates (recorded in eof() and eol(), respectively)
    isize vLatched = VPOS_CNT;
    isize hLatched = HPOS_CNT;

    // The frame count
    i64 frame = 0;

    // Long frame flipflop
    bool lof = false;
    bool lofToggle = false;

    // Long line fliflop
    bool lol = false;
    bool lolToggle = false;

    // The type of the current line
    TV type;


    //
    // Methods from Serializable
    //

public:

    template <class T>
    void serialize(T& worker)
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

    } STRUCT_SERIALIZERS(serialize);


    //
    // Querying coordinates
    //

    isize vPrev() const { return v ? v - 1 : vLatched - 1; }
    isize hPrev() const { return h ? h - 1 : hLatched - 1; }

    
    //
    // Querying boundaries
    //

    isize hCnt() const { return lol ? 228 : 227; }
    isize hMax() const { return lol ? 227 : 226; }
    isize vCnt() const { return type == TV::PAL ? vCntPal() : vCntNtsc(); }
    isize vMax() const { return type == TV::PAL ? vMaxPal() : vMaxNtsc(); }
    isize vMaxPal() const { return lof ? 312 : 311; }
    isize vMaxNtsc() const { return lof ? 262 : 261; }
    isize vCntPal() const { return lof ? 313 : 312; }
    isize vCntNtsc() const { return lof ? 263 : 262; }


    //
    // Comparing
    //

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


    //
    // Calculating new beam positions
    //

    Beam& operator+=(isize i);
    Beam operator+(const isize i) const;

    Beam& operator-=(isize i);
    Beam operator-(const isize i) const;

    // Computes the DMA cycle difference to the specified position
    isize diff(isize v2, isize h2) const;

    // Predicts the type of the current frame
    FrameType predictFrameType() const;

    // Predicts the type of the next frame
    static FrameType predictNextFrameType(FrameType type, bool toggle);
    FrameType predictNextFrameType() const;

    // Returns the number of DMA cycles executed in a single frame
    static isize cyclesPerFrame(FrameType type);
    isize cyclesPerFrame() const;

    // Returns the number of DMA cycles executed in a certain number of frames
    static isize cyclesPerFrames(isize count, FrameType type, bool toggle);
    isize cyclesPerFrames(isize count) const;


    //
    // Converting positions
    //

    // Translates a DMA cycle to a pixel position
    Pixel pixel(isize h) const;
    Pixel pixel() const { return pixel(h); }


    //
    // Switching lines, frames, and video modes
    //

    // Called by Agnus in the EOL handler to switch to the next line
    void eol();

    // Called by Agnus in the EOF handler to switch to the next frame
    void eof();

    // Called by Agnus when the video format is changed (PAL / NTSC)
    void switchMode(TV format);
};

}
