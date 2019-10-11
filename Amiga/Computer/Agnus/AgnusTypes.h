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

// Emulated model
typedef enum : long
{
    AGNUS_8367, // Used if an A1000 is emulated
    AGNUS_8372  // Used if an A500 or A2000 is emulated
}
AgnusType;

inline bool isAgnusType(long value)
{
    return value >= AGNUS_8367 && value <= AGNUS_8372;
}

inline const char *AgnusTypeName(AgnusType type)
{
    assert(isAgnusType(type));

    switch (type) {
        case AGNUS_8367: return "AGNUS_8367";
        case AGNUS_8372: return "AGNUS_8372";
        default:         return "???";
    }
}

typedef struct
{
    AgnusType type;
}
AgnusConfig;

// Register change identifiers
typedef enum : int32_t
{
    REG_NONE = 0,
    REG_INTREQ,
    REG_INTENA,
    REG_BPLCON0_AGNUS,
    REG_BPLCON0_DENISE,
    REG_BPLCON1,
    REG_BPLCON2,
    REG_DMACON,
    REG_DIWSTRT,
    REG_DIWSTOP,
    REG_DDFSTRT,
    REG_DDFSTOP,
    REG_BPL1MOD,
    REG_BPL2MOD,
    REG_BPL1PTH,
    REG_BPL1PTL,
    REG_BPL2PTH,
    REG_BPL2PTL,
    REG_BPL3PTH,
    REG_BPL3PTL,
    REG_BPL4PTH,
    REG_BPL4PTL,
    REG_BPL5PTH,
    REG_BPL5PTL,
    REG_BPL6PTH,
    REG_BPL6PTL,

    SPR_HPOS0,
    SPR_HPOS1,
    SPR_HPOS2,
    SPR_HPOS3,
    SPR_HPOS4,
    SPR_HPOS5,
    SPR_HPOS6,
    SPR_HPOS7
}
RegChangeID;

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
    SPR_DMA_DATA
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
    uint16_t bplcon0;
    uint16_t dmacon;
    uint16_t diwstrt;
    uint16_t diwstop;
    uint16_t ddfstrt;
    uint16_t ddfstop;

    int16_t bpl1mod;
    int16_t bpl2mod;
    uint8_t bpu;

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
    int accuracy;
}
BlitterConfig;

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
