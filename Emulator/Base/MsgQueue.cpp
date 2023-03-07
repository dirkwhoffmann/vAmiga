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

        debug(QUEUE_DEBUG, "%s [%llx]\n", MsgTypeEnum::key(msg.type), msg.payload);

        // Send the message immediately if a lister has been registered
        if (listener) { callback(listener, msg); return; }

        // Otherwise, store it in the ring buffer
        queue.write(msg);
    }
}

void
MsgQueue::put(MsgType type, i64 data)
{
    Message msg = { .type = type, .payload = data };
    put(msg);
}

void
MsgQueue::put(MsgType type, i32 d1, i32 d2)
{
    Message msg = { .type = type, .long1 = d1, .long2 = d2 };
    put(msg);
}

void
MsgQueue::put(MsgType type, i16 d1, i16 d2, i16 d3, i16 d4)
{
    Message msg = { .type = type, .word1 = d1, .word2 = d2, .word3 = d3, .word4 = d4 };
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
