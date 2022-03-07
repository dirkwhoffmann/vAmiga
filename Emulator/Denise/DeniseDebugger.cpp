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

void
DeniseDebugger::resetDIWTracker()
{
    recordDIW(denise.diwstrt, denise.diwstop);
    vpChanged = true;
    vpMsgSent = 0;
}

void
DeniseDebugger::recordDIW(u16 diwstrt, u16 diwstop)
{
    if (denise.config.viewportTracking) {
        
        maxViewPort.hstrt = LO_BYTE(diwstrt) > 2 ? LO_BYTE(diwstrt) : 2;
        maxViewPort.vstrt = HI_BYTE(diwstrt);
        maxViewPort.hstop = LO_BYTE(diwstop) | 0x100;
        maxViewPort.vstop = HI_BYTE(diwstop) | ((diwstop & 0x8000) ? 0 : 0x100);
     }
}

void
DeniseDebugger::updateDIW(u16 diwstrt, u16 diwstop)
{
    if (denise.config.viewportTracking) {
        
        isize hstrt = LO_BYTE(diwstrt) > 2 ? LO_BYTE(diwstrt) : 2;
        isize vstrt = HI_BYTE(diwstrt);
        isize hstop = LO_BYTE(diwstop) | 0x100;
        isize vstop = HI_BYTE(diwstop) | ((diwstop & 0x8000) ? 0 : 0x100);
                
        maxViewPort.hstrt = std::min(maxViewPort.hstrt, hstrt);
        maxViewPort.vstrt = std::min(maxViewPort.vstrt, vstrt);
        maxViewPort.hstop = std::max(maxViewPort.hstop, hstop);
        maxViewPort.vstop = std::max(maxViewPort.vstop, vstop);
    }
}

SpriteInfo
DeniseDebugger::getSpriteInfo(isize nr)
{
    SYNCHRONIZED
    return latchedSpriteInfo[nr];
}

void
DeniseDebugger::vsyncHandler()
{
    //
    // Viewport tracking
    //
    
    if (denise.config.viewportTracking) {
        
        // Compare the recorded viewport with the previous one
        vpChanged |=
        latchedMaxViewPort.hstrt != maxViewPort.hstrt ||
        latchedMaxViewPort.hstop != maxViewPort.hstop ||
        latchedMaxViewPort.vstrt != maxViewPort.vstrt ||
        latchedMaxViewPort.vstop != maxViewPort.vstop;
        
        // Take action if the viewport has changed
        if (vpChanged) {
            
            /*
            msg("Old viewport: (%ld,%ld) - (%ld,%ld)\n",
                  latchedMaxViewPort.hstrt,
                  latchedMaxViewPort.vstrt,
                  latchedMaxViewPort.hstop,
                  latchedMaxViewPort.vstop);
            msg("New viewport: (%ld,%ld) - (%ld,%ld)\n",
                  maxViewPort.hstrt,
                  maxViewPort.vstrt,
                  maxViewPort.hstop,
                  maxViewPort.vstop);
            */
            
            latchedMaxViewPort = maxViewPort;
            
            // Notify the GUI if the last message was sent a while ago
            if (abs(agnus.clock - vpMsgSent) > MSEC(200)) {
                
                msgQueue.put(MSG_VIEWPORT,
                             i16(latchedMaxViewPort.hstrt),
                             i16(latchedMaxViewPort.vstrt),
                             i16(latchedMaxViewPort.hstop),
                             i16(latchedMaxViewPort.vstop));
                
                vpMsgSent = agnus.clock;
                vpChanged = false;
            }
        }
        
        // Start over with the current viewport
        recordDIW(denise.diwstrt, denise.diwstop);
    }
    
    //
    // Sprite tracking
    //
    
    if (amiga.inDebugMode()) {
        
        // Latch recorded sprite data
        for (isize i = 0; i < 8; i++) {
            
            latchedSpriteInfo[i] = spriteInfo[i];
            spriteInfo[i] = { };
        }
        
        std::memcpy(latchedSpriteData, spriteData, sizeof(spriteData));
    }
}
