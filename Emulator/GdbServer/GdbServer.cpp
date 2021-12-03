// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "GdbServer.h"
#include "IOUtils.h"
#include "MsgQueue.h"
#include "RetroShell.h"
#include <unistd.h>

GdbServer::GdbServer(Amiga& ref) : SubComponent(ref)
{
}

GdbServerConfig
GdbServer::getDefaultConfig()
{
    GdbServerConfig defaults;

    defaults.port = 8080;
    defaults.verbose = true;

    return defaults;
}

void
GdbServer::resetConfig()
{
    auto defaults = getDefaultConfig();
    
    setConfigItem(OPT_GDB_PORT, defaults.port);
    setConfigItem(OPT_GDB_VERBOSE, defaults.verbose);
}

i64
GdbServer::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_GDB_PORT:       return config.port;
        case OPT_GDB_VERBOSE:    return config.verbose;
            
        default:
            fatalError;
    }
}

void
GdbServer::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_GDB_PORT:
            
            config.port = (isize)value;
            return;
                        
        case OPT_GDB_VERBOSE:
            
            config.verbose = (bool)value;
            return;

        default:
            fatalError;
    }
}

void
GdbServer::start()
{
    debug(GDB_DEBUG, "start\n");
    
    // Error out if the server is already running
    if (this->port) throw VAError(ERROR_GDB_RUNNING);
        
    this->port = config.port;

    // Spawn a new thread
    if (serverThread.joinable()) serverThread.join();
    serverThread = std::thread(&GdbServer::main, this);
}

void
GdbServer::stop()
{
    debug(GDB_DEBUG, "stop\n");
 
    if (port) {
        
        // Trigger an exception inside the server thread
        connection.close();
        listener.close();
        
        // Wait until the server thread has terminated
        serverThread.join();
        
        debug(GDB_DEBUG, "stopped\n");
    }
}

void
GdbServer::main()
{
    debug(GDB_DEBUG, "main\n");

    try {
        
        // Create a port listener
        listener = PortListener(port);
        
        // Wait for a client to connect
        connection = listener.accept();
        
        debug(GDB_DEBUG, "Entering main loop\n");

        while (1) {
            
            auto cmd = connection.recv();
            debug(GDB_DEBUG, "Received %s\n", cmd.c_str());

            if (config.verbose) {
                
                retroShell << cmd << '\n';
                msgQueue.put(MSG_GDB_UPDATE);
            }
            
            // if (cmd == "") break;
        }
        
    } catch (...) {
        
        port = 0;
        connection.close();
        listener.close();
        
        debug(GDB_DEBUG, "Leaving main\n");
    }
}
