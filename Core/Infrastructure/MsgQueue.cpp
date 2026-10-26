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

        /* Send all pending messages.
         *
         * Only when there is somewhere to send them: passing no callback is
         * how a client detaches, and draining into it would call through a
         * null pointer. The backlog is left in the queue for whoever
         * attaches next.
         */
        if (callback) {

            while (!entries.empty()) {

                Entry &entry = entries.front();

                Message m = entry.message;
                m.str = entry.str.c_str();
                callback(listener, m, entry.exception);

                entries.pop_front();
            }
        }
    }
}

bool
MsgQueue::get(Message &msg, std::exception_ptr &exception)
{
    if (!enabled) return false;

    {   SYNCHRONIZED

        if (entries.empty()) return false;

        Entry &entry = entries.front();

        // Keep the attached string alive until the next call
        current = std::move(entry.str);
        msg = entry.message;
        msg.str = current.c_str();
        exception = entry.exception;

        entries.pop_front();
        return true;
    }
}

void
MsgQueue::put(const Message &msg, const string &str, std::exception_ptr exception)
{
    if (enabled) {

        SYNCHRONIZED

        logmsg(LOG_MSG, "%s [%llx]\n", MsgEnum::key(msg.type), msg.value);

        if (listener) {

            // Send the message immediately if a listener has been registered
            Message m = msg;
            m.str = str.c_str();
            callback(listener, m, exception);

        } else if (isize(entries.size()) < CAPACITY) {

            entries.push_back(Entry { msg, str, exception });

        } else {

            logmsg(LOG_WARN, "Message lost: %s [%llx]\n", MsgEnum::key(msg.type), msg.value);
        }
    }
}

void
MsgQueue::put(Msg type, i64 payload, i64 payload2, const string &str, std::exception_ptr exception)
{
    put( Message { .type = type, .value = payload, .value2 = payload2 }, str, exception);
}

void
MsgQueue::put(Msg type, const string &payload)
{
    put( Message { .type = type }, payload);
}

void
MsgQueue::put(Msg type, std::exception_ptr exception)
{
    put( Message { .type = type }, "", exception );
}

void
MsgQueue::put(Msg type, CpuMsg payload, const string &str)
{
    put( Message { .type = type, .cpu = payload }, str);
}

void
MsgQueue::put(Msg type, ConfigMsg payload, const string &str, std::exception_ptr exception)
{
    put( Message { .type = type, .config = payload }, str, exception);
}

void
MsgQueue::put(Msg type, BeamMsg payload, const string &str)
{
    put( Message { .type = type, .beam = payload }, str);
}

void
MsgQueue::put(Msg type, DriveMsg payload, const string &str)
{
    put( Message { .type = type, .drive = payload }, str);
}

void
MsgQueue::put(Msg type, HdcMsg payload, const string &str)
{
    put( Message { .type = type, .hdc = payload }, str);
}

void
MsgQueue::put(Msg type, ScriptMsg payload, const string &str)
{
    put( Message { .type = type, .script = payload }, str);
}

void
MsgQueue::put(Msg type, ViewportMsg payload, const string &str)
{
    put( Message { .type = type, .viewport = payload }, str);
}

void
MsgQueue::put(Msg type, SnapshotMsg payload, const string &str)
{
    put( Message { .type = type, .snapshot = payload }, str);
}

}
