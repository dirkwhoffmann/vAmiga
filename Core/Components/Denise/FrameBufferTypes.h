// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"

namespace vamiga {

/* The Texel datatype is utilized to access single texture elements. On the
 * emulator side, one Texel covers a single Amiga hires pixel and holds two
 * RGBA values, one per super-hires pixel. On the GPU side a texel is always
 * 32 bit, so the GPU sees a texture of doubled horizontal resolution, i.e.
 * one texel per super-hires pixel.
 *
 * This used to be selectable at compile time via a macro called TPP (texels
 * per pixel), which could map Texel to a single u32 and produce a texture in
 * hires resolution. That option has been removed: super-hires resolution is
 * now the only texture format, so that a super-hires pixel is representable
 * everywhere in the pipeline rather than being squeezed into a hires cell.
 *
 * The TEXEL macro repeats the RGBA pattern, so writing a single Texel sets
 * two GPU texels at once. Writing a 64 bit value costs about as much as
 * writing a 32 bit one, which keeps the emulator-side price near zero; the
 * cost is on the GPU side, which has to move twice as much memory per frame.
 */
typedef u64 Texel;
#define TEXEL(rgba) ((u64)rgba << 32 | rgba)

}
