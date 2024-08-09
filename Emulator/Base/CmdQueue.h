// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CmdQueueTypes.h"
#include "CoreObject.h"
#include "Synchronizable.h"
#include "RingBuffer.h"
#include <atomic>

namespace vamiga {

/// Command queue
class CmdQueue final : CoreObject, Synchronizable {

    /// Ring buffer storing all pending commands
    util::RingBuffer <Cmd, 128> queue;

public:
    
    /// Indicates if the queue is empty
    std::atomic<bool> empty = true;

    //
    // Methods
    //

private:

    const char *objectName() const override { return "CmdQueue"; }


    //
    // Managing the queue
    //

public:

    // Sends a command
    void put(const Cmd &cmd);

    // Polls a command
    bool poll(Cmd &cmd);
};

}
