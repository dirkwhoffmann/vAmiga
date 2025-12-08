// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RemoteManager.h"
#include "Agnus.h"
#include "SerialPort.h"
#include "utl/io.h"

namespace vamiga {

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
                os << " (" << SrvStateEnum::key(server->state) << ")" << std::endl;
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
            if (server.isOff()) server.switchState(SrvState::WAITING);
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
RemoteManager::serviceServerEvent()
{
    // The server event slot is no longer used, as the launch demon is
    // now run in update(). It is safe to remove the SRV_SLOT.
}

}
