// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
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
    }
}

void
MsgQueue::put(const Message &msg)
{
    if (enabled) {

        SYNCHRONIZED

        debug(MSG_DEBUG, "%s [%llx]\n", MsgTypeEnum::key(msg.type), msg.value);

        if (listener) {

            // Send the message immediately if a lister has been registered
            callback(listener, msg); return;

        } else {

            // Otherwise, store it in the ring buffer
            if (!queue.isFull()) {
                queue.write(msg);
            } else {
                warn("Message lost: %s [%llx]\n", MsgTypeEnum::key(msg.type), msg.value);
            }
        }
    }
}

void
MsgQueue::put(MsgType type, i64 payload)
{
    put( Message { .type = type, .value = payload } );
}

void
MsgQueue::put(MsgType type, CpuMsg payload)
{
    put( Message { .type = type, .cpu = payload } );
}

void
MsgQueue::put(MsgType type, DriveMsg payload)
{
    put( Message { .type = type, .drive = payload } );
}

void
MsgQueue::put(MsgType type, HdcMsg payload)
{
    put( Message { .type = type, .hdc = payload } );
}

void
MsgQueue::put(MsgType type, ScriptMsg payload)
{
    put( Message { .type = type, .script = payload } );
}

void
MsgQueue::put(MsgType type, ViewportMsg payload)
{
    put( Message { .type = type, .viewport = payload } );
}

void
MsgQueue::put(MsgType type, SnapshotMsg payload)
{
    put( Message { .type = type, .snapshot = payload } );
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
