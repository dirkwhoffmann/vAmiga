// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Concurrency.h"

namespace vamiga {

class Wakeable
{
    static constexpr auto timeout = std::chrono::milliseconds(100);

    std::mutex condMutex;
    std::condition_variable condVar;
    bool ready = false;

public:

    void waitForWakeUp(util::Time timeout);
    void wakeUp();
};

}
