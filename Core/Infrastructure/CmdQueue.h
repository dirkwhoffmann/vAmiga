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
#include "utl/concurrency.h"
#include "utl/storage.h"
#include <atomic>

namespace vamiga {

/// Command queue
class CmdQueue final : public CoreObject, Synchronizable {

    /// Ring buffer storing all pending commands
    utl::RingBuffer <Command, 256> queue;

public:
    
    /// Indicates if the queue is empty
    std::atomic<bool> empty = true;

    /* Number of commands that have been sent but not carried out yet.
     *
     * Counted rather than derived from the queue, because a command stops
     * being queued the moment it is polled and only takes effect a moment
     * later. Anyone waiting for a command to have happened has to wait for
     * that second moment, which is what done() reports.
     */
    std::atomic<isize> pending = 0;

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
    void put(const Command &cmd);

    // Polls a command
    bool poll(Command &cmd);

    // Reports that a polled command has been carried out
    void done();

    /* Blocks until every command sent so far has been carried out.
     *
     * Only ever called from the user thread, and only while the emulator
     * thread is actually running: a suspended or halted thread polls nothing,
     * so the wait would never end.
     */
    void wait();
};

}
