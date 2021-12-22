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
    
    auto printState = [&os] (const RemoteServer &server) {
        
        auto port = server.getPort();
        
        if (server.isConnected()) {
            os << "Connected to client at port " << port << std::endl;
        } else if (server.isListening()) {
            os << "Listening at port " << port << std::endl;
        } else {
            os << "Off" << std::endl;
        }
    };
        
    if (category & dump::State) {
                
        os << tab("Serial port server");
        printState(serServer);
        os << tab("Retro shell server");
        printState(rshServer);
        os << tab("GDB server");
        printState(gdbServer);
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
RemoteManager::defaultPort(ServerType type)
{
    switch (type) {
            
        case SERVER_SER: return 8080;
        case SERVER_RSH: return 8081;
        case SERVER_GDB: return 8082;
            
        default:
            fatalError;
    }
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
