// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "PromServer.h"
#include "httplib.h"

namespace vamiga {

void
PromServer::start()
{
    if (state == SRV_STATE_OFF) {

        debug(true, "start()");

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

        state = SRV_STATE_CONNECTED;
    }
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
    string metrics =
    "# HELP example_metric A simple example metric\n"
    "# TYPE example_metric counter\n"
    "example_metric{label=\"value1\"} 42\n"
    "example_metric{label=\"value2\"} 73\n\n"

    "# HELP another_metric Another simple example metric\n"
    "# TYPE another_metric gauge\n"
    "another_metric 123.45\n";

    return metrics;
}

}
