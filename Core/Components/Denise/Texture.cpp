// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Texture.h"

namespace vamiga {

Texture::Texture()
{
    pixels.alloc(PIXELS);
}

void
Texture::clear()
{
    auto *ptr = pixels.ptr;

    for (isize row = 0; row < VPIXELS; row++, ptr += HPIXELS) {
        for (isize col = 0; col < HPIXELS; col++) {
            ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? cb1 : cb2;
        }
    }
}

void
Texture::clear(isize row)
{
    auto *ptr = pixels.ptr + row * HPIXELS;

    for (isize col = 0; col < HPIXELS; col++) {
        ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? cb1 : cb2;
    }
}

void
Texture::clear(isize row, isize cycle)
{
    auto *ptr = pixels.ptr + row * HPIXELS + 4 * cycle;

    for (isize col = 0; col < 4; col++) {
        ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? cb1 : cb2;
    }
}

}
