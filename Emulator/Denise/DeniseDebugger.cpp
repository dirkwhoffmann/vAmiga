// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DeniseDebugger.h"
#include "Amiga.h"
// #include "Agnus.h"
// #include "Denise.h"

void
DeniseDebugger::_initialize()
{
    std::memset(spriteInfo, 0, sizeof(spriteInfo));
    std::memset(latchedSpriteInfo, 0, sizeof(latchedSpriteInfo));
}

void
DeniseDebugger::_reset(bool hard)
{
    
}

void
DeniseDebugger::recordSprites(u8 armed)
{
    for (isize i = 0; i < 8; i++) {
        if (GET_BIT(armed, i)) recordSprite(i);
    }
}

void
DeniseDebugger::recordSprite(isize nr)
{
    assert(nr < 8);

    isize line = spriteInfo[nr].height;

    // Record data registers
    spriteData[nr][line] = HI_W_LO_W(denise.sprdatb[nr], denise.sprdata[nr]);

    // Record additional information in sprite line 0
    if (line == 0) {
        
        spriteInfo[nr].hstrt = ((denise.sprpos[nr] & 0xFF) << 1) | (denise.sprctl[nr] & 0x01);
        spriteInfo[nr].vstrt = agnus.sprVStrt[nr];
        spriteInfo[nr].vstop = agnus.sprVStop[nr];
        spriteInfo[nr].attach = IS_ODD(nr) ? GET_BIT(denise.sprctl[nr], 7) : 0;
        
        for (isize i = 0; i < 16; i++) {
            spriteInfo[nr].colors[i] = pixelEngine.getColor(i + 16);
        }
    }
    
    spriteInfo[nr].height = (line + 1) % VPOS_CNT;
}

SpriteInfo
DeniseDebugger::getSpriteInfo(isize nr)
{
    SpriteInfo result;
    synchronized { result = latchedSpriteInfo[nr]; }
    return result;
}

void
DeniseDebugger::vsyncHandler()
{
    // Only proceed if the emulator runs in debug mode
    if (!amiga.inDebugMode()) return;
    
    // Latch recorded data
    for (isize i = 0; i < 8; i++) {
        
        latchedSpriteInfo[i] = spriteInfo[i];
        spriteInfo[i] = { };
        /*
        spriteInfo[i].height = 0;
        spriteInfo[i].vstrt = 0;
        spriteInfo[i].vstop = 0;
        spriteInfo[i].hstrt = 0;
        spriteInfo[i].attach = false;
        */
    }
    
    std::memcpy(latchedSpriteData, spriteData, sizeof(spriteData));
}
