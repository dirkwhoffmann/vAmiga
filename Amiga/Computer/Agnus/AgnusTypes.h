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

// Action flags
#define AGS_HSYNC            (1ULL << 0)
#define AGS_BPLCON0_0        (1ULL << 1)  // BPLCON0 change (Agnus view)
#define AGS_BPLCON0_1        (1ULL << 2)
#define AGS_BPLCON0_2        (1ULL << 3)
#define AGS_BPLCON0_3        (1ULL << 4)
#define AGS_BPLCON0_DENISE_0 (1ULL << 5)  // BPLCON0 change (Denise view)
#define AGS_BPLCON0_DENISE_1 (1ULL << 6)
#define AGS_BPLCON1_0        (1ULL << 7)  // BPLCON1
#define AGS_BPLCON1_1        (1ULL << 8)
#define AGS_BPLCON2_0        (1ULL << 9)  // BPLCON2
#define AGS_BPLCON2_1        (1ULL << 10)
#define AGS_DMACON_0         (1ULL << 11) // DMACON
#define AGS_DMACON_1         (1ULL << 12)
#define AGS_DIWSTRT_0        (1ULL << 13) // DIWSTRT
#define AGS_DIWSTRT_1        (1ULL << 14)
#define AGS_DIWSTOP_0        (1ULL << 15) // AGS_DIWSTOP
#define AGS_DIWSTOP_1        (1ULL << 16)
#define AGS_BPL1MOD_0        (1ULL << 17)
#define AGS_BPL1MOD_1        (1ULL << 18)
#define AGS_BPL2MOD_0        (1ULL << 19)
#define AGS_BPL2MOD_1        (1ULL << 20)
#define AGS_BPL1PTH_0        (1ULL << 21)
#define AGS_BPL1PTH_1        (1ULL << 22)
#define AGS_BPL1PTL_0        (1ULL << 23)
#define AGS_BPL1PTL_1        (1ULL << 24)
#define AGS_BPL2PTH_0        (1ULL << 25)
#define AGS_BPL2PTH_1        (1ULL << 26)
#define AGS_BPL2PTL_0        (1ULL << 27)
#define AGS_BPL2PTL_1        (1ULL << 28)
#define AGS_BPL3PTH_0        (1ULL << 29)
#define AGS_BPL3PTH_1        (1ULL << 30)
#define AGS_BPL3PTL_0        (1ULL << 31)
#define AGS_BPL3PTL_1        (1ULL << 32)
#define AGS_BPL4PTH_0        (1ULL << 33)
#define AGS_BPL4PTH_1        (1ULL << 34)
#define AGS_BPL4PTL_0        (1ULL << 35)
#define AGS_BPL4PTL_1        (1ULL << 36)
#define AGS_BPL5PTH_0        (1ULL << 37)
#define AGS_BPL5PTH_1        (1ULL << 38)
#define AGS_BPL5PTL_0        (1ULL << 39)
#define AGS_BPL5PTL_1        (1ULL << 40)
#define AGS_BPL6PTH_0        (1ULL << 41)
#define AGS_BPL6PTH_1        (1ULL << 42)
#define AGS_BPL6PTL_0        (1ULL << 43)
#define AGS_BPL6PTL_1        (1ULL << 44)

#define AGS_COUNT            (1ULL << 45)

#define AGS_BPLxPTH_0 \
AGS_BPL1PTH_0 | AGS_BPL2PTH_0 | AGS_BPL3PTH_0 | \
AGS_BPL4PTH_0 | AGS_BPL5PTH_0 | AGS_BPL6PTH_0

#define AGS_BPLxPTH_1 \
AGS_BPL1PTH_1 | AGS_BPL2PTH_1 | AGS_BPL3PTH_1 | \
AGS_BPL4PTH_1 | AGS_BPL5PTH_1 | AGS_BPL6PTH_1

#define AGS_BPLxPTL_0 \
AGS_BPL1PTL_0 | AGS_BPL2PTL_0 | AGS_BPL3PTL_0 | \
AGS_BPL4PTL_0 | AGS_BPL5PTL_0 | AGS_BPL6PTL_0

#define AGS_BPLxPTL_1 \
AGS_BPL1PTL_1 | AGS_BPL2PTL_1 | AGS_BPL3PTL_1 | \
AGS_BPL4PTL_1 | AGS_BPL5PTL_1 | AGS_BPL6PTL_1

#define AGS_DELAY_MASK ~( AGS_COUNT | \
AGS_BPLCON0_0 | AGS_BPLCON0_DENISE_0 | AGS_BPLCON1_0  | AGS_BPLCON2_0 | \
AGS_DMACON_0 | \
AGS_DIWSTRT_0 | AGS_DIWSTOP_0 | \
AGS_BPL1MOD_0 | AGS_BPL2MOD_0 | \
AGS_BPLxPTH_0 | AGS_BPLxPTL_0 )

#define AGS_REG_CHANGE UINT64_MAX // TODO: USE PROPER MASK AFTER DEBUGGING

typedef enum : int8_t
{
    BUS_NONE,
    BUS_CPU,
    BUS_REFRESH,
    BUS_DISK,
    BUS_AUDIO,
    BUS_BITPLANE,
    BUS_SPRITE,
    BUS_COPPER,
    BUS_BLITTER,
    BUS_OWNER_COUNT
}
BusOwner;

static inline bool isBusOwner(long value) { return value >= 0 && value < BUS_OWNER_COUNT; }

typedef enum
{
    SPR_DMA_IDLE,
    SPR_DMA_DATA, 
    
    SPR_FETCH_CONFIG, // DEPRECATED
    SPR_WAIT_VSTART, // DEPRECATED
    SPR_FETCH_DATA // DEPRECATED
}
SprDMAState;

typedef enum : long
{
    MODULATE_FG_LAYER,
    MODULATE_BG_LAYER,
    MODULATE_BOTH_LAYERS
}
DmaDebuggerDisplayMode;

typedef struct
{
    bool enabled;
    bool visualize[BUS_OWNER_COUNT];
    double colorRGB[BUS_OWNER_COUNT][3];
    double opacity;
    DmaDebuggerDisplayMode displayMode;
}
DMADebuggerInfo;

typedef struct
{
    uint16_t dmacon;
    uint16_t diwstrt;
    uint16_t diwstop;
    uint16_t ddfstrt;
    uint16_t ddfstop;

    int16_t bpl1mod;
    int16_t bpl2mod;
    uint8_t  numBpls;

    uint32_t dskpt;
    uint32_t audlc[4];
    uint32_t bplpt[6];
    uint32_t sprpt[8];
}
DMAInfo;

typedef struct
{
    bool active;
    bool cdang;
    uint32_t coppc;
    uint32_t cop1lc;
    uint32_t cop2lc;
    uint16_t cop1ins;
    uint16_t cop2ins;
    int16_t length1;
    int16_t length2;
}
CopperInfo;

typedef struct
{
    bool active;
    uint16_t bltcon0;
    uint16_t bltcon1;
    uint16_t bltapt;
    uint16_t bltbpt;
    uint16_t bltcpt;
    uint16_t bltdpt;
    uint16_t bltafwm;
    uint16_t bltalwm;
    uint16_t bltsize;
    int16_t bltamod;
    int16_t bltbmod;
    int16_t bltcmod;
    int16_t bltdmod;
    uint16_t anew;
    uint16_t bnew;
    uint16_t ahold;
    uint16_t bhold;
    uint16_t chold;
    uint16_t dhold;
    bool bbusy;
    bool bzero;
}
BlitterInfo;

#endif
