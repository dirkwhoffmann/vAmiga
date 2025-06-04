// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HttpServer.h"

namespace vamiga {

class PromServer final : public HttpServer {

public:

    using HttpServer::HttpServer;

protected:

    PromServer& operator= (const PromServer& other) {

        HttpServer::operator = (other);
        return *this;
    }


    //
    // Methods from CoreObject
    //

protected:

    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from RemoteServer
    //

public:

    void main() override;


    //
    // Handling requests
    //

    // Generate a response
    string respond(const httplib::Request& request);
};

}
