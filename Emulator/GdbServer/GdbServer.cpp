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
#include "unistd.h"

GdbServer::GdbServer(Amiga& ref) : SubComponent(ref)
{
}

void
GdbServer::start(isize port)
{
    debug(GDB_DEBUG, "start(%ld)\n", port);
    
    // Error out if the server is already running
    if (this->port) throw VAError(ERROR_GDB_RUNNING);
        
    this->port = port;

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
            
            // if (cmd == "") break;
        }
        
    } catch (...) {
        
        port = 0;
        connection.close();
        listener.close();
        
        debug(GDB_DEBUG, "Leaving main\n");
    }
}
