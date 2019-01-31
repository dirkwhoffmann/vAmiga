// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Paula::Paula()
{
    setDescription("Paula");
    
    // Register sub components
    registerSubcomponents(vector<HardwareComponent *> {
        
        &audioUnit
    });
    
    // Register snapshot items
    SnapshotItem items[] = {
        
        { NULL,                0,                          0 }};
    
    registerSnapshotItemsOld(items, sizeof(items));
    
}

void
Paula::_powerOn()
{
    
}

void
Paula::_setWarp(bool value)
{
    /* Warping has the unavoidable drawback that audio playback gets out of
     * sync. To cope with this issue, we ramp down the volume when entering
     * warp mode and fade in smoothly when warping ends.
     */
    if (value) {
        
        audioUnit.rampDown();
        
    } else {
        
        audioUnit.rampUp();
        audioUnit.alignWritePtr();
    }
}
