// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "Texture.h"

namespace vamiga {

Texture::Texture()
{
    pixels.alloc(PIXELS);
}

void
Texture::clear(Texel col1, Texel col2)
{
    auto *ptr = pixels.ptr;

    for (isize row = 0; row < VPIXELS; row++, ptr += HPIXELS) {
        for (isize col = 0; col < HPIXELS; col++) {
            ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? col1 : col2;
        }
    }
}

void
Texture::clear(isize row, Texel col1, Texel col2)
{
    auto *ptr = pixels.ptr + row * HPIXELS;

    for (isize col = 0; col < HPIXELS; col++) {
        ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? col1 : col2;
    }
}

void
Texture::clear(isize row, isize cycle, Texel col1, Texel col2)
{
    auto *ptr = pixels.ptr + row * HPIXELS + 4 * cycle;

    for (isize col = 4 * cycle; col < 4 * cycle + 4; col++) {
        ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? col1 : col2;
    }
}

}
