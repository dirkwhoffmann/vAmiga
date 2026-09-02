// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Transport.h"
#include "Socket.h"

namespace vamiga {

class TcpTransport : public Transport {

    // Sockets
    Socket listener;
    Socket connection;

    using Transport::Transport;

    TcpTransport& operator=(const TcpTransport& other) {

        Transport::operator=(other);
        return *this;
    }

    //
    // Methods from Transport
    //

public:

    // See StdioTransport::~StdioTransport() -- stops the server thread
    // before this object's socket members and Transport's serverThread are
    // torn down.
    ~TcpTransport();

    virtual void disconnect() override;
    void main(u16 port, const string &endpoint = "") override;

private:

    // Inner loops (called from main)
    void mainLoop(u16 port);
    void sessionLoop();


    //
    // Sending
    //

public:

    // Sends a packet
    void send(const string &payload) override;
};

}
