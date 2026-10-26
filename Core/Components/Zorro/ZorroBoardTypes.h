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
// Constants
//

// Autoconfig related constants ('type' field)
inline constexpr int ERT_ZORROII = 0xc0;
inline constexpr int ERT_ZORROIII = 0x80;
inline constexpr int ERTF_MEMLIST = (1<<5);
inline constexpr int ERTF_DIAGVALID = (1<<4);
inline constexpr int ERTF_CHAINEDCONFIG = (1<<3); // Autoconfig related constants ('flags' field)

inline constexpr int ERFF_MEMSPACE = (1<<7);
inline constexpr int ERFF_NOSHUTUP = (1<<6);
inline constexpr int ERFF_EXTENDED = (1<<5);
inline constexpr int ERFF_ZORRO_III = (1<<4); //
// Enumerations
//

enum class BoardState : long
{
    AUTOCONF,
    ACTIVE,
    SHUTUP
};

struct BoardStateEnum : Reflectable<BoardStateEnum, BoardState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(BoardState::SHUTUP);
    
    static const char *_key(BoardState value)
    {
        switch (value) {
                
            case BoardState::AUTOCONF:    return "AUTOCONF";
            case BoardState::ACTIVE:      return "ACTIVE";
            case BoardState::SHUTUP:      return "SHUTUP";
        }
        return "???";
    }
    static const char *help(BoardState value)
    {
        return "";
    }
};

}
