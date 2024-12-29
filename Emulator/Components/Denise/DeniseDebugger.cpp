// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DeniseDebugger.h"
#include "Emulator.h"

namespace vamiga {

void
DeniseDebugger::_initialize()
{    
    std::memset(spriteInfo, 0, sizeof(spriteInfo));
    std::memset(latchedSpriteInfo, 0, sizeof(latchedSpriteInfo));
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
        
        spriteInfo[nr].hstrt =
        (denise.sprpos[nr] & 0xFF) << 2 |
        (denise.sprctl[nr] & 0x01) << 1 |
        (denise.sprctl[nr] & 0x10) >> 4;
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
    recordDiwH(denise.hstrt, denise.hstop);
    recordDiwV(agnus.sequencer.vstrt, agnus.sequencer.vstop);
    vpChanged = true;
    vpMsgSent = 0;
}

void
DeniseDebugger::recordDiwH(isize hstrt, isize hstop)
{
    if (denise.config.viewportTracking) {

        maxViewPort.hstrt = hstrt;
        maxViewPort.hstop = hstop;

        trace(DIW_DEBUG, "recordDiwH: (%ld,%ld)\n", maxViewPort.hstrt, maxViewPort.hstop);
    }
}

void
DeniseDebugger::recordDiwV(isize vstrt, isize vstop)
{
    if (denise.config.viewportTracking) {

        maxViewPort.vstrt = vstrt;
        maxViewPort.vstop = vstop;
    }
}

void
DeniseDebugger::updateDiwH(isize hstrt, isize hstop)
{
    if (denise.config.viewportTracking) {

        maxViewPort.hstrt = std::min(maxViewPort.hstrt, hstrt);
        maxViewPort.hstop = std::max(maxViewPort.hstop, hstop);

        trace(DIW_DEBUG, "updateDiwH: (%ld,%ld)\n", maxViewPort.hstrt, maxViewPort.hstop);
    }
}

void
DeniseDebugger::updateDiwV(isize vstrt, isize vstop)
{
    if (denise.config.viewportTracking) {

        maxViewPort.vstrt = std::min(maxViewPort.vstrt, vstrt);
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
DeniseDebugger::hsyncHandler(isize vpos)
{
    if (LINE_DEBUG) {

        if (LINE_DEBUG == vpos) {

            auto *ptr = pixelEngine.workingPtr(vpos);

            for (Pixel i = 0; i < HPIXELS; i++) {
                ptr[i] = (i & 1) ? 0xFF0000FF : 0xFFFFFFFF;
            }
        }
    }
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

            latchedMaxViewPort = maxViewPort;
            
            // Notify the GUI if the last message was sent a while ago
            if (std::abs(agnus.clock - vpMsgSent) > MSEC(200)) {

                msgQueue.put(MSG_VIEWPORT, ViewportMsg {
                    i16(latchedMaxViewPort.hstrt),
                    i16(latchedMaxViewPort.vstrt),
                    i16(latchedMaxViewPort.hstop),
                    i16(latchedMaxViewPort.vstop)
                });
                
                vpMsgSent = agnus.clock;
                vpChanged = false;
            }
        }
        
        // Start over with the current viewport
        recordDiwH(denise.hstrt, denise.hstop);
        recordDiwV(agnus.sequencer.vstrt, agnus.sequencer.vstop);
    }
    
    //
    // Sprite tracking
    //
    
    if (emulator.isTracking()) {
        
        // Latch recorded sprite data
        for (isize i = 0; i < 8; i++) {
            
            latchedSpriteInfo[i] = spriteInfo[i];
            spriteInfo[i] = { };
        }
        
        std::memcpy(latchedSpriteData, spriteData, sizeof(spriteData));
    }
}

void
DeniseDebugger::eofHandler()
{
    
}

}
