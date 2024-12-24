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
PromServer::start()
{
    if (state == SRV_STATE_OFF) {

        debug(true, "start()\n");

        try {

            // Start the server in a separate thread
            serverThread = std::thread(&PromServer::startServer, this);

        } catch (std::exception &err) {

            debug(SRV_DEBUG, "Server thread interrupted: %s\n", err.what());
        }

        std::cout << "Started\n";
        state = SRV_STATE_CONNECTED;
    }
}

void
PromServer::startServer()
{
    debug(true, "startServer()\n");

    // Create an HTTP server
    httplib::Server svr;

    // Define the "/metrics" endpoint where Prometheus will scrape metrics
    svr.Get("/metrics", [this](const httplib::Request& req, httplib::Response& res) {

        // Generate the metrics as a string
        std::string metrics = generate_metrics();

        // Set the response headers to indicate it's plain text
        res.set_content(metrics, "text/plain");
    });

    // Start the server to listen on localhost, port 8080
    std::cout << "Starting Prometheus data provider on http://localhost:8080/metrics\n";
    svr.listen("localhost", 8080);
}

void
PromServer::stop()
{
    if (state == SRV_STATE_CONNECTED) {

        debug(true, "stop()");

        state = SRV_STATE_OFF;
    }
}

// Function to generate some example metrics in the Prometheus format
string
PromServer::generate_metrics()
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

}
