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

namespace httplib { class Server; struct Request; struct Response; }

namespace vamiga {

class HttpTransport : public Transport {

    using Transport::Transport;

protected:

    HttpTransport& operator= (const HttpTransport& other) {

        Transport::operator = (other);
        return *this;
    }

    // A simple third-party HTTP server
    httplib::Server *srv = nullptr;


    //
    // Methods from Transport
    //

public:

    // See StdioTransport::~StdioTransport() -- stops the server thread
    // before this object's srv pointer and Transport's serverThread are
    // torn down.
    ~HttpTransport();

    void disconnect() override;
    void main(u16 port, const string &endpoint) override;
    void send(const string &payload) override { }

private:

    // Records a request/response pair and forwards it to the delegate
    void deliver(const httplib::Request &req, httplib::Response &res);
};

}
