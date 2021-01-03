// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _MESSAGE_QUEUE_H
#define _MESSAGE_QUEUE_H

#include "HardwareComponent.h"

class MessageQueue : public HardwareComponent {
        
    // Maximum number of queued messages
    const static size_t capacity = 64;
    
    // Ring buffer storing all pending messages
    Message queue[capacity];
    
    // The ring buffer's read and write pointers
    int r = 0;
    int w = 0;
        
    // List of all registered listeners
    map <const void *, Callback *> listeners;
    
    
    //
    // Constructing
    //
    
public:
    
    MessageQueue();
    
    const char *getDescription() override { return "MessageQueue"; }

private:
    
    void _reset(bool hard) override { };

    
    //
    // Serializing
    //
    
private:
    
    // template <class T> void applyToPersistentItems(T& worker) { }
    // template <class T> void applyToHardResetItems(T& worker) { }
    // template <class T> void applyToResetItems(T& worker) { }

    size_t _size() override { return 0; }
    size_t _load(u8 *buffer) override { return 0; }
    size_t _save(u8 *buffer) override { return 0; }
    
    
    //
    // Managing the queue
    //
    
public:
    
    // Registers a listener together with it's callback function
    void addListener(const void *listener, Callback *func);
    
    // Unregisters a listener
    void removeListener(const void *listener);
    
    // Returns the next pending message, or nullptr if the queue is empty
    Message get();
    
    // Writes a message into the queue and propagates it to all listeners
    void put(MsgType type, u64 data = 0);
    
private:
    
    // Used by 'put' to propagates a single message to all registered listeners
    void propagate(Message *msg);
};

#endif
