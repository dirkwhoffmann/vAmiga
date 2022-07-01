//
// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FrameBuffer.h"

FrameBuffer::FrameBuffer()
{
    pixels.alloc(PIXELS);
}

void
FrameBuffer::clear()
{
    auto *ptr = pixels.ptr;

    for (isize row = 0; row < VPIXELS; row++, ptr += HPIXELS) {
        for (isize col = 0; col < HPIXELS; col++) {
            ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? cb1 : cb2;
        }
    }
}

void
FrameBuffer::clear(isize row)
{
    auto *ptr = pixels.ptr + row * HPIXELS;

    for (isize col = 0; col < HPIXELS; col++) {
        ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? cb1 : cb2;
    }
}

void
FrameBuffer::clear(isize row, isize cycle)
{
    auto *ptr = pixels.ptr + row * HPIXELS + 4 * cycle;

    for (isize col = 0; col < 4; col++) {
        ptr[col] = ((row >> 2) & 1) == ((col >> 3) & 1) ? cb1 : cb2;
    }
}
