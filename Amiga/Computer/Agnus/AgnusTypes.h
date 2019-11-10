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
    AGNUS_8367, // OCS Agnus
    AGNUS_8372, // ECS Agnus (up to 1MB Chip Ram)
    AGNUS_8375, // ECS Agnus (up to 2MB Chip Ram)
    AGNUS_CNT
}
AgnusRevision;

inline bool isAgnusRevision(long value)
{
    return value >= 0 && value < AGNUS_CNT;
}

inline const char *AgnusRevisionName(AgnusRevision type)
{
    assert(isAgnusRevision(type));

    switch (type) {
        case AGNUS_8367: return "AGNUS_8367";
        case AGNUS_8372: return "AGNUS_8372";
        case AGNUS_8375: return "AGNUS_8375";
        default:         return "???";
    }
}

typedef struct
{
    // Emulated chip model
    AgnusRevision revision;
}
AgnusConfig;

// Register change identifiers
typedef enum : int32_t
{
    REG_NONE = 0,
    REG_INTREQ,
    REG_INTENA,
    REG_IRQLEVEL,
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

    REG_SPR0DATA,
    REG_SPR1DATA,
    REG_SPR2DATA,
    REG_SPR3DATA,
    REG_SPR4DATA,
    REG_SPR5DATA,
    REG_SPR6DATA,
    REG_SPR7DATA,

    REG_SPR0DATB,
    REG_SPR1DATB,
    REG_SPR2DATB,
    REG_SPR3DATB,
    REG_SPR4DATB,
    REG_SPR5DATB,
    REG_SPR6DATB,
    REG_SPR7DATB,

    REG_SPR0POS,
    REG_SPR1POS,
    REG_SPR2POS,
    REG_SPR3POS,
    REG_SPR4POS,
    REG_SPR5POS,
    REG_SPR6POS,
    REG_SPR7POS,

    REG_SPR0CTL,
    REG_SPR1CTL,
    REG_SPR2CTL,
    REG_SPR3CTL,
    REG_SPR4CTL,
    REG_SPR5CTL,
    REG_SPR6CTL,
    REG_SPR7CTL,

    // DEPRECATED
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
    SPR_DMA_ACTIVE
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
    long count[BUS_OWNER_COUNT];
}
AgnusStats;

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
    // uint16_t bltsize;
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
