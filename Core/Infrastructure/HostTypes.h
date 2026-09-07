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

//
// Enumerations
//

/// Texture format
enum class TexFormat : long
{
    ABGR,                       ///< AABBGGRR
    ARGB,                       ///< AARRGGBB
    RGBA                        ///< RRGGBBAA
};

struct TexFormatEnum : Reflectable<TexFormatEnum, TexFormat> {

    static constexpr long minVal = 0;
    static constexpr long maxVal = long(TexFormat::RGBA);

    static const char *_key(TexFormat value)
    {
        switch (value) {

            case TexFormat::ABGR:       return "ABGR";
            case TexFormat::ARGB:       return "ARGB";
            case TexFormat::RGBA:       return "RGBA";
        }
        return "???";
    }
    static const char *help(TexFormat value)
    {
        switch (value) {

            case TexFormat::ABGR:       return "32 bit AABBGGRR";
            case TexFormat::ARGB:       return "32 bit AARRGGBB";
            case TexFormat::RGBA:       return "32 bit RRGGBBAA";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    // Refresh rate of the host display
    isize refreshRate;

    // Audio sample rate of the host computer
    isize sampleRate;

    // Texture format
    TexFormat texFormat;

    // Framebuffer dimensions
    isize frameBufferWidth;
    isize frameBufferHeight;
}
HostConfig;

}
