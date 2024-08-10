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
#include "DiskControllerTypes.h"
#include "StateMachineTypes.h"
#include "UARTTypes.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(INT_SOURCE)
{
    INT_TBE,
    INT_DSKBLK,
    INT_SOFT,
    INT_PORTS,
    INT_COPER,
    INT_VERTB,
    INT_BLIT,
    INT_AUD0,
    INT_AUD1,
    INT_AUD2,
    INT_AUD3,
    INT_RBF,
    INT_DSKSYN,
    INT_EXTER
};
typedef INT_SOURCE IrqSource;

#ifdef __cplusplus
struct IrqSourceEnum : vamiga::util::Reflection<IrqSourceEnum, IrqSource>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = INT_EXTER;

    static const char *prefix() { return "INT"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case INT_TBE:     return "TBE";
            case INT_DSKBLK:  return "DSKBLK";
            case INT_SOFT:    return "SOFT";
            case INT_PORTS:   return "PORTS";
            case INT_COPER:   return "COPER";
            case INT_VERTB:   return "VERTB";
            case INT_BLIT:    return "BLIT";
            case INT_AUD0:    return "AUD0";
            case INT_AUD1:    return "AUD1";
            case INT_AUD2:    return "AUD2";
            case INT_AUD3:    return "AUD3";
            case INT_RBF:     return "RBF";
            case INT_DSKSYN:  return "DSKSYN";
            case INT_EXTER:   return "EXTER";
        }
        return "???";
    }
};
#endif

//
// Structures
//

typedef struct
{
    u16 intreq;
    u16 intena;
    u16 adkcon;
}
PaulaInfo;
