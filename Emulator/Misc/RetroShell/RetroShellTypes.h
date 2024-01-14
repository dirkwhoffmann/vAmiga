// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(RETRO_SHELL_KEY)
{
    RSKEY_UP,
    RSKEY_DOWN,
    RSKEY_LEFT,
    RSKEY_RIGHT,
    RSKEY_DEL,
    RSKEY_CUT,
    RSKEY_BACKSPACE,
    RSKEY_HOME,
    RSKEY_END,
    RSKEY_TAB,
    RSKEY_RETURN,
    RSKEY_CR
};
typedef RETRO_SHELL_KEY RetroShellKey;

#ifdef __cplusplus
struct RetroShellKeyEnum : util::Reflection<RetroShellKeyEnum, RetroShellKey>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = RSKEY_CR;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "RSKEY"; }
    static const char *key(RetroShellKey value)
    {
        switch (value) {
                
            case RSKEY_UP:          return "UP";
            case RSKEY_DOWN:        return "DOWN";
            case RSKEY_LEFT:        return "LEFT";
            case RSKEY_RIGHT:       return "RIGHT";
            case RSKEY_DEL:         return "DEL";
            case RSKEY_CUT:         return "CUT";
            case RSKEY_BACKSPACE:   return "BACKSPACE";
            case RSKEY_HOME:        return "HOME";
            case RSKEY_END:         return "END";
            case RSKEY_TAB:         return "TAB";
            case RSKEY_RETURN:      return "RETURN";
            case RSKEY_CR:          return "CR";
        }
        return "???";
    }
};
#endif
