// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga/Foundation/Reflection.h"

namespace vamiga {

//
// Constants
//

// Autoconfig related constants ('type' field)
#define ERT_ZORROII         0xc0
#define ERT_ZORROIII        0x80
#define ERTF_MEMLIST        (1<<5)
#define ERTF_DIAGVALID      (1<<4)
#define ERTF_CHAINEDCONFIG  (1<<3)

// Autoconfig related constants ('flags' field)

#define ERFF_MEMSPACE       (1<<7)
#define ERFF_NOSHUTUP       (1<<6)
#define ERFF_EXTENDED       (1<<5)
#define ERFF_ZORRO_III      (1<<4)


//
// Enumerations
//

enum class BoardState : long
{
    AUTOCONF,
    ACTIVE,
    SHUTUP
};

struct BoardStateEnum : Reflection<BoardStateEnum, BoardState>
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
