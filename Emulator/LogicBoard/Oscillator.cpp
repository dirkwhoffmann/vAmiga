// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Oscillator.h"

#include "Agnus.h"
#include "Chrono.h"

Oscillator::Oscillator(Amiga& ref) : AmigaComponent(ref)
{

}
    
const char *
Oscillator::getDescription() const
{
#ifdef __MACH__
    return "Oscillator (Mac)";
#else
    return "Oscillator (Generic)";
#endif
}

void
Oscillator::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {
        
    }
}

void
Oscillator::restart()
{
    clockBase = agnus.clock;
    timeBase = Time::now();
}

void
Oscillator::synchronize()
{
    // Only proceed if we are not running in warp mode
    if (warpMode) return;
    
    auto clockDelta  = agnus.clock - clockBase;
    auto elapsedTime = Time((i64)(clockDelta * 1000 / masterClockFrequency));
    auto targetTime  = timeBase + elapsedTime;
    auto now         = Time::now();
    
    // Check if we're running too slow ...
    if (now > targetTime) {
        
        // Check if we're completely out of sync...
        if ((now - targetTime).asMilliseconds() > 200) {
            
            // warn("The emulator is way too slow (%f).\n", (now - targetTime).asSeconds());
            restart();
            return;
        }
    }
    
    // Check if we're running too fast ...
    if (now < targetTime) {
        
        // Check if we're completely out of sync...
        if ((targetTime - now).asMilliseconds() > 200) {
            
            warn("The emulator is way too fast (%f).\n", (targetTime - now).asSeconds());
            restart();
            return;
        }
        
        // See you soon...
        targetTime.sleepUntil();
    }
}
