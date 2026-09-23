// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "CmdQueue.h"

namespace vamiga {

void
CmdQueue::put(const Command &cmd)
{
    {   SYNCHRONIZED

        logmsg(LOG_CMD, "%s [%llx]\n", CmdEnum::key(cmd.type), cmd.value);

        if (!queue.isFull()) {

            queue.write(cmd);

            // Counted here, not before the check: a command that was dropped
            // is never carried out, and wait() would sit there forever.
            pending++;

        } else {
            logmsg(LOG_WARN, "Command lost: %s [%llx]\n", CmdEnum::key(cmd.type), cmd.value);
        }

        empty = false;
    }
}

void
CmdQueue::done()
{
    if (--pending == 0) pending.notify_all();
}

void
CmdQueue::wait()
{
    for (isize n; (n = pending.load()) != 0; ) pending.wait(n);
}

bool
CmdQueue::poll(Command &cmd)
{
    if (empty) return false;

    {   SYNCHRONIZED

        cmd = queue.read();
        empty = queue.isEmpty();

        return true;
    }
}

}

