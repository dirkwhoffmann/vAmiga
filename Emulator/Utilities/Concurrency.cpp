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

namespace util {

void
Wakeable::waitForWakeUp()
{
    std::unique_lock<std::mutex> lock(condMutex);
    auto timeout = std::chrono::system_clock::now();
    timeout += std::chrono::milliseconds(40);
    condVar.wait_until(lock, timeout, [this]{ return ready; });
    ready = false;
}

void
Wakeable::wakeUp()
{
    {
        std::lock_guard<std::mutex> lock(condMutex);
        ready = true;
    }
    condVar.notify_one();
}

}
