// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "PixelEngineTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class DeniseRev : long
{
    OCS,           // Revision 8362R8
    ECS            // Revision 8373      (only partially supported)
};

struct DeniseRevEnum : Reflection<DeniseRevEnum, DeniseRev>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DeniseRev::ECS);
    
    static const char *_key(DeniseRev value)
    {
        switch (value) {
                
            case DeniseRev::OCS:          return "OCS";
            case DeniseRev::ECS:          return "ECS";
        }
        return "???";
    }
    static const char *help(DeniseRev value)
    {
        switch (value) {
                
            case DeniseRev::OCS:          return "MOS 8362R8";
            case DeniseRev::ECS:          return "MOS 8373 (Super Denise)";
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
    
    // Hides certain bitplanes
    u8 hiddenBitplanes;
    
    // Hides certain sprites
    u8 hiddenSprites;
    
    // Hides certain graphics layers
    u16 hiddenLayers;
    
    // Alpha channel value for hidden layers
    u8 hiddenLayerAlpha;
    
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
    // Extracted information from DIWSTRT and DIWSTOP
    isize hstrt;
    isize hstop;
    isize vstrt;
    isize vstop;
}
ViewPortInfo;

typedef struct
{
    bool ecs;
    
    u16 bplcon0;
    u16 bplcon1;
    u16 bplcon2;
    i16 bpu;
    u16 bpldat[6];
    
    u16 diwstrt;
    u16 diwstop;
    ViewPortInfo viewport;
    
    u16 joydat[2];
    u16 clxdat;
    
    u16 colorReg[32];
    u32 color[32];
    
    SpriteInfo sprite[8];
}
DeniseInfo;

}
