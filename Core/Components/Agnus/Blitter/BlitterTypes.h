// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {

//
// Bit masks
//

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


//
// Structures
//

/// Configuration options of the Blitter.
typedef struct
{
    isize accuracy;                         ///< Blitter emulation accuracy
}
BlitterConfig;

/// State of the Blitter (registers and progress).
typedef struct
{
    u16 bltcon0;                            ///< Blitter control register 0 (BLTCON0)
    u16 bltcon1;                            ///< Blitter control register 1 (BLTCON1)
    u16 ash;                                ///< Barrel shifter distance for source A
    u16 bsh;                                ///< Barrel shifter distance for source B
    u16 minterm;                            ///< Minterm (logic function) selected by BLTCON0
    u32 bltapt;                             ///< Pointer to source A
    u32 bltbpt;                             ///< Pointer to source B
    u32 bltcpt;                             ///< Pointer to source C
    u32 bltdpt;                             ///< Pointer to destination D
    u16 bltafwm;                            ///< First word mask for source A (BLTAFWM)
    u16 bltalwm;                            ///< Last word mask for source A (BLTALWM)
    i16 bltamod;                            ///< Modulo for source A
    i16 bltbmod;                            ///< Modulo for source B
    i16 bltcmod;                            ///< Modulo for source C
    i16 bltdmod;                            ///< Modulo for destination D
    u16 aold;                               ///< Previous word of source A
    u16 bold;                               ///< Previous word of source B
    u16 anew;                               ///< Current word of source A
    u16 bnew;                               ///< Current word of source B
    u16 ahold;                              ///< Holding register for source A
    u16 bhold;                              ///< Holding register for source B
    u16 chold;                              ///< Holding register for source C
    u16 dhold;                              ///< Holding register for destination D
    u16 barrelAin;                          ///< Input of the barrel shifter for source A
    u16 barrelAout;                         ///< Output of the barrel shifter for source A
    u16 barrelBin;                          ///< Input of the barrel shifter for source B
    u16 barrelBout;                         ///< Output of the barrel shifter for source B
    u16 mintermOut;                         ///< Output of the minterm logic
    u16 fillIn;                             ///< Input of the fill logic
    u16 fillOut;                            ///< Output of the fill logic
    bool bbusy;                             ///< Blitter busy flag
    bool bzero;                             ///< Blitter zero flag
    bool firstWord;                         ///< The first word of a line is being processed
    bool lastWord;                          ///< The last word of a line is being processed
    bool fci;                               ///< Fill carry input
    bool fco;                               ///< Fill carry output
    bool fillEnable;                        ///< Fill mode is enabled
    bool storeToDest;                       ///< The result is written to destination D
}
BlitterInfo;

}
