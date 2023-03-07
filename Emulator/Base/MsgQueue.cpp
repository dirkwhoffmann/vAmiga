// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MsgQueue.h"

namespace vamiga {

void
MsgQueue::setListener(const void *listener, Callback *callback)
{
    {   SYNCHRONIZED
        
        this->listener = listener;
        this->callback = callback;
        
        // Send all pending messages
        while (!queue.isEmpty()) {

            Message &msg = queue.read();
            callback(listener, msg);
        }
        
        put(MSG_REGISTER);
    }
}

void
MsgQueue::put(const Message &msg)
{
    {   SYNCHRONIZED

        debug(QUEUE_DEBUG, "%s [%llx %llx %llx %llx]\n",
              MsgTypeEnum::key(msg.type),
              msg.data1, msg.data2, msg.data3, msg.data4);

        // Send the message immediately if a lister has been registered
        if (listener) { callback(listener, msg); return; }

        // Otherwise, store it in the ring buffer
        queue.write(msg);
    }
}

void
MsgQueue::put(MsgType type, i64 d1, i64 d2, i64 d3, i64 d4)
{
    Message msg = { .type = type, .data1 = d1, .data2 = d2, .data3 = d3, .data4 = d4 };
    put(msg);
}

bool
MsgQueue::get(Message &msg)
{
    {   SYNCHRONIZED

        if (queue.isEmpty()) return false;

        msg = queue.read();
        return true;
    }
}

}
