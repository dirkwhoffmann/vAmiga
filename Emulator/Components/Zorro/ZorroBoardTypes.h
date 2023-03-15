// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

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

enum_long(BOARD_STATE)
{
    STATE_AUTOCONF,
    STATE_ACTIVE,
    STATE_SHUTUP
};
typedef BOARD_STATE BoardState;

#ifdef __cplusplus
struct BoardStateEnum : util::Reflection<BoardStateEnum, BoardState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = STATE_SHUTUP;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "STATE"; }
    static const char *key(BoardState value)
    {
        switch (value) {

            case STATE_AUTOCONF:    return "AUTOCONF";
            case STATE_ACTIVE:      return "ACTIVE";
            case STATE_SHUTUP:      return "SHUTUP";
        }
        return "???";
    }
};
#endif
