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

bool
MsgQueue::get(Message &msg)
{
    if (!enabled) return false;

    {   SYNCHRONIZED

        if (queue.isEmpty()) return false;

        msg = queue.read();
        return true;
    }
}

isize
MsgQueue::get(isize count, Message *buffer)
{
    if (!enabled) return false;

    {   SYNCHRONIZED

        auto max = std::min(queue.count(), count);
        for (isize i = 0; i < max; i++) {
            buffer[i] = queue.read();
        }
        return max;
    }
}

void
MsgQueue::put(const Message &msg)
{
    if (enabled) {

        SYNCHRONIZED

        debug(MSG_DEBUG, "%s [%llx]\n", MsgEnum::key(msg.type), msg.value);

        if (listener) {

            // Send the message immediately if a lister has been registered
            callback(listener, msg);

        } else {

            // Otherwise, store it in the ring buffer
            if (!queue.isFull()) {
                auto *current = queue.currentAddr(); 
                if (current->type == msg.type) {
                    *current = msg; 
                } else {
                    queue.write(msg);
                }
            } else {
                    warn("Message lost: %s [%llx]\n", MsgEnum::key(msg.type), msg.value);
            }
        }
    }
}

void
MsgQueue::put(Msg type, i64 payload, i64 payload2)
{
    put( Message { .type = type, .value = payload, .value2 = payload2 } );
}

void
MsgQueue::put(Msg type, CpuMsg payload)
{
    put( Message { .type = type, .cpu = payload } );
}

void
MsgQueue::put(Msg type, DriveMsg payload)
{
    put( Message { .type = type, .drive = payload } );
}

void
MsgQueue::put(Msg type, HdcMsg payload)
{
    put( Message { .type = type, .hdc = payload } );
}

void
MsgQueue::put(Msg type, ScriptMsg payload)
{
    put( Message { .type = type, .script = payload } );
}

void
MsgQueue::put(Msg type, ViewportMsg payload)
{
    put( Message { .type = type, .viewport = payload } );
}

void
MsgQueue::put(Msg type, SnapshotMsg payload)
{
    put( Message { .type = type, .snapshot = payload } );
}

string
MsgQueue::getPayload(isize index)
{
    {   SYNCHRONIZED

        return (isize)payload.size() > index ? payload[index] : string("");
    }
}

void
MsgQueue::setPayload(const std::vector<string> &payload)
{
    {   SYNCHRONIZED

        this->payload = payload;
    }
}

}
