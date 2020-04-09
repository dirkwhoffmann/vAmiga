// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _PAULA_T_INC
#define _PAULA_T_INC

#include "AudioUnitTypes.h"
#include "DiskControllerTypes.h"
#include "DriveTypes.h"

//
// Enumerations
//

typedef enum : long
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
    INT_EXTER,
    INT_COUNT
}
IrqSource;

static inline bool isIrqSource(long value) { return value >= 0 && value < INT_COUNT; }


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

typedef struct
{
    u16 receiveBuffer;
    u16 receiveShiftReg;
    u16 transmitBuffer;
    u16 transmitShiftReg;
}
UARTInfo;

#endif
