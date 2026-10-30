// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {

//
// Enumerations
//

/// State of the keyboard.
enum class KbState
{
    SELFTEST,
    SYNC,
    STRM_ON,
    STRM_OFF,
    SEND
};

struct KbStateEnum : utl::Reflectable<KbStateEnum, KbState>
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

/// Configuration options of the keyboard.
typedef struct
{
    bool accurate;                          ///< Emulate the keyboard protocol accurately
}
KeyboardConfig;

/// State of the keyboard.
typedef struct
{
    KbState state;                          ///< State of the keyboard protocol
    u8 shiftReg;                            ///< Shift register
    
} KeyboardInfo;

}
