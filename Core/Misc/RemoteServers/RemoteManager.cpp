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
#include "IOUtils.h"
#include "Agnus.h"
#include "SerialPort.h"

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
}

void
RemoteManager::_dump(Category category, std::ostream &os) const
{
    using namespace util;
    
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

void
RemoteManager::cacheInfo(RemoteManagerInfo &result) const
{
    {   SYNCHRONIZED

        info.rshInfo = rshServer.getInfo();
        info.rpcInfo = rpcServer.getInfo();
        info.gdbInfo = gdbServer.getInfo();
        info.promInfo = promServer.getInfo();
        info.serInfo = serServer.getInfo();
    }
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
            if (!server.isOff()) server.stop();
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
