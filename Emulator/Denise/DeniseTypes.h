// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DENISE_T_INC
#define _DENISE_T_INC

#include "AmigaConstants.h"

//
// Enumerations
//

typedef enum : long
{
    DENISE_8362R8, // OCS
    DENISE_8373    // ECS (not supported yet)
}
DeniseRevision;

inline bool isDeniseRevision(long value)
{
    return value >= 0 && value <= DENISE_8373;
}

inline const char *DeniseRevisionName(DeniseRevision type)
{
    assert(isDeniseRevision(type));

    switch (type) {
        case DENISE_8362R8: return "DENISE_8362R8";
        case DENISE_8373:   return "DENISE_8373";
        default:            return "???";
    }
}

typedef enum : long
{
    COLOR_PALETTE = 0,
    BLACK_WHITE_PALETTE,
    PAPER_WHITE_PALETTE,
    GREEN_PALETTE,
    AMBER_PALETTE,
    SEPIA_PALETTE
}
Palette;

inline bool isPalette(long value) {
    return value >= COLOR_PALETTE && value <= SEPIA_PALETTE;
}

typedef enum
{
    MODE_SPF = 0, // Single-playfield mode
    MODE_DPF,     // Dual-playfield mode
    MODE_HAM      // Hold-and-modify mode
}
DrawingMode;

inline bool isDrawingMode(long value) {
    return value >= MODE_SPF && value <= MODE_HAM;
}


//
// Structures
//

typedef struct
{
    i32 *data;
    bool longFrame;
    bool interlace;
}
ScreenBuffer;

typedef struct
{
    // Number of lines the sprite was armed
    u16 height;

    // Extracted information from SPRxPOS and SPRxCTL
    i16 hstrt;
    i16 vstrt;
    i16 vstop;
    bool attach;
    
    // Upper 16 color register (recorded where the observed sprite starts)
    u16 colors[16];

    // Recorded data words of the observed sprite
    u64 data[VPOS_CNT];
}
SpriteInfo;

typedef struct
{
    // Emulated chip model
    DeniseRevision revision;

    // Turns sprite emulation on or off
    bool emulateSprites;

    // Checks for sprite-sprite collisions
    bool clxSprSpr;

    // Checks for sprite-playfield collisions
    bool clxSprPlf;

    // Checks for playfield-playfield collisions
    bool clxPlfPlf;
}
DeniseConfig;

typedef struct
{
    u16 bplcon0;
    u16 bplcon1;
    u16 bplcon2;
    i16 bpu;
    u16 bpldat[6];

    u16 diwstrt;
    u16 diwstop;
    i16 diwHstrt;
    i16 diwHstop;
    i16 diwVstrt;
    i16 diwVstop;

    u16 joydat[2];
    u16 clxdat;

    u16 colorReg[32];
    u32 color[32];
}
DeniseInfo;

typedef struct
{
    long spriteLines;
}
DeniseStats;

#endif
