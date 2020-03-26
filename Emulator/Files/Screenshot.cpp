// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
Screenshot::take(Amiga *amiga)
{
    u32 *source = (u32 *)amiga->denise.pixelEngine.getStableLongFrame().data;
    u32 *target = screen;
    
    int dx = 4;
    int dy = 2;
    int xStart = 4 * HBLANK_MAX, xEnd = HPIXELS + 4 * HBLANK_MIN;
    int yStart = VBLANK_CNT, yEnd = VPIXELS;
    
    width  = (xEnd - xStart) / dx;
    height = (yEnd - yStart) / dy;
    
    source += xStart + yStart * HPIXELS;
    
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            target[x] = source[x * dx];
        }
        source += dy * HPIXELS;
        target += width;
    }
    
    timestamp = time(NULL);
}
