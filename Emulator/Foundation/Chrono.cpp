// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Chrono.h"

#ifdef __MACH__

//
// macOS
//

static struct mach_timebase_info timebaseInfo()
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
