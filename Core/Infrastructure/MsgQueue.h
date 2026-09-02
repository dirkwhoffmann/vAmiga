// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MsgQueueTypes.h"
#include "CoreObject.h"
#include "utl/abilities/Synchronizable.h"
#include "utl/storage.h"

namespace vamiga {

class MsgQueue final : CoreObject, utl::Synchronizable {

    // vAmiga carries a heavier message load than other ports in this family
    // (e.g., VirtualC64), so its ring buffer is kept larger.
    static constexpr isize CAPACITY = 4096;

    // Ring buffer storing all pending messages
    utl::RingBuffer <Message, CAPACITY> queue;

    // Attached string objects
    string attachments[CAPACITY];

    // Used by WASM builds to pass additional parameters
    std::vector<string> payload;

    // The registered listener
    const void *listener = nullptr;

    // The registered callback function
    Callback *callback = nullptr;

    // If disabled, no messages will be stored
    bool enabled = true;


    //
    // Constructing
    //
    
    //
    // Methods from CoreObject
    //

public:

    const char *objectName() const override { return "MsgQueue"; }

    
    //
    // Managing the queue
    //
    
public:

    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    // Registers a listener together with it's callback function
    void setListener(const void *listener, Callback *func);

    // Disables the message queue
    void disable() { enabled = false; }

    // Reads a message
    bool get(Message &msg);

    // Sends a message
    void put(const Message &msg, const string &str = "");
    void put(Msg type, i64 payload = 0, i64 payload2 = 0, const string &str = "");
    void put(Msg type, const string &payload);
    void put(Msg type, CpuMsg payload);
    void put(Msg type, DriveMsg payload);
    void put(Msg type, HdcMsg payload);
    void put(Msg type, ScriptMsg payload);
    void put(Msg type, ViewportMsg payload);
    void put(Msg type, SnapshotMsg payload);

    // Gets or sets the payload (used by WASM builds)
    string getPayload(isize index);
    void setPayload(const std::vector<string> &payload);
};

}
