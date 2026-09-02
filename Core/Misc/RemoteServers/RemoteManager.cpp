// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "RemoteManager.h"
#include "Agnus.h"
#include "SerialPort.h"
#include "MsgQueue.h"
#include "utl/io.h"
#include <chrono>

namespace vamiga {

isize
TrafficLog::append(ServerType server, TrafficDirection direction, const string &payload)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    entries.push_back(TrafficEntry {

        .nr         = counter,
        .server     = server,
        .direction  = direction,
        .time       = i64(ms),
        .payload    = payload
    });

    // Drop the oldest entries if the log has grown too large
    while (isize(entries.size()) > capacity) entries.pop_front();

    return counter++;
}

std::vector<TrafficEntry>
TrafficLog::read(isize nr) const
{
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<TrafficEntry> result;

    for (const auto &entry : entries) {
        if (entry.nr > nr) result.push_back(entry);
    }

    return result;
}

void
TrafficLog::clear()
{
    std::lock_guard<std::mutex> lock(mutex);

    entries.clear();
}

RemoteManager::RemoteManager(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<CoreComponent *> {
        
        &rshServer,
        &rpcServer,
        &gdbServer,
        &promServer,
        &serServer
    };

    info.bind([this] { return cacheInfo(); } );
}

void
RemoteManager::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

    if (category == Category::State) {

        os << "Remote server status: " << std::endl << std::endl;

        for (auto server : servers) {
            
            auto name = server->objectName();
            auto port = server->config.port;
            
            os << tab(string(name));
            
            if (server->isOff()) {
                os << "Off" << std::endl;
            } else {
                os << "Port " << dec(port);
                os << " (" << SrvStateEnum::key(server->getState()) << ")" << std::endl;
            }
        }
    }
}

RemoteManagerInfo
RemoteManager::cacheInfo() const
{
    RemoteManagerInfo info;

    info.rshInfo = rshServer.cacheInfo();
    info.rpcInfo = rpcServer.cacheInfo();
    info.gdbInfo = gdbServer.cacheInfo();
    info.promInfo = promServer.cacheInfo();
    info.serInfo = serServer.cacheInfo();

    return info;
}

isize
RemoteManager::numLaunching() const
{
    isize result = 0;
    for (auto &s : servers) if (s->isStarting()) result++;
    return result;
}

isize
RemoteManager::numListening() const
{
    isize result = 0;
    for (auto &s : servers) if (s->isListening()) result++;
    return result;
}

isize
RemoteManager::numConnected() const
{
    isize result = 0;
    for (auto &s : servers) if (s->isConnected()) result++;
    return result;
}

isize
RemoteManager::numErroneous() const
{
    isize result = 0;
    for (auto &s : servers) if (s->isErroneous()) result++;
    return result;
}

void
RemoteManager::update()
{
    if (frame++ % 32 != 0) return;

    auto launchDaemon = [&](RemoteServer &server, ServerConfig &config) {

        if (config.enable) {
            if (server.isOff()) server.waitForLaunch();
        } else {
            if (!server.isOff()) server.stop();
        }

        if (server.canRun()) {
            if (server.isWaiting()) server.start();
        } else {
            if (!server.isOff() && !server.isWaiting()) server.stop();
        }
    };

    launchDaemon(rshServer, rshServer.config);
    launchDaemon(rpcServer, rpcServer.config);
    launchDaemon(gdbServer, gdbServer.config);
    launchDaemon(promServer, promServer.config);
    launchDaemon(serServer, serServer.config);
}

void
RemoteManager::recordTraffic(ServerType server, TrafficDirection direction, const string &payload)
{
    // Ignore empty packets
    if (payload.empty()) return;

    // Store the packet in the traffic log
    auto nr = trafficLog.append(server, direction, payload);

    // Inform the GUI
    auto msg = direction == TrafficDirection::SENT ? Msg::SRV_SEND : Msg::SRV_RECEIVE;
    msgQueue.put(msg, i64(server), i64(nr));
}

void
RemoteManager::send(ServerType server, const string &payload)
{
    switch (server) {

        case ServerType::RSH:  rshServer.send(payload);  break;
        case ServerType::RPC:  rpcServer.send(payload);  break;
        case ServerType::GDB:  gdbServer.send(payload);  break;
        case ServerType::PROM: promServer.send(payload); break;
        case ServerType::SER:  serServer.send(payload);  break;

        default:
            fatalError;
    }
}

}
