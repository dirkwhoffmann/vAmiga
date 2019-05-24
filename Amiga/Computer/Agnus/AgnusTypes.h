// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _AGNUS_T_INC
#define _AGNUS_T_INC

typedef struct
{
    int16_t y;
    int16_t x;
}
Beam;

typedef enum : int8_t
{
    BUS_NONE,
    BUS_CPU,
    BUS_DISK,
    BUS_AUDIO,
    BUS_SPRITE,
    BUS_BITPLANE,
    BUS_BLITTER,
    BUS_COPPER,
    BUS_OWNER_COUNT
}
BusOwner;

static inline bool isBusOwner(long value) { return value < BUS_OWNER_COUNT; }

typedef enum
{
    SPR_DMA_IDLE,
    SPR_DMA_DATA, 
    
    SPR_FETCH_CONFIG, // DEPRECATED
    SPR_WAIT_VSTART, // DEPRECATED
    SPR_FETCH_DATA // DEPRECATED
}
SprDMAState;

#endif
