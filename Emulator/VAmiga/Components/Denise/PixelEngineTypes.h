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

namespace vamiga {

//
// Enumerations
//

enum class Palette
{
    COLOR,
    BLACK_WHITE,
    PAPER_WHITE,
    GREEN,
    AMBER,
    SEPIA
};

struct PaletteEnum : Reflection<PaletteEnum, Palette>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Palette::SEPIA);
    
    static const char *_key(Palette value)
    {
        switch (value) {
                
            case Palette::COLOR:        return "COLOR";
            case Palette::BLACK_WHITE:  return "BLACK_WHITE";
            case Palette::PAPER_WHITE:  return "PAPER_WHITE";
            case Palette::GREEN:        return "GREEN";
            case Palette::AMBER:        return "AMBER";
            case Palette::SEPIA:        return "SEPIA";
        }
        return "???";
    }
    static const char *help(Palette value)
    {
        switch (value) {
                
            case Palette::COLOR:        return "Color palette";
            case Palette::BLACK_WHITE:  return "Black and white palette";
            case Palette::PAPER_WHITE:  return "Paper white palette";
            case Palette::GREEN:        return "Green palette";
            case Palette::AMBER:        return "Amber palette";
            case Palette::SEPIA:        return "Sepia palette";
        }
        return "???";
    }
};


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

}
