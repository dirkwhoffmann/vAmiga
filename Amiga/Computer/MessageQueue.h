// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _MESSAGE_QUEUE_INC
#define _MESSAGE_QUEUE_INC

#include "AmigaObject.h"
#include <map>

using namespace std;

class MessageQueue : public AmigaObject {
    
private:
    
    // Maximum number of queued messages
    const static size_t capacity = 64;
    
    // Message ring buffer
    Message queue[capacity];
    
    // The ring buffers read and write pointers
    int r = 0;
    int w = 0;
    
    // Mutex for streamlining parallel read and write accesses
    pthread_mutex_t lock;
    
    // A list of all registered listeners
    map <const void *, Callback *> listeners;
    
public:
    
    // Constructor and destructor
    MessageQueue();
    ~MessageQueue();
    
    // Registers a listener together with it's callback function.
    void addListener(const void *listener, Callback *func);
    
    // Removes a listener.
    void removeListener(const void *listener);
    
    // Returns the next pending message, or NULL if the queue is empty
    Message getMessage();
    
    // Writes a message into the queue and propagates it to all listeners.
    void putMessage(MessageType type, uint64_t data = 0);
    
private:
    
    // Propagates a single message to all registered listeners.
    void propagateMessage(Message *msg);
};

#endif
