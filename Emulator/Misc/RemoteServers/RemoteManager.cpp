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

RemoteManager::RemoteManager(Amiga& ref) : SubComponent(ref)
{

}

RemoteManager::~RemoteManager()
{
    debug(SRV_DEBUG, "Shutting down RemoteServer\n");
}

void
RemoteManager::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    for (auto server : servers) {
                
        auto name = server->getDescription();
        auto port = server->getPort();

        os << tab(string(name));
        
        if (server->isListening()) {
            os << "Port " << dec(port) << " (listening)" << std::endl;
        } else if (server->isConnected()) {
            os << "Port " << dec(port) << " (connected)" << std::endl;
        } else {
            os << "Off" << std::endl;
        }
    }
}

RemoteServer &
RemoteManager::getServer(ServerType type)
{
    switch (type) {
            
        case SERVER_SER: return serServer;
        case SERVER_RSH: return rshServer;
        case SERVER_GDB: return gdbServer;
            
        default:
            fatalError;
    }
}

isize
RemoteManager::defaultPort(ServerType type) const
{
    switch (type) {
            
        case SERVER_SER: return 8080;
        case SERVER_RSH: return 8081;
        case SERVER_GDB: return 8082;
            
        default:
            fatalError;
    }
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

void
RemoteManager::start(ServerType type, isize port)
{
    auto &server = getServer(type);
    server.start(port);
}

void
RemoteManager::stop(ServerType type)
{
    auto &server = getServer(type);
    server.stop();
}

void
RemoteManager::disconnect(ServerType type)
{
    auto &server = getServer(type);
    server.disconnect();
}
