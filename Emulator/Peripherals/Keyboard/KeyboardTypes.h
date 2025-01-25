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

enum class KeyboardState
{
    SELFTEST,
    SYNC,
    STRM_ON,
    STRM_OFF,
    SEND
};

struct KeyboardStateEnum : util::Reflection<KeyboardStateEnum, KeyboardState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(KeyboardState::SEND);
    
    static const char *_key(KeyboardState value)
    {
        switch (value) {
                
            case KeyboardState::SELFTEST:  return "SELFTEST";
            case KeyboardState::SYNC:      return "SYNC";
            case KeyboardState::STRM_ON:   return "STRM_ON";
            case KeyboardState::STRM_OFF:  return "STRM_OFF";
            case KeyboardState::SEND:      return "SEND";
        }
        return "???";
    }
    static const char *help(KeyboardState value)
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
