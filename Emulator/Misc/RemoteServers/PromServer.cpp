// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "PromServer.h"
#include "Emulator.h"
#include "httplib.h"
#include <thread>

namespace vamiga {

void
PromServer::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    HttpServer::_dump(category, os);
}

string
PromServer::respond(const httplib::Request& request)
{
    auto emuStats = emulator.getStats();

    string metrics =
    "# HELP cpu_load Host CPU load\n"
    "# TYPE cpu_load gauge\n"
    "cpu_load " + std::to_string(emuStats.cpuLoad) + "\n\n"

    "# HELP fps Frames per second\n"
    "# TYPE fps gauge\n"
    "fps " + std::to_string(emuStats.fps) + "\n\n";

    return metrics;
}

void
PromServer::main()
{
    try {

        // Create the HTTP server
        if (!srv) srv = new httplib::Server();

        // Define the "/metrics" endpoint where Prometheus will scrape metrics
        srv->Get("/metrics", [this](const httplib::Request& req, httplib::Response& res) {
            res.set_content(respond(req), "text/plain");
        });

        // Start the server to listen on localhost
        debug(SRV_DEBUG, "Starting Prometheus data provider\n");
        srv->listen("localhost", (int)config.port);

    } catch (std::exception &err) {

        debug(SRV_DEBUG, "Server thread interrupted\n");
        handleError(err.what());
    }
}

}
