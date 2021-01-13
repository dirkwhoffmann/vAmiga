// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "MessageQueue.h"

void
MessageQueue::addListener(const void *listener, Callback *func)
{
    synchronized {
        listeners.push_back(std::pair <const void *, Callback *> (listener, func));
    }
    
    // Distribute all pending messages
    Message msg;
    while ((msg = get()).type != MSG_NONE) {
        propagate(msg);
    }

    put(MSG_REGISTER);
}

void
MessageQueue::removeListener(const void *listener)
{
    put(MSG_UNREGISTER);
    
    synchronized {
        
        for (auto it = listeners.begin(); it != listeners.end(); it++) {
            if (it->first == listener) { listeners.erase(it);  break; }
        }
    }
}

Message
MessageQueue::get()
{ 
	Message result;
    
    synchronized {
        
        if (queue.isEmpty()) {
            result = { MSG_NONE, 0 };
        } else {
            result = queue.read();
        }
    }
    
    return result;
}
 
void
MessageQueue::put(MsgType type, i64 data)
{
    synchronized {
                        
        debug (QUEUE_DEBUG, "%s [%lld]\n", MsgTypeEnum::key(type), data);
        
        // Delete the oldest message if the queue overflows
        if (queue.isFull()) (void)queue.read();
    
        // Write data
        Message msg = { type, data };
        queue.write(msg);
        
        // Serve registered callbacks
        propagate(msg);
    }
}

void
MessageQueue::dump()
{
    for (int i = queue.begin(); i != queue.end(); i = queue.next(i)) {
        msg("%02d", i); dump(queue.elements[i]);
    }
}

void
MessageQueue::dump(const Message &msg)
{
    msg("%s [%ld]\n", MsgTypeEnum::key(msg.type), msg.data);
}

void
MessageQueue::propagate(const Message &msg) const
{
    for (auto i = listeners.begin(); i != listeners.end(); i++) {
        i->second(i->first, msg.type, msg.data);
    }
}
