// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "PromServer.h"
#include "Emulator.h"
#include "httplib.h"
#include <thread>

namespace vamiga {

void
PromServer::_initialize()
{
    config.transport = TransportProtocol::HTTP;
}

void
PromServer::_dump(Category category, std::ostream &os) const
{
    RemoteServer::_dump(category, os);
}

Transport &
PromServer::transport()
{
    switch (config.transport) {

        case TransportProtocol::HTTP: return http;

        default:
            fatalError;
    }
}

const Transport &
PromServer::transport() const
{
    return const_cast<PromServer *>(this)->transport();
}

bool
PromServer::isSupported(TransportProtocol protocol) const
{
    return protocol == TransportProtocol::HTTP;
}

void
PromServer::didReceive(const httplib::Request &req, httplib::Response &res)
{
    res.set_content(respond(req), "text/plain");
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

    {   auto metrics = emulator.metrics.current();

        translate("vamiga_cpu_load", "",
                  "gauge", metrics.cpuLoad,
                  {{"component","emulator"}});

        translate("vamiga_fps", "",
                  "gauge", metrics.fps,
                  {{"component","emulator"}});

        translate("vamiga_resyncs", "",
                  "gauge", metrics.resyncs,
                  {{"component","emulator"}});
    }

    {   auto metrics = agnus.metrics.current();

        translate("vamiga_activity_copper", "",
                  "gauge", metrics.copperActivity,
                  {{"component","agnus"}});

        translate("vamiga_activity_blitter", "",
                  "gauge", metrics.blitterActivity,
                  {{"component","agnus"}});

        translate("vamiga_activity_disk", "",
                  "gauge", metrics.diskActivity,
                  {{"component","agnus"}});

        translate("vamiga_activity_disk", "",
                  "gauge", metrics.diskActivity,
                  {{"component","agnus"}});

        translate("vamiga_activity_audio", "",
                  "gauge", metrics.audioActivity,
                  {{"component","agnus"}});

        translate("vamiga_activity_sprite", "",
                  "gauge", metrics.spriteActivity,
                  {{"component","agnus"}});

        translate("vamiga_activity_bitplane", "",
                  "gauge", metrics.bitplaneActivity,
                  {{"component","agnus"}});
    }

    {   auto metrics_a = ciaa.metrics.current();
        auto metrics_b = ciab.metrics.current();

        translate("vamiga_ciaa_idle_sec", "",
                  "gauge", metrics_a.idleCycles,
                  {{"component","ciaa"}});
        translate("vamiga_ciab_idle_sec", "",
                  "gauge", metrics_b.idleCycles,
                  {{"component","ciab"}});

        translate("vamiga_cia_idle_sec_total", "",
                  "gauge", metrics_a.totalCycles,
                  {{"component","ciaa"}});
        translate("vamiga_cia_idle_sec_total", "",
                  "gauge", metrics_b.totalCycles,
                  {{"component","ciab"}});

        translate("vamiga_cia_idle_percentage", "",
                  "gauge", metrics_a.idlePercentage,
                  {{"component","ciaa"}});
        translate("vamiga_cia_idle_percentage", "",
                  "gauge", metrics_b.idlePercentage,
                  {{"component","ciab"}});
    }

    {   auto metrics = mem.metrics.current();

        translate("vamiga_mem_accesses", "",
                  "gauge", metrics.chipReads.accumulated,
                  {{"component","memory"},{"location","chip_ram"},{"type","read"}});
        translate("vamiga_mem_accesses", "",
                  "gauge", metrics.chipWrites.accumulated,
                  {{"component","memory"},{"location","chip_ram"},{"type","write"}});

        translate("vamiga_mem_accesses", "",
                  "gauge", metrics.slowReads.accumulated,
                  {{"component","memory"},{"location","slow_ram"},{"type","read"}});
        translate("vamiga_mem_accesses", "",
                  "gauge", metrics.slowWrites.accumulated,
                  {{"component","memory"},{"location","slow_ram"},{"type","write"}});

        translate("vamiga_mem_accesses", "",
                  "gauge", metrics.fastReads.accumulated,
                  {{"component","memory"},{"location","fast_ram"},{"type","read"}});
        translate("vamiga_mem_accesses", "",
                  "gauge", metrics.fastWrites.accumulated,
                  {{"component","memory"},{"location","fast_ram"},{"type","write"}});

        translate("vamiga_mem_accesses", "",
                  "gauge", metrics.kickReads.accumulated,
                  {{"component","memory"},{"location","rom"},{"type","read"}});
        translate("vamiga_mem_accesses", "",
                  "gauge", metrics.kickWrites.accumulated,
                  {{"component","memory"},{"location","rom"},{"type","write"}});
    }

    {   auto metrics = audioPort.metrics.current();

        translate("vamiga_audio_buffer_exceptions", "",
                  "gauge", metrics.bufferOverflows,
                  {{"component","audio"},{"type","overflow"}});
        translate("vamiga_audio_buffer_exceptions", "",
                  "gauge", metrics.bufferUnderflows,
                  {{"component","audio"},{"type","underflow"}});

        translate("vamiga_audio_samples", "",
                  "gauge", metrics.consumedSamples,
                  {{"component","audio"},{"type","consumed"}});
        translate("vamiga_audio_samples", "",
                  "gauge", metrics.producedSamples,
                  {{"component","audio"},{"type","produced"}});
        translate("vamiga_audio_samples", "",
                  "gauge", metrics.idleSamples,
                  {{"component","audio"},{"type","idle"}});

        translate("vamiga_audio_fill_level", "",
                  "gauge", metrics.fillLevel,
                  {{"component","audio"}});
    }

    return output.str();
}

}
