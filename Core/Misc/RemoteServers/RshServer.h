// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RemoteServer.h"
#include "Console.h"
#include "TcpTransport.h"

namespace vamiga {

class RshServer final : public RemoteServer, public ConsoleDelegate {

    TcpTransport tcp = TcpTransport(*this);

public:

    using RemoteServer::RemoteServer;

    RshServer& operator= (const RshServer& other) {

        RemoteServer::operator = (other);
        return *this;
    }


    //
    // Methods from CoreObject
    //

private:

    void _initialize() override;
    void _dump(Category category, std::ostream &os) const override;

    //
    // Methods from RemoteServer
    //

    bool canRun() override { return true; }

    Transport &transport() override;
    const Transport &transport() const override;
    bool isSupported(TransportProtocol protocol) const override;

public:

    // Sends a packet, mapping control characters for terminal display
    void send(const string &payload) override;

private:


    //
    // Methods from TransportDelegate
    //

    void didStart() override;
    void didConnect() override;
    void didDisconnect() override;
    void didReceive(const string &payload) override;


    //
    // Methods from ConsoleDelegate
    //

    void didActivate() override;
    void didDeactivate() override;
    void willExecute(const InputLine &input) override;
    void didExecute(const InputLine &input, std::stringstream &ss) override;
    void didExecute(const InputLine &input, std::stringstream &ss, std::exception &e) override;
};

}
