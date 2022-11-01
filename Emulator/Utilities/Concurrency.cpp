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
Wakeable::waitForWakeUp()
{
    static util::Time now, past;

    /*
    std::unique_lock<std::mutex> lock(condMutex);
    auto timeout = std::chrono::system_clock::now();
    timeout += std::chrono::milliseconds(400);
    condVar.wait_until(lock, timeout, [this]{ return ready; });
    ready = false;
    */

    // THIS IS JUST FOR TESTING... REMOVE ACTIVE WAITING ASAP
    auto base = util::Time::now();

    while (ready == false) {

        auto now = util::Time::now();
        if ((now - base).asMilliseconds() > 200) break;
    }
    now = util::Time::now();
    past = now;

    ready = false;
}

void
Wakeable::wakeUp()
{
    static util::Time now, past;

    /*
    {
        std::lock_guard<std::mutex> lock(condMutex);
        ready = true;
    }
    condVar.notify_one();
    */

    now = util::Time::now();
    past = now;
    
    ready = true;
}

}
