// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Constants.h"
#include "DeniseTypes.h"

class DeniseDebugger: public SubComponent {
    
    // Sprite information recorded in the current frame (constantly changing)
    SpriteInfo spriteInfo[8] = { };
    u64 spriteData[8][VPOS_CNT] = { };

    // Sprite information recorded in the previous frame (shown by the GUI)
    SpriteInfo latchedSpriteInfo[8] = { };
    u64 latchedSpriteData[8][VPOS_CNT] = { };
    
    
    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;


    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "DeniseDebugger"; }
    void _dump(dump::Category category, std::ostream& os) const override { };
    
    
    //
    // Methods from AmigaComponent
    //

private:
    
    void _initialize() override;
    void _reset(bool hard) override;
    
    isize _size() override { return 0; }
    isize _load(const u8 *buffer) override { return 0; }
    isize _save(u8 *buffer) override { return 0; }
    

    //
    // Tracking sprites
    //
    
public:

    void recordSprites(u8 armed);
    void recordSprite(isize x);
    
    
    //
    // Querying recorded data
    //
    
    //
    // Analyzing
    //

public:
    
    SpriteInfo getSpriteInfo(isize nr);
    isize getSpriteHeight(isize nr) const { return latchedSpriteInfo[nr].height; }
    u16 getSpriteColor(isize nr, isize reg) const { return latchedSpriteInfo[nr].colors[reg]; }
    u64 getSpriteData(isize nr, isize line) const { return latchedSpriteData[nr][line]; }

    
    //
    //
    //
    
    void vsyncHandler();
};
