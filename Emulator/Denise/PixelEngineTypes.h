// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(PALETTE)
{
    PALETTE_COLOR,
    PALETTE_BLACK_WHITE,
    PALETTE_PAPER_WHITE,
    PALETTE_GREEN,
    PALETTE_AMBER,
    PALETTE_SEPIA
};
typedef PALETTE Palette;

#ifdef __cplusplus
struct PaletteEnum : util::Reflection<PaletteEnum, Palette>
{    
    static long minVal() { return 0; }
    static long maxVal() { return PALETTE_SEPIA; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "PALETTE"; }
    static const char *key(Palette value)
    {
        switch (value) {
                
            case PALETTE_COLOR:        return "COLOR";
            case PALETTE_BLACK_WHITE:  return "BLACK_WHITE";
            case PALETTE_PAPER_WHITE:  return "PAPER_WHITE";
            case PALETTE_GREEN:        return "GREEN";
            case PALETTE_AMBER:        return "AMBER";
            case PALETTE_SEPIA:        return "SEPIA";
        }
        return "???";
    }
};
#endif

//
// Structures
//

typedef struct
{
    Palette palette;
    isize brightness;
    isize contrast;
    isize saturation;
}
PixelEngineConfig;
