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

struct DeniseRevEnum : Reflectable<DeniseRevEnum, DeniseRev>
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
    // see Agnus::dmaDebugger)
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
    isize vstrt;
    isize vstop;
    bool attach;
    
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
    isize hstop;
    isize vstrt;
    isize vstop;
}
ViewPortInfo;

typedef struct
{
    // bool ecs;
    
    u16 bplcon0;
    u16 bplcon1;
    u16 bplcon2;
    u16 bplcon3;
    u16 bplcon4;
    i16 bpu;
    u16 bpldat[8];
    
    u16 diwstrt;
    u16 diwstop;
    u16 diwhigh;
    ViewPortInfo viewport;

    u16 joydat[2];
    u16 clxdat;
    u16 clxcon;
    u16 clxcon2;
    
    // The AGA color table is 256 entries deep (8 banks of 32, selected by
    // BPLCON3's 3-bit BANK field), but only the first 4 banks (128 entries)
    // are ever meaningfully addressed in practice, so that's all that's
    // cached here -- see SiAmDeniseController's Colors tab, which shows
    // one 32-swatch box per bank.
    u16 colorReg[128];
    u32 color[32];

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
    u16 sprdatb[8];
    u16 sprpos[8];
    u16 sprctl[8];

    SpriteInfo sprite[8];
}
DeniseInfo;

}
