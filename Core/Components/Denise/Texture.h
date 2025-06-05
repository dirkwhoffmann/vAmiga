// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FrameBufferTypes.h"
#include "Buffer.h"
#include "Constants.h"
#include "Colors.h"


namespace vamiga {

using util::Buffer;

class Texture {

public:
    
    // Predefined colors
    static constexpr Texel black    = TEXEL(0xFF000000);
    static constexpr Texel grey2    = TEXEL(0xFF222222);
    static constexpr Texel grey4    = TEXEL(0xFF444444);
    static constexpr Texel white    = TEXEL(0xFFFFFFFF);
    static constexpr Texel red      = TEXEL(0xFF0000FF);
    static constexpr Texel green    = TEXEL(0xFF00FF00);
    static constexpr Texel blue     = TEXEL(0xFFFF0000);
    static constexpr Texel yellow   = TEXEL(0xFF00FFFF);
    static constexpr Texel magenta  = TEXEL(0xFFFF00FF);
    static constexpr Texel cyan     = TEXEL(0xFFFFFF00);

    // Color aliases
    static constexpr Texel vblank   = grey4;    // VBLANK area
    static constexpr Texel hblank   = grey4;    // HBLANK area
    static constexpr Texel cb1      = grey2;    // Checkerboard color 1
    static constexpr Texel cb2      = grey4;    // Checkerboard color 2

    // Frame number
    i64 nr = 0;

    // Pixel buffer
    Buffer <Texel> pixels;

    // The long-frame bit of this frame
    bool lof = false;

    // The long-frame bit of the previous frame
    bool prevlof = false;

    Texture();

    // Initializes (a portion of) the frame buffer with a checkerboard pattern
    void clear();
    void clear(isize row);
    void clear(isize row, isize cycle);
};

}
