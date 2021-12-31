// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

//
// Bit masks
//

#ifdef __cplusplus

// BLTCON0 (area mode)
constexpr u16 BLTCON0_ASH     = 0xF000;
constexpr u16 BLTCON0_USEA    = 0x0800;
constexpr u16 BLTCON0_USEB    = 0x0400;
constexpr u16 BLTCON0_USEC    = 0x0200;
constexpr u16 BLTCON0_USED    = 0x0100;
constexpr u16 BLTCON0_LF      = 0x00FF;

// BLTCON0 (line mode)
constexpr u16 BLTCON0_START   = 0xF000;

// BLTCON1 (area mode)
constexpr u16 BLTCON1_BSH     = 0xF000;
constexpr u16 BLTCON1_DOFF    = 0x0080;
constexpr u16 BLTCON1_EFE     = 0x0010;
constexpr u16 BLTCON1_IFE     = 0x0008;
constexpr u16 BLTCON1_FCI     = 0x0004;
constexpr u16 BLTCON1_DESC    = 0x0002;
constexpr u16 BLTCON1_LINE    = 0x0001;

// BLTCON1 (line mode)
constexpr u16 BLTCON1_TEXTURE = 0xF000;
constexpr u16 BLTCON1_SIGN    = 0x0040;
constexpr u16 BLTCON1_SUD     = 0x0010;
constexpr u16 BLTCON1_SUL     = 0x0008;
constexpr u16 BLTCON1_AUL     = 0x0004;
constexpr u16 BLTCON1_SING    = 0x0002;

#endif

//
// Structures
//

typedef struct
{
    isize accuracy;
}
BlitterConfig;

typedef struct
{
    u16 bltcon0;
    u16 bltcon1;
    u16 ash;
    u16 bsh;
    u16 minterm;
    u32 bltapt;
    u32 bltbpt;
    u32 bltcpt;
    u32 bltdpt;
    u16 bltafwm;
    u16 bltalwm;
    i16 bltamod;
    i16 bltbmod;
    i16 bltcmod;
    i16 bltdmod;
    u16 aold;
    u16 bold;
    u16 anew;
    u16 bnew;
    u16 ahold;
    u16 bhold;
    u16 chold;
    u16 dhold;
    u16 barrelAin;
    u16 barrelAout;
    u16 barrelBin;
    u16 barrelBout;
    u16 mintermOut;
    u16 fillIn;
    u16 fillOut;
    bool bbusy;
    bool bzero;
    bool firstWord;
    bool lastWord;
    bool fci;
    bool fco;
    bool fillEnable;
    bool storeToDest;
}
BlitterInfo;
