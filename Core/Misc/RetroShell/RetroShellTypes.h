// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class RSKey
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    PAGE_UP,
    PAGE_DOWN,
    DEL,
    CUT,
    BACKSPACE,
    HOME,
    END,
    TAB,
    RETURN,
    CR
};

struct RSKeyEnum : Reflection<RSKeyEnum, RSKey>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(RSKey::CR);

    static const char *_key(RSKey value)
    {
        switch (value) {

            case RSKey::UP:          return "UP";
            case RSKey::DOWN:        return "DOWN";
            case RSKey::LEFT:        return "LEFT";
            case RSKey::RIGHT:       return "RIGHT";
            case RSKey::PAGE_UP:     return "PAGE_UP";
            case RSKey::PAGE_DOWN:   return "PAGE_DOWN";
            case RSKey::DEL:         return "DEL";
            case RSKey::CUT:         return "CUT";
            case RSKey::BACKSPACE:   return "BACKSPACE";
            case RSKey::HOME:        return "HOME";
            case RSKey::END:         return "END";
            case RSKey::TAB:         return "TAB";
            case RSKey::RETURN:      return "RETURN";
            case RSKey::CR:          return "CR";
        }
        return "???";
    }
    static const char *help(RSKey value)
    {
        return "";
    }
};

//
// Structures
//

typedef struct
{
    // Active console
    isize console;

    // Relative position of the cursor
    isize cursorRel;
}
RetroShellInfo;

// Used in operator overloads
struct vspace { isize lines = 0; };

}
