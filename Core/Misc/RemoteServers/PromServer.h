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
#include "HttpTransport.h"

namespace vamiga {

class PromServer final : public RemoteServer {

    HttpTransport http = HttpTransport(*this);

public:

    using RemoteServer::RemoteServer;

    PromServer& operator= (const PromServer& other) {

        RemoteServer::operator = (other);
        return *this;
    }


    //
    // Methods from CoreObject
    //

protected:

    void _initialize() override;
    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from RemoteServer
    //

public:

    bool canRun() override { return true; }
    void start() override { transport().start(config.port, "/metrics"); }

private:

    Transport &transport() override;
    const Transport &transport() const override;
    bool isSupported(TransportProtocol protocol) const override;


    //
    // Methods from TransportDelegate
    //

    void didReceive(const httplib::Request &req, httplib::Response &res) override;


    //
    // Handling requests
    //

    // Generate a response
    string respond(const httplib::Request& request);
};

}
