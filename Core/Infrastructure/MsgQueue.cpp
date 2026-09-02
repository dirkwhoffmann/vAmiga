// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "MsgQueue.h"

namespace vamiga {

using namespace utl;

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

void
MsgQueue::put(const Message &msg, const string &str)
{
    if (enabled) {

        SYNCHRONIZED

        logmsg(LOG_MSG, "%s [%llx]\n", MsgEnum::key(msg.type), msg.value);

        if (listener) {

            // Send the message immediately if a listener has been registered.
            // The string only needs to outlive this call, so a local copy is
            // sufficient -- unlike the queued case below, nothing needs to
            // keep it alive afterwards.
            Message m = msg;
            m.str = str.c_str();
            callback(listener, m);

        } else if (!queue.isFull()) {

            // Coalesce with the oldest pending message if it has the same
            // type, so a burst of identical messages doesn't fill up the
            // queue while nobody is draining it.
            auto *current = queue.currentAddr();

            if (current->type == msg.type) {

                auto r = queue.begin();
                *current = msg;
                attachments[r] = str;
                queue.elements[r].str = attachments[r].c_str();

            } else {

                auto w = queue.end();
                queue.write(msg);
                attachments[w] = str;
                queue.elements[w].str = attachments[w].c_str();
            }

        } else {

            logmsg(LOG_WARN, "Message lost: %s [%llx]\n", MsgEnum::key(msg.type), msg.value);
        }
    }
}

void
MsgQueue::put(Msg type, i64 payload, i64 payload2, const string &str)
{
    put( Message { .type = type, .value = payload, .value2 = payload2 }, str);
}

void
MsgQueue::put(Msg type, const string &payload)
{
    put( Message { .type = type }, payload);
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
