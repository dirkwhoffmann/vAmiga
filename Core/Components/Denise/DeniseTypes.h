// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class DeniseRev : long
{
    OCS,           // MOS 8362R8
    ECS,           // MOS 8373      (partially supported)
    AGA            // MOS 4203      (partially supported)
};

struct DeniseRevEnum : utl::Reflectable<DeniseRevEnum, DeniseRev>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DeniseRev::AGA);
    
    static const char *_key(DeniseRev value)
    {
        switch (value) {
                
            case DeniseRev::OCS:          return "OCS";
            case DeniseRev::ECS:          return "ECS";
            case DeniseRev::AGA:          return "AGA";
        }
        return "???";
    }
    static const char *help(DeniseRev value)
    {
        switch (value) {
                
            case DeniseRev::OCS:          return "MOS 8362R8";
            case DeniseRev::ECS:          return "MOS 8373 (Super Denise)";
            case DeniseRev::AGA:          return "MOS 4203 (Lisa)";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    // Emulated chip model
    DeniseRev revision;
    
    // Informs the GUI about viewport changes
    bool viewportTracking;
    
    // Number of frames to be skipped in warp mode
    isize frameSkipping;
    
    /* Merges each pair of super hires pixels into their average colour.
     * ECS super hires emits two pixels where a hires display has one, and a
     * real monitor blurs them together; a framebuffer sampled at hires
     * resolution otherwise keeps only one of the two. Off by default, since
     * it trades detail for fidelity to what a CRT shows.
     */
    bool shresBlend;

    // Hides certain bitplanes
    u8 hiddenBitplanes;
    
    // Hides certain sprites
    u8 hiddenSprites;
    
    // Hides certain graphics layers (only takes effect in XRayMode::XRAY_LAYERS,
    // see Agnus::xray)
    u16 hiddenLayers;

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
    // Number of lines the sprite was armed
    isize height;
    
    // Extracted information from SPRxPOS and SPRxCTL
    isize hstrt;
    isize vstrt;                            ///< Vertical start position
    isize vstop;                            ///< Vertical stop position
    bool attach;                            ///< The sprite is attached to its neighbor
    
    // Upper 16 color registers (at the time the observed sprite starts)
    u16 colors[16];
    
    // Latched sprite data
    const u64 *data;
}
SpriteInfo;

typedef struct
{
    // Extracted information from DIWSTRT and DIWSTOP.
    // hstrt/hstop are measured in super-hires pixels (see Denise::hstrt).
    isize hstrt;
    isize hstop;                            ///< Right edge of the visible area
    isize vstrt;                            ///< Top edge of the visible area
    isize vstop;                            ///< Bottom edge of the visible area
}
ViewPortInfo;

typedef struct
{
    // bool ecs;
    
    u16 bplcon0;                            ///< Bitplane control register 0 (BPLCON0)
    u16 bplcon1;                            ///< Bitplane control register 1 (BPLCON1, scroll)
    u16 bplcon2;                            ///< Bitplane control register 2 (BPLCON2, priorities)
    u16 bplcon3;                            ///< Bitplane control register 3 (BPLCON3, ECS/AGA)
    u16 bplcon4;                            ///< Bitplane control register 4 (BPLCON4, AGA)
    i16 bpu;                                ///< Number of active bitplanes
    u16 bpldat[8];                          ///< Bitplane data registers (BPLxDAT)
    
    u16 diwstrt;                            ///< Display window start (DIWSTRT)
    u16 diwstop;                            ///< Display window stop (DIWSTOP)
    u16 diwhigh;                            ///< Display window high bits (DIWHIGH, ECS)
    ViewPortInfo viewport;                  ///< Visible area of the screen

    u16 joydat[2];                          ///< Joystick/mouse data registers (JOYxDAT)
    u16 clxdat;                             ///< Collision data register (CLXDAT)
    u16 clxcon;                             ///< Collision control register (CLXCON)
    u16 clxcon2;                            ///< Collision control register 2 (CLXCON2, AGA)
    
    // The AGA color table is 256 entries deep (8 banks of 32, selected by
    // BPLCON3's 3-bit BANK field), but only the first 4 banks (128 entries)
    // are ever meaningfully addressed in practice, so that's all that's
    // cached here -- see SiAmDeniseController's Colors tab, which shows
    // one 32-swatch box per bank.
    u16 colorReg[128];
    u32 color[32];                          ///< Color palette registers

    // What CPU/Copper reads of COLOR00..COLOR31 (the 32 physical registers,
    // not the full 128-entry table above) actually see right now --
    // Denise::spypeekCOLORxx() itself returns 0 unless the chipset is AGA
    // and RDRAM is set, since OCS/ECS never support reading these back at
    // all. Shown on the Colors tab's own "Registers" box, separate from the
    // swatch grid, which always shows every bank's true color regardless of
    // whether it's currently readable.
    u16 colorRegPeek[32];

    // Raw hardware sprite registers, one entry per sprite -- distinct from
    // sprite[] below, which holds the debugger's own latched/decoded view
    // (armed height, hstrt/vstrt/vstop, attach, colors, pixel data) rather
    // than these four registers as the chipset itself holds them right now.
    // Shown on the Sprites tab's own "Registers" box.
    u16 sprdata[8];
    u16 sprdatb[8];                         ///< Second sprite data registers (SPRxDATB)
    u16 sprpos[8];                          ///< Sprite position registers (SPRxPOS)
    u16 sprctl[8];                          ///< Sprite control registers (SPRxCTL)

    SpriteInfo sprite[8];                   ///< State of the eight sprites
}
DeniseInfo;

}
