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

//
// Enumerations
//

typedef enum : long
{
    FILT_NONE,
    FILT_BUTTERWORTH
}
FilterType;


//
// Structures
//

typedef struct
{
    uint16_t intreq;
    uint16_t intena;
    uint16_t adkcon;
}
PaulaInfo;

typedef struct
{
    int8_t selectedDrive;
    DriveState state;
    int32_t fifo[6];
    uint8_t fifoCount;
    
    uint16_t dsklen;
    uint16_t dskbytr;
    uint16_t dsksync;
    uint8_t prb;
}
DiskControllerInfo;

#endif
