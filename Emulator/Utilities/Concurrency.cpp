// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Concurrency.h"
#include "Chrono.h"

namespace util {

void
Wakeable::waitForWakeUp(Time timeout)
{
    auto now = std::chrono::system_clock::now();
    auto delay = std::chrono::nanoseconds(timeout.asNanoseconds());

    std::unique_lock<std::mutex> lock(condMutex);
    condVar.wait_until(lock, now + delay, [this]{ return ready; });
    ready = false;

    /* REMOVE ASAP
     static util::Time now, past;
    auto base = util::Time::now();

    while (ready == false) {

        auto now = util::Time::now();
        if ((now - base).asMilliseconds() > 200) break;
    }
    now = util::Time::now();
    past = now;

    ready = false;
    */
}

void
Wakeable::wakeUp()
{
    {
        std::lock_guard<std::mutex> lock(condMutex);
        ready = true;
    }
    condVar.notify_one();

    /* REMOVE ASAP
    static util::Time now, past;

    now = util::Time::now();
    past = now;
    
    ready = true;
    */
}

}
