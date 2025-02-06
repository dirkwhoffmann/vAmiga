// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "CmdQueue.h"

namespace vamiga {

void
CmdQueue::put(const Command &cmd)
{
    {   SYNCHRONIZED

        debug(CMD_DEBUG, "%s [%llx]\n", CmdTypeEnum::key(cmd.type), cmd.value);

        if (!queue.isFull()) {
            queue.write(cmd);
        } else {
            warn("Command lost: %s [%llx]\n", CmdTypeEnum::key(cmd.type), cmd.value);
        }

        empty = false;
    }
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

