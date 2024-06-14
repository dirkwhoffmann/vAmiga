// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Amiga.h"

namespace vamiga {

class Emulator {

public:

    // The virtual Amiga
    // Amiga main = Amiga(*this, 0);
    Amiga main = Amiga(*this);

    // Later, these functions will be inherited from Thread
    void suspendThread() { main.suspendThread(); }
    void resumeThread() { main.resumeThread(); }


    //
    // Methods from Thread
    //

    /*
private:

    void update() override;
    void computeFrame() override;

    void didPowerOn() override { CoreComponent::powerOn(); }
    void didPowerOff() override { CoreComponent::powerOff(); }
    void didPause() override { CoreComponent::pause(); }
    void didRun() override { CoreComponent::run(); }
    void didHalt() override { CoreComponent::halt(); }
    void didWarpOn() override { CoreComponent::warpOn(); }
    void didWarpOff() override { CoreComponent::warpOff(); }
    void didTrackOn() override { CoreComponent::trackOn(); }
    void didTrackOff() override { CoreComponent::trackOff(); }

public:

    isize missingFrames() const override;
*/
    
    //
    // Warp mode
    //

    // Indicates if the emulator should run in warp mode
    bool shouldWarp() const;
};

}

