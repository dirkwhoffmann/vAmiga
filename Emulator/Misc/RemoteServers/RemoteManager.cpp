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
#include "Scheduler.h"
#include "IOUtils.h"

RemoteManager::RemoteManager(Amiga& ref) : SubComponent(ref)
{
}

RemoteManager::~RemoteManager()
{
    debug(SRV_DEBUG, "Shutting down\n");

    try {
        
        serServer.stop();
        rshServer.stop();
        gdbServer.stop();
        
    } catch(...) { }
}

void
RemoteManager::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    for (auto server : servers) {
        
        auto name = server->getDescription();
        auto port = server->getPort();
        
        os << tab(string(name));
        
        if (server->isOff()) {
            os << "Off" << std::endl;
        } else if (server->isStarting()) {
            os << "Port " << dec(port) << " (launching)" << std::endl;
        } else if (server->isListening()) {
            os << "Port " << dec(port) << " (listening)" << std::endl;
        } else if (server->isConnected()) {
            os << "Port " << dec(port) << " (connected)" << std::endl;
        } else {
            fatalError;
        }
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
    assert(scheduler.id[SLOT_SRV] == SRV_DAEMON);
        
    // Run the launch daemon
    for (auto &server : servers) {

        if (server->isStarting() && server->canStart()) {
            
            // Try to switch the server on
            debug(SRV_DEBUG, "Trying to start pending server\n");
            server->start();
        }
    }
    
    // Schedule next event
    scheduler.scheduleInc <SLOT_SRV> (SEC(0.5), SRV_DAEMON);
}
