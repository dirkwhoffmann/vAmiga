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

namespace httplib { class Server; struct Request; }

namespace vamiga {

class HttpServer : public RemoteServer {

    using RemoteServer::RemoteServer;

protected:

    HttpServer& operator= (const HttpServer& other) {

        RemoteServer::operator = (other);
        return *this;
    }

    // A simple (third-party) HTTP server
    httplib::Server *srv = nullptr;


    //
    // Methods from CoreObject
    //

protected:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from RemoteServer
    //

public:

    virtual void disconnect() override;


    //
    // Running the server
    //

private:

    // The main thread function
    // void main() override;
};

}
