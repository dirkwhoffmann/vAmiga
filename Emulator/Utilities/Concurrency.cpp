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
    // Wait for wakup signal or time out
    future.wait_for(std::chrono::milliseconds(25));

    // Prepare a new promise/future pair
    promise = std::promise<int>();
    future = promise.get_future();
}

void
Wakeable::wakeUp()
{
    auto state = promise.get_future().wait_for(std::chrono::seconds(0));
    if (state != std::future_status::ready) {
        promise.set_value(true);
    }

    // REMOVE ASAP
    state = promise.get_future().wait_for(std::chrono::seconds(0));
    assert(state == std::future_status::ready);
}

}
