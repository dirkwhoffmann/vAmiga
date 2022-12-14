// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RemoteManager.h"
#include "IOUtils.h"
#include "Agnus.h"
#include "SerialPort.h"

namespace vamiga {

RemoteManager::RemoteManager(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<AmigaComponent *> {
        
        &serServer,
        &rshServer,
        &gdbServer
    };
}

void
RemoteManager::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Status) {

        os << "Remote server status: " << std::endl << std::endl;

        for (auto server : servers) {
            
            auto name = server->getDescription();
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

i64
RemoteManager::getConfigItem(Option option, long id) const
{
    switch ((ServerType)id) {
            
        case SERVER_SER: return serServer.getConfigItem(option);
        case SERVER_RSH: return rshServer.getConfigItem(option);
        case SERVER_GDB: return gdbServer.getConfigItem(option);

        default:
            fatalError;
    }
}

void
RemoteManager::setConfigItem(Option option, i64 value)
{
    for (auto &server : servers) {
        server->setConfigItem(option, value);
    }
}

void
RemoteManager::setConfigItem(Option option, long id, i64 value)
{
    switch ((ServerType)id) {
            
        case SERVER_SER: serServer.setConfigItem(option, value); break;
        case SERVER_RSH: rshServer.setConfigItem(option, value); break;
        case SERVER_GDB: gdbServer.setConfigItem(option, value); break;

        default:
            fatalError;
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
        serServer.shouldRun() ? serServer._start() : serServer._stop();
    }
    if (rshServer.config.autoRun) {
        rshServer.shouldRun() ? rshServer._start() : rshServer._stop();
    }
    if (gdbServer.config.autoRun) {
        gdbServer.shouldRun() ? gdbServer._start() : gdbServer._stop();
    }

    // Schedule next event
    agnus.scheduleInc <SLOT_SRV> (SEC(0.5), SRV_LAUNCH_DAEMON);
}

}
