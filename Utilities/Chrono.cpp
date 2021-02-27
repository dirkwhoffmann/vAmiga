// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Chrono.h"

namespace utl {

#ifdef __MACH__

//
// macOS
//

struct mach_timebase_info timebaseInfo()
{
    struct mach_timebase_info tb;
    mach_timebase_info(&tb);
    return tb;
}

Time
Time::now()
{
    static struct mach_timebase_info tb = timebaseInfo();
    return (i64)mach_absolute_time() * tb.numer / tb.denom;
}

void
Time::sleep()
{
    static struct mach_timebase_info tb = timebaseInfo();
    if (ticks > 0) {
        mach_wait_until(now().asNanoseconds() + (ticks * tb.denom / tb.numer));
    }
}

void
Time::sleepUntil()
{
    static struct mach_timebase_info tb = timebaseInfo();
    mach_wait_until(ticks * tb.denom / tb.numer);
}

#else
    
//
// Linux
//

Time
Time::now()
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (i64)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void
Time::sleep()
{
    if (ticks > 0) {
        req.tv_sec = 0;
        req.tv_nsec = ticks;
        nanosleep(&req, &rem);
    }
}

void
Time::sleepUntil()
{
    (now() - *this).sleep();
}

#endif

//
// All platforms
//

bool
Time::operator==(const Time &rhs) const
{
    return this->asNanoseconds() == rhs.asNanoseconds();
}

bool
Time::operator!=(const Time &rhs) const
{
    return this->asNanoseconds() == rhs.asNanoseconds();
}

bool
Time::operator<=(const Time &rhs) const
{
    return this->asNanoseconds() <= rhs.asNanoseconds();
}

bool
Time::operator>=(const Time &rhs) const
{
    return this->asNanoseconds() >= rhs.asNanoseconds();
}

bool
Time::operator<(const Time &rhs) const
{
    return this->asNanoseconds() < rhs.asNanoseconds();
}

bool
Time::operator>(const Time &rhs) const
{
    return this->asNanoseconds() > rhs.asNanoseconds();
}

Time
Time::operator+(const Time &rhs) const
{
    return Time(this->ticks + rhs.ticks);
}

Time
Time::operator-(const Time &rhs) const
{
    return Time(this->ticks - rhs.ticks);
}

Time
Time::operator*(const int i) const
{
    return Time(i * this->ticks);
}

Time&
Time::operator+=(const Time &rhs)
{
    return *this = *this + rhs;
}

Time&
Time::operator-=(const Time &rhs)
{
    return *this = *this - rhs;
}

Time&
Time::operator*=(const int i)
{
    return *this = *this * i;
}

Time
Time::abs() const
{
    return Time(ticks >= 0 ? ticks : -ticks);
}

Time
Time::diff() const {
    return *this - now();
}

Clock::Clock()
{
    start = Time::now();
}

Time
Clock::getElapsedTime() const
{
    return Time::now() - start;
}

Time
Clock::restart()
{
    Time result = getElapsedTime();
    start = Time::now();
    
    return result;
}

SyncClock::SyncClock(float hz)
{
    slice = Time((i64)(1000000000 / hz));
    target = Time::now() + slice;
}

void
SyncClock::wait()
{
    // How long do we need to sleep?
    Time delay = target.diff();
    
    // Restart the clock if it got out of sync
    if (delay.abs() > slice * 4) restart();

    // Once in a while...
    if (++frames == 60) {
        
        Time delta = stopWatch.restart();
        
        // ...compute the frames per second
        hardFps = frames / delta.asSeconds();
        softFps = 0.5 * softFps + 0.5 * hardFps;
        frames = 0;
        
        // ...compute the CPU load for this thread
        hardLoad = load.asSeconds() / delta.asSeconds();
        softLoad = 0.5 * softLoad + 0.5 * hardLoad;
        load = 0;
        
        /*
        printf("fps: %f (~ %f) load: %2.1f%% (~ %2.1f%%)\n",
               hardFps, softFps, hardLoad * 100, softLoad * 100);
        */
    }
    
    // Sleep
    delay.sleep();
    target = target + slice;
    load = load + slice - delay;
}

void
SyncClock::restart()
{
    printf("Restarting SyncClock\n");
    target = Time::now() + slice;
}

}
