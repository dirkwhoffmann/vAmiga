// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FrameBufferTypes.h"
#include "Buffer.h"
#include "Constants.h"

using util::Buffer;

struct FrameBuffer {

    // Checkerboard colors
    static constexpr Texel col1 = TEXEL(0xFF222222); // 0xFF662222
    static constexpr Texel col2 = TEXEL(0xFF444444); // 0xFFAA4444

    Buffer <Texel> pixels;
    bool longFrame;

    FrameBuffer();

    // Initializes (a portion of) the frame buffer with a checkerboard pattern
    void clear();
    void clear(isize row);
    void clear(isize row, isize cycle);
};
