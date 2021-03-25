// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Reflection.hpp"

namespace va {

#include "KeyboardTypes.h"

//
// Reflection APIs
//

struct KeyboardStateEnum : Reflection<KeyboardStateEnum, KeyboardState> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <  KB_COUNT;
    }
    
    static const char *prefix() { return "KB"; }
    static const char *key(KeyboardState value)
    {
        switch (value) {
                
            case KB_SELFTEST:  return "SELFTEST";
            case KB_SYNC:      return "SYNC";
            case KB_STRM_ON:   return "STRM_ON";
            case KB_STRM_OFF:  return "STRM_OFF";
            case KB_SEND:      return "SEND";
            case KB_COUNT:     return "???";
        }
        return "???";
    }
};

}
