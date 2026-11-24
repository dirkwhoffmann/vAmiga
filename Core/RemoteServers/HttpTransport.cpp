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
    {   std::lock_guard<std::mutex> lock(srvLock);

        /* Stop the server to exit the main thread. The server thread deletes
         * the server once listen() has returned. Requests in flight are still
         * answered. Their handlers notice that the transport is terminating
         * and stop waiting for the emulator (see RemoteServer::waitFor()).
         */
        if (srv) srv->stop();
    }

    delegate.didDisconnect();
}

void
HttpTransport::main(u16 port, const string &endpoint)
{
    // A stop() may already have landed while this thread was being spawned
    if (terminating()) { switchState(SrvState::OFF); return; }

    auto server = std::make_unique<httplib::Server>();

    // Define the endpoints
    auto handler = [this](const httplib::Request& req, httplib::Response& res) {

        // Requests that sneak in while the server is shutting down are turned away
        if (terminating()) { res.status = 503; return; }

        switchState(SrvState::CONNECTED);
        deliver(req, res);
    };
    server->Get(endpoint, handler);
    server->Post(endpoint, handler);

    // Publish the server, unless a stop request has arrived in the meantime
    {   std::lock_guard<std::mutex> lock(srvLock);

        if (terminating()) { switchState(SrvState::OFF); return; }
        srv = server.get();
    }

    try {

        // Start the server to listen on localhost
        switchState(SrvState::LISTENING);
        server->listen("localhost", port);

    } catch (std::exception &err) {

        logmsg(LOG_SRV, "HTTP server thread interrupted\n");
        delegate.didTerminate(err.what());
    }

    // Withdraw the server before it is deleted
    std::lock_guard<std::mutex> lock(srvLock);
    srv = nullptr;
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
