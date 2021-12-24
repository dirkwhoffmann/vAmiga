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

enum_long(SRV_STATE)
{
    SRV_STATE_OFF,
    SRV_STATE_LAUNCHING,
    SRV_STATE_LISTENING,
    SRV_STATE_CONNECTED,
    SRV_STATE_ERROR
};
typedef SRV_STATE SrvState;

#ifdef __cplusplus
struct SrvStateEnum : util::Reflection<SrvStateEnum, SrvState>
{
    static long minVal() { return 0; }
    static long maxVal() { return SRV_STATE_ERROR; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "SRV"; }
    static const char *key(SrvState value)
    {
        switch (value) {
                
            case SRV_STATE_OFF:         return "OFF";
            case SRV_STATE_LAUNCHING:   return "LAUNCHING";
            case SRV_STATE_LISTENING:   return "LISTENING";
            case SRV_STATE_CONNECTED:   return "CONNECTED";
            case SRV_STATE_ERROR:       return "ERROR";
        }
        return "???";
    }
};
#endif
