// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Constants.h"
#include "DeniseTypes.h"

namespace vamiga {

class DeniseDebugger final : public SubComponent {

    friend class Denise;
    
    Descriptions descriptions = {{

        .type           = DeniseDebuggerClass,
        .name           = "DeniseDebugger",
        .description    = "Denise Debugger",
        .shell          = ""
    }};

    ConfigOptions options = {

    };

    // Largest viewport seen in the current frame (constantly changing)
    ViewPortInfo maxViewPort = { };

    // Largest viewport seen in the previous frame (stable)
    ViewPortInfo latchedMaxViewPort = { };
    
    // Indicates if the viewport has been changed recently
    bool vpChanged = false;
    
    // Remembers when the latest viewport change message was sent
    Cycle vpMsgSent = 0;
    
    // Sprite information recorded in the current frame (constantly changing)
    SpriteInfo spriteInfo[8] = { };
    u64 spriteData[8][VPOS_CNT] = { };

    // Sprite information recorded in the previous frame (stable)
    SpriteInfo latchedSpriteInfo[8] = { };
    u64 latchedSpriteData[8][VPOS_CNT] = { };
    
    
    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;

    DeniseDebugger& operator= (const DeniseDebugger& other) {

        return *this;
    }

    
    //
    // Methods from Serializable
    //
    
private:
    
    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:
    
    void _dump(Category category, std::ostream& os) const override { };
    void _initialize() override;


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Tracking sprites
    //
    
public:

    void recordSprites(u8 armed);
    void recordSprite(isize x);
    
    //
    // Tracking viewport changes
    //
    
    void resetDIWTracker();
    void recordDiwH(isize hstrt, isize hstop);
    void recordDiwV(isize vstrt, isize vstop);
    void updateDiwH(isize hstrt, isize hstop);
    void updateDiwV(isize vstrt, isize vstop);

    
    //
    // Analyzing
    //

public:
    
    SpriteInfo getSpriteInfo(isize nr);
    isize getSpriteHeight(isize nr) const { return latchedSpriteInfo[nr].height; }
    u16 getSpriteColor(isize nr, isize reg) const { return latchedSpriteInfo[nr].colors[reg]; }
    u64 getSpriteData(isize nr, isize line) const { return latchedSpriteData[nr][line]; }

    
    //
    // Handling SYNC events
    //

    void hsyncHandler(isize vpos);
    void vsyncHandler();

    // Called at the end of each frame
    void eofHandler();
};

}
