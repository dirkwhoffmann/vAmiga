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
#define AGN_HSYNC            (1ULL << 0)
#define AGN_BPLCON0_0        (1ULL << 1)  // BPLCON0 change (Agnus view)
#define AGN_BPLCON0_1        (1ULL << 2)
#define AGN_BPLCON0_2        (1ULL << 3)
#define AGN_BPLCON0_3        (1ULL << 4)
#define AGN_BPLCON0_DENISE_0 (1ULL << 5)  // BPLCON0 change (Denise view)
#define AGN_BPLCON0_DENISE_1 (1ULL << 6)
#define AGN_BPLCON1_0        (1ULL << 7)  // BPLCON1
#define AGN_BPLCON1_1        (1ULL << 8)
#define AGN_BPLCON2_0        (1ULL << 9)  // BPLCON2
#define AGN_BPLCON2_1        (1ULL << 10)
#define AGN_DMACON_0         (1ULL << 11) // DMACON
#define AGN_DMACON_1         (1ULL << 12)
#define AGN_DIWSTRT_0        (1ULL << 13) // DIWSTRT
#define AGN_DIWSTRT_1        (1ULL << 14)
#define AGN_DIWSTOP_0        (1ULL << 15) // DIWSTOP
#define AGN_DIWSTOP_1        (1ULL << 16)
#define AGN_BPL1MOD_0        (1ULL << 17) // BPL1MOD
#define AGN_BPL1MOD_1        (1ULL << 18)
#define AGN_BPL2MOD_0        (1ULL << 19) // BPL2MOD
#define AGN_BPL2MOD_1        (1ULL << 20)
#define AGN_BPL1PTH_0        (1ULL << 21) // BPL1PTH
#define AGN_BPL1PTH_1        (1ULL << 22)
#define AGN_BPL1PTL_0        (1ULL << 23) // BPL1PTL
#define AGN_BPL1PTL_1        (1ULL << 24)
#define AGN_BPL2PTH_0        (1ULL << 25) // BPL2PTH
#define AGN_BPL2PTH_1        (1ULL << 26)
#define AGN_BPL2PTL_0        (1ULL << 27) // BPL2PTL
#define AGN_BPL2PTL_1        (1ULL << 28)
#define AGN_BPL3PTH_0        (1ULL << 29) // BPL3PTH
#define AGN_BPL3PTH_1        (1ULL << 30)
#define AGN_BPL3PTL_0        (1ULL << 31) // BPL3PTL
#define AGN_BPL3PTL_1        (1ULL << 32)
#define AGN_BPL4PTH_0        (1ULL << 33) // BPL4PTH
#define AGN_BPL4PTH_1        (1ULL << 34)
#define AGN_BPL4PTL_0        (1ULL << 35) // BPL4PTL
#define AGN_BPL4PTL_1        (1ULL << 36)
#define AGN_BPL5PTH_0        (1ULL << 37) // BPL5PTH
#define AGN_BPL5PTH_1        (1ULL << 38)
#define AGN_BPL5PTL_0        (1ULL << 39) // BPL5PTL
#define AGN_BPL5PTL_1        (1ULL << 40)
#define AGN_BPL6PTH_0        (1ULL << 41) // BPL6PTH
#define AGN_BPL6PTH_1        (1ULL << 42)
#define AGN_BPL6PTL_0        (1ULL << 43) // BPL6PTL
#define AGN_BPL6PTL_1        (1ULL << 44)

#define AGN_COUNT            (1ULL << 45)

#define AGN_BPLxPTH_0 \
AGN_BPL1PTH_0 | AGN_BPL2PTH_0 | AGN_BPL3PTH_0 | \
AGN_BPL4PTH_0 | AGN_BPL5PTH_0 | AGN_BPL6PTH_0

#define AGN_BPLxPTH_1 \
AGN_BPL1PTH_1 | AGN_BPL2PTH_1 | AGN_BPL3PTH_1 | \
AGN_BPL4PTH_1 | AGN_BPL5PTH_1 | AGN_BPL6PTH_1

#define AGN_BPLxPTL_0 \
AGN_BPL1PTL_0 | AGN_BPL2PTL_0 | AGN_BPL3PTL_0 | \
AGN_BPL4PTL_0 | AGN_BPL5PTL_0 | AGN_BPL6PTL_0

#define AGN_BPLxPTL_1 \
AGN_BPL1PTL_1 | AGN_BPL2PTL_1 | AGN_BPL3PTL_1 | \
AGN_BPL4PTL_1 | AGN_BPL5PTL_1 | AGN_BPL6PTL_1

#define AGN_DELAY_MASK ~( \
AGN_HSYNC | \
AGN_BPLCON0_0 | AGN_BPLCON0_DENISE_0 | AGN_BPLCON1_0  | AGN_BPLCON2_0 | \
AGN_DMACON_0 | \
AGN_DIWSTRT_0 | AGN_DIWSTOP_0 | \
AGN_BPL1MOD_0 | AGN_BPL2MOD_0 | \
AGN_BPLxPTH_0 | AGN_BPLxPTL_0 | \
AGN_COUNT )

#define AGN_REG_CHANGE_MASK UINT64_MAX // TODO: USE PROPER MASK AFTER DEBUGGING

/*
#define AGN_REG_CHANGE_MASK ( \
AGN_BPLCON0_3 | AGN_BPLCON0_DENISE_0 | AGN_BPLCON1_1 | AGN_BPLCON2_1 | \
AGN_DMACON_1 | \
AGN_DIWSTRT_1 | AGN_DIWSTOP_1 | \
AGN_BPL1MOD_1 | AGN_BPL2MOD_1 | \
AGN_BPLxPTH_1 | AGN_BPLxPTL_1 )
*/

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
AgnusInfo;

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
