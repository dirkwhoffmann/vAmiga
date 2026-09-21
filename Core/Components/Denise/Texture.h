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
#include "Constants.h"
#include "utl/color/Colors.h"
#include "utl/storage.h"


namespace vamiga {

using utl::Buffer;

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

    // Frame number
    i64 nr = 0;

    // Pixel buffer
    Buffer <Texel> pixels;

    // The long-frame bit of this frame
    bool lof = false;

    // The long-frame bit of the previous frame
    bool prevlof = false;

    Texture();

    /* Initializes (a portion of) the frame buffer with a checkerboard
     * pattern woven from col1 and col2. Both colors are taken as-is --
     * whoever calls this is responsible for packing them in the host's
     * current HOST_TEX_FORMAT (see GpuColor<F>/PixelEngine::toTexel<F>);
     * Texture itself has no notion of texel format. Defaults to the
     * class's own predefined grey checkerboard, which -- like all the
     * other named constants above -- is only really correct for formats
     * with alpha in the top byte (ABGR/ARGB); pass explicit, properly
     * converted colors wherever host format correctness matters.
     */
    void clear(Texel col1 = grey2, Texel col2 = grey4);
    void clear(isize row, Texel col1 = grey2, Texel col2 = grey4);
    void clear(isize row, isize cycle, Texel col1 = grey2, Texel col2 = grey4);
};

}
