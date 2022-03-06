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

void
MsgQueue::setListener(const void *listener, Callback *callback)
{
    synchronized {
        
        this->listener = listener;
        this->callback = callback;
        
        // Send all pending messages
        while (!queue.isEmpty()) {
            Message &msg = queue.read();
            callback(listener, msg.type, msg.data);
        }
        put(MSG_REGISTER);
    }
}

void
MsgQueue::put(MsgType type, i64 val)
{
    {   SYNCHRONIZED
        
        debug(QUEUE_DEBUG, "%s [%lld]\n", MsgTypeEnum::key(type), val);
        
        // Send the message immediately if a lister has been registered
        if (listener) { callback(listener, type, val); return; }
        
        // Otherwise, store it in the ring buffer
        Message msg = { type, val }; queue.write(msg);
    }
}

void
MsgQueue::put(MsgType type, i16 val1, i16 val2, i16 val3, i16 val4)
{
    u64 u1 = (u16)(val1);
    u64 u2 = (u16)(val2);
    u64 u3 = (u16)(val3);
    u64 u4 = (u16)(val4);
    
    put(type, (i64)(u4 << 48 | u3 << 32 | u2 << 16 | u1));
}
