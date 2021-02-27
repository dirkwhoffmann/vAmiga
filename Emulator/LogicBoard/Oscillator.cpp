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
#ifdef __MACH__
    mach_timebase_info(&tb);
#endif
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

u64
Oscillator::nanos()
{
#ifdef __MACH__
    
    return abs_to_nanos(mach_absolute_time());
    
#else
    
    struct timespec ts;
    
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * 1000000000 + ts.tv_nsec;
    
#endif
}

void
Oscillator::restart()
{
    clockBase = agnus.clock;
    timeBase = utl::Time::now();
}

void
Oscillator::synchronize()
{
    // Only proceed if we are not running in warp mode
    if (warpMode) return;
    
    auto clockDelta  = agnus.clock - clockBase;
    auto elapsedTime = utl::Time((i64)(clockDelta * 1000 / masterClockFrequency));
    auto targetTime  = timeBase + elapsedTime;
    auto now         = utl::Time::now();
    
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
        waitUntil(targetTime.asNanoseconds());
    }
}

void
Oscillator::waitUntil(u64 deadline)
{
#ifdef __MACH__
    
    mach_wait_until(nanos_to_abs(deadline));
    
#else

    req.tv_sec = 0;
    req.tv_nsec = (long)deadline - (long)nanos();
    nanosleep(&req, &rem);
    
#endif
}

#ifdef __MACH__
mach_timebase_info_data_t Oscillator::tb;
#endif
