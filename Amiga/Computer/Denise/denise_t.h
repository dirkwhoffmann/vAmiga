// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This include file contains all type definitions related to Denise.
// It conforms to standard ANSI-C in order to be compatible with Swift.

#ifndef _DENISE_T_INC
#define _DENISE_T_INC

typedef struct
{
    uint16_t bplcon0;
    uint16_t bplcon1;
    uint16_t bplcon2;
    uint16_t bpldat[6];
    uint32_t color[32];
}
DeniseInfo;

typedef enum
{
    SPR_FETCH_CONFIG,
    SPR_WAIT_VSTART,
    SPR_FETCH_DATA
}
SprDMAState;

#endif
