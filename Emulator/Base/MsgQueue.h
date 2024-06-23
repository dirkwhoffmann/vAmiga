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
#include "Synchronizable.h"
#include "RingBuffer.h"

namespace vamiga {

class MsgQueue final : CoreObject, Synchronizable {

    // Ring buffer storing all pending messages
    util::RingBuffer <Message, 512> queue;

    // The registered listener
    const void *listener = nullptr;
    
    // The registered callback function
    Callback *callback = nullptr;
    
    
    //
    // Constructing
    //

    // using SubComponent::SubComponent;

    
    //
    // Methods from CoreObject
    //

public:

    const char *objectName() const override { return "MsgQueue"; }

    // const Descriptions &getDescriptions() const override { return descriptions; }

    //
    // Methods from Configurable
    //
    
    // const ConfigOptions &getOptions() const override { return options; }


    //
    // Managing the queue
    //
    
public:
    
    // Registers a listener together with it's callback function
    void setListener(const void *listener, Callback *func);

    // Sends a message
    void put(const Message &msg);
    void put(MsgType type, i64 payload = 0);
    void put(MsgType type, CpuMsg payload);
    void put(MsgType type, DriveMsg payload);
    void put(MsgType type, HdcMsg payload);
    void put(MsgType type, ScriptMsg payload);
    void put(MsgType type, ViewportMsg payload);
    void put(MsgType type, SnapshotMsg payload);

    // Reads a message
    bool get(Message &msg);
};

}
