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

// #define ACTIVE_WAITING // REMOVE ASAP

namespace util {

void
Wakeable::waitForWakeUp()
{
    static util::Time now, past;

#ifndef ACTIVE_WAITING

    std::unique_lock<std::mutex> lock(condMutex);
    auto timeout = std::chrono::system_clock::now();
    timeout += std::chrono::milliseconds(400);
    condVar.wait_until(lock, timeout, [this]{ return ready; });
    ready = false;

#else

    // REMOVEASAP
    auto base = util::Time::now();

    while (ready == false) {

        auto now = util::Time::now();
        if ((now - base).asMilliseconds() > 200) break;
    }
    now = util::Time::now();
    past = now;

    ready = false;

#endif
}

void
Wakeable::wakeUp()
{
#ifndef ACTIVE_WAITING
    {
        std::lock_guard<std::mutex> lock(condMutex);
        ready = true;
    }
    condVar.notify_one();

#else

    static util::Time now, past;

    now = util::Time::now();
    past = now;
    
    ready = true;

#endif
}

}
