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
PromServer::_dump(Category category, std::ostream &os) const
{
    using namespace util;

    HttpServer::_dump(category, os);
}

string
PromServer::respond(const httplib::Request& request)
{
    std::ostringstream output;
    
    auto translate = [&](const string& metric,
                         const string& help,
                         const string& type,
                         auto value,
                         const std::map<string, string>& labels = {}) {
        
        if (!help.empty()) {
            output << "# HELP " << metric << " " << help << "\n";
        }
        if (!metric.empty()) {
            output << "# TYPE " << metric << " " << type << "\n";
        }
        output << metric;
        if (!labels.empty()) {
            output << "{";
            bool first = true;
            for (const auto& [key,val] : labels) {
                if (!first) output << ",";
                output << key << "=\"" << val << "\"";
                first = false;
            }
            output << "}";
        }
        output << " " << value << "\n\n";
    };
    
    output << std::fixed << std::setprecision(4);
    
    {   auto stats = emulator.getStats();
        
        translate("vamiga_cpu_load", "",
                  "gauge", stats.cpuLoad,
                  {{"component","emulator"}});
        
        translate("vamiga_fps", "",
                  "gauge", stats.fps,
                  {{"component","emulator"}});
        
        translate("vamiga_resyncs", "",
                  "gauge", stats.resyncs,
                  {{"component","emulator"}});
    }
    
    {   auto stats = agnus.getStats();
        
        translate("vamiga_activity_copper", "",
                  "gauge", stats.copperActivity,
                  {{"component","agnus"}});
        
        translate("vamiga_activity_blitter", "",
                  "gauge", stats.blitterActivity,
                  {{"component","agnus"}});
        
        translate("vamiga_activity_disk", "",
                  "gauge", stats.diskActivity,
                  {{"component","agnus"}});
        
        translate("vamiga_activity_disk", "",
                  "gauge", stats.diskActivity,
                  {{"component","agnus"}});
        
        translate("vamiga_activity_audio", "",
                  "gauge", stats.audioActivity,
                  {{"component","agnus"}});
        
        translate("vamiga_activity_sprite", "",
                  "gauge", stats.spriteActivity,
                  {{"component","agnus"}});
        
        translate("vamiga_activity_bitplane", "",
                  "gauge", stats.bitplaneActivity,
                  {{"component","agnus"}});
    }
    
    {   auto stats_a = ciaa.getStats();
        auto stats_b = ciaa.getStats();
        
        translate("vamiga_ciaa_idle_sec", "",
                  "gauge", stats_a.idleCycles,
                  {{"component","ciaa"}});
        translate("vamiga_ciab_idle_sec", "",
                  "gauge", stats_b.idleCycles,
                  {{"component","ciab"}});
        
        translate("vamiga_cia_idle_sec_total", "",
                  "gauge", stats_a.totalCycles,
                  {{"component","ciaa"}});
        translate("vamiga_cia_idle_sec_total", "",
                  "gauge", stats_b.totalCycles,
                  {{"component","ciab"}});
        
        translate("vamiga_cia_idle_percentage", "",
                  "gauge", stats_a.idlePercentage,
                  {{"component","ciaa"}});
        translate("vamiga_cia_idle_percentage", "",
                  "gauge", stats_b.idlePercentage,
                  {{"component","ciab"}});
    }
    
    {   auto stats = mem.getStats();

        translate("vamiga_mem_accesses", "",
                  "gauge", stats.chipReads.accumulated,
                  {{"component","memory"},{"location","chip_ram"},{"type","read"}});
        translate("vamiga_mem_accesses", "",
                  "gauge", stats.chipWrites.accumulated,
                  {{"component","memory"},{"location","chip_ram"},{"type","write"}});

        translate("vamiga_mem_accesses", "",
                  "gauge", stats.slowReads.accumulated,
                  {{"component","memory"},{"location","slow_ram"},{"type","read"}});
        translate("vamiga_mem_accesses", "",
                  "gauge", stats.slowWrites.accumulated,
                  {{"component","memory"},{"location","slow_ram"},{"type","write"}});

        translate("vamiga_mem_accesses", "",
                  "gauge", stats.fastReads.accumulated,
                  {{"component","memory"},{"location","fast_ram"},{"type","read"}});
        translate("vamiga_mem_accesses", "",
                  "gauge", stats.fastWrites.accumulated,
                  {{"component","memory"},{"location","fast_ram"},{"type","write"}});

        translate("vamiga_mem_accesses", "",
                  "gauge", stats.kickReads.accumulated,
                  {{"component","memory"},{"location","rom"},{"type","read"}});
        translate("vamiga_mem_accesses", "",
                  "gauge", stats.kickWrites.accumulated,
                  {{"component","memory"},{"location","rom"},{"type","write"}});
    }
    
    {   auto stats = audioPort.getStats();
        
        translate("vamiga_audio_buffer_exceptions", "",
                  "gauge", stats.bufferOverflows,
                  {{"component","audio"},{"type","overflow"}});
        translate("vamiga_audio_buffer_exceptions", "",
                  "gauge", stats.bufferUnderflows,
                  {{"component","audio"},{"type","underflow"}});
        
        translate("vamiga_audio_samples", "",
                  "gauge", stats.consumedSamples,
                  {{"component","audio"},{"type","consumed"}});
        translate("vamiga_audio_samples", "",
                  "gauge", stats.producedSamples,
                  {{"component","audio"},{"type","produced"}});
        translate("vamiga_audio_samples", "",
                  "gauge", stats.idleSamples,
                  {{"component","audio"},{"type","idle"}});
        
        translate("vamiga_audio_fill_level", "",
                  "gauge", stats.fillLevel,
                  {{"component","audio"}});
    }
        
    return output.str();
}

void
PromServer::main()
{
    try {

        // Create the HTTP server
        if (!srv) srv = new httplib::Server();

        // Define the "/metrics" endpoint where Prometheus will scrape metrics
        srv->Get("/metrics", [this](const httplib::Request& req, httplib::Response& res) {
            
            switchState(SrvState::CONNECTED);
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
