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
#include <stdexcept>

namespace vamiga {

void
CmdQueue::put(const Command &cmd, const string &str, Completion completion)
{
    bool lost = false;
    bool announce = false;

    {   SYNCHRONIZED

        logmsg(LOG_CMD, "%s [%llx]\n", CmdEnum::key(cmd.type), cmd.value);

        if (isize(entries.size()) < CAPACITY) {

            entries.push_back(Entry { cmd, str, std::move(completion) });

        } else {

            logmsg(LOG_WARN, "Command lost: %s [%llx]\n", CmdEnum::key(cmd.type), cmd.value);
            lost = true;
            announce = !overflowing;
            overflowing = true;
        }

        empty = false;
    }

    // Don't leave a waiting sender hanging
    if (lost && completion) {
        completion(std::make_exception_ptr(std::runtime_error("Command queue is full")));
    }

    // Tell the application once per overflow
    if (announce && onFull) onFull();
}

bool
CmdQueue::poll(Command &cmd, Completion &completion)
{
    if (empty) return false;

    {   SYNCHRONIZED

        if (entries.empty()) { empty = true; return false; }

        auto &entry = entries.front();

        // Keep the attached string alive until the next call
        current = std::move(entry.str);
        cmd = entry.cmd;
        cmd.str = current.c_str();
        completion = std::move(entry.completion);

        entries.pop_front();
        if (isize(entries.size()) < CAPACITY) overflowing = false;
        empty = entries.empty();

        return true;
    }
}

}

