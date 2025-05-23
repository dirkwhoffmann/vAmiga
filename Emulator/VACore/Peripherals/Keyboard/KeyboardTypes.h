// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VACore/Foundation/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class KbState
{
    SELFTEST,
    SYNC,
    STRM_ON,
    STRM_OFF,
    SEND
};

struct KbStateEnum : Reflection<KbStateEnum, KbState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(KbState::SEND);
    
    static const char *_key(KbState value)
    {
        switch (value) {
                
            case KbState::SELFTEST:  return "SELFTEST";
            case KbState::SYNC:      return "SYNC";
            case KbState::STRM_ON:   return "STRM_ON";
            case KbState::STRM_OFF:  return "STRM_OFF";
            case KbState::SEND:      return "SEND";
        }
        return "???";
    }
    static const char *help(KbState value)
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
    KbState state;
    u8 shiftReg;
    
} KeyboardInfo;

}
