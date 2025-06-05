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
        
        &serServer,
        &rshServer,
        &promServer,
        &gdbServer
    };
}

void
RemoteManager::_dump(Category category, std::ostream &os) const
{
    using namespace util;
    
    if (category == Category::Status) {

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
        
        info.numLaunching = numLaunching();
        info.numListening = numListening();
        info.numConnected = numConnected();
        info.numErroneous = numErroneous();
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
RemoteManager::serviceServerEvent()
{
    assert(agnus.id[SLOT_SRV] == SRV_LAUNCH_DAEMON);

    // Run the launch daemon
    if (serServer.config.autoRun) {
        serServer.shouldRun() ? serServer.start() : serServer.stop();
    }
    if (rshServer.config.autoRun) {
        rshServer.shouldRun() ? rshServer.start() : rshServer.stop();
    }
    if (promServer.config.autoRun) {
        promServer.shouldRun() ? promServer.start() : promServer.stop();
    }
    if (gdbServer.config.autoRun) {
        gdbServer.shouldRun() ? gdbServer.start() : gdbServer.stop();
    }

    // Schedule next event
    agnus.scheduleInc <SLOT_SRV> (SEC(0.5), SRV_LAUNCH_DAEMON);
}

}
