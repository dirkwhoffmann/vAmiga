// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "HttpTransport.h"
#include "httplib.h"

namespace vamiga {

HttpTransport::~HttpTransport()
{
    stop();
}

void
HttpTransport::disconnect()
{
    if (srv) {

        // Stop the server to exit the main thread
        srv->stop();
        delete srv;
        srv = nullptr;
    }

    delegate.didDisconnect();
}

void
HttpTransport::main(u16 port, const string &endpoint)
{
    try {

        // Create the HTTP server
        if (!srv) srv = new httplib::Server();

        // Define the endpoints
        srv->Get(endpoint, [this](const httplib::Request& req, httplib::Response& res) {

            switchState(SrvState::CONNECTED);
            deliver(req, res);
        });

        srv->Post(endpoint, [this](const httplib::Request& req, httplib::Response& res) {

            switchState(SrvState::CONNECTED);
            deliver(req, res);
        });

        // Start the server to listen on localhost
        switchState(SrvState::LISTENING);
        srv->listen("localhost", port);

    } catch (std::exception &err) {

        logmsg(LOG_SRV, "HTTP server thread interrupted\n");
        delegate.didTerminate(err.what());
    }
}

void
HttpTransport::deliver(const httplib::Request &req, httplib::Response &res)
{
    /* HTTP traffic bypasses send(). The response is written into the response
     * object by the delegate, so both directions are recorded here.
     */
    record(TrafficDirection::RECEIVED,
           req.method + " " + req.path + (req.body.empty() ? "" : "\n" + req.body));

    delegate.didReceive(req, res);

    record(TrafficDirection::SENT, res.body);
}

}
