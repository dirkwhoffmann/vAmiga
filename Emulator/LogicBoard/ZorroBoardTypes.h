//
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
    static long minVal() { return 0; }
    static long maxVal() { return STATE_SHUTUP; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
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
