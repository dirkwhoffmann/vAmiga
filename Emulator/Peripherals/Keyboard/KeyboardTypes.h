// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum_long(KB_STATE)
{
    KB_SELFTEST,
    KB_SYNC,
    KB_STRM_ON,
    KB_STRM_OFF,
    KB_SEND
};
typedef KB_STATE KeyboardState;

struct KeyboardStateEnum : util::Reflection<KeyboardStateEnum, KeyboardState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = KB_SEND;
    
    static const char *_key(long value)
    {
        switch (value) {
                
            case KB_SELFTEST:  return "SELFTEST";
            case KB_SYNC:      return "SYNC";
            case KB_STRM_ON:   return "STRM_ON";
            case KB_STRM_OFF:  return "STRM_OFF";
            case KB_SEND:      return "SEND";
        }
        return "???";
    }
    static const char *help(long value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct
{
    bool accurate;
}
KeyboardConfig;

typedef struct
{
    KeyboardState state;
    u8 shiftReg;
    
} KeyboardInfo;

}
