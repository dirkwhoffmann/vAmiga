// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RemoteServer.h"
#include "Amiga.h"
#include "CPU.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MemUtils.h"
#include "MsgQueue.h"
#include "RetroShell.h"

RemoteServer::RemoteServer(Amiga& ref) : SubComponent(ref)
{

}

RemoteServer::~RemoteServer()
{
    if (isListening()) stop();
}

void
RemoteServer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
    }
    
    if (category & dump::State) {
        
        os << tab("Listening") << bol(listening) << std::endl;
        os << tab("Connected") << bol(connected) << std::endl;
        os << tab("Port") << dec(port) << std::endl;
    }
}

void
RemoteServer::start(isize port)
{
    debug(SRV_DEBUG, "Starting remote server at port %ld...\n", port);

    // Only proceed if the server is not running
    if (listening) throw VAError(ERROR_SERVER_RUNNING);

    // Make sure that we continue with a terminated server thread
    if (serverThread.joinable()) serverThread.join();
    
    // Spawn a new thread
    this->port = port;
    serverThread = std::thread(&RemoteServer::main, this);
}

void
RemoteServer::stop()
{
    debug(SRV_DEBUG, "Stopping remote server...\n");
 
    // Only proceed if an open connection exists
    if (!listening) throw VAError(ERROR_SERVER_NOT_RUNNING);
        
    // Interrupt the server thread
    listening = false;
    disconnect();

    // Wait until the server thread has terminated
    serverThread.join();
}

void
RemoteServer::disconnect()
{
    // Trigger an exception inside the server thread
    connection.close();
    listener.close();
    
    // Inform the user about the disconnected client
    retroShell << "Disconnecting client" << '\n';
}

void
RemoteServer::send(char payload)
{
    switch (payload) {
            
        case '\n':
            
            send("\n");
            break;
            
        case '\r':
            
            send("\33[2K\r");
            break;
            
        default:
            
            if (isprint(payload)) send(string(1, payload));
            break;
    }
}

void
RemoteServer::send(int payload)
{
    send(std::to_string(payload));
}

void
RemoteServer::send(long payload)
{
    send(std::to_string(payload));
}


void
RemoteServer::send(std::stringstream &payload)
{
    string line;
    while(std::getline(payload, line)) {
        send(line + "\n");
    }
}

void
RemoteServer::main()
{
    listening = true;
    debug(SRV_DEBUG, "Remote server started\n");
    msgQueue.put(MSG_SRV_START);

    try {
        
        mainLoop();
        
    } catch (std::exception &err) {

        handleError(err.what());
    }

    listening = false;
    debug(SRV_DEBUG, "Remote server stopped\n");
    msgQueue.put(MSG_SRV_STOP);
}

void
RemoteServer::mainLoop()
{
    while (listening) {
        
        // Create a port listener
        listener = PortListener((u16)port);
        
        try {
            
            // Wait for a client
            connection = listener.accept();
            
            connected = true;
            debug(SRV_DEBUG, "Connection established\n");
            msgQueue.put(MSG_SRV_CONNECT);
            
            // Print the startup message and the input prompt
            welcome();
            
            // Receive and process packages
            while (1) { receive(); }
            
        } catch (std::exception &err) {
            
            if (listening) handleError(err.what());
            
            connection.close();
            listener.close();

            connected = false;
            debug(SRV_DEBUG, "Client disconnected\n");
            msgQueue.put(MSG_SRV_DISCONNECT);
        }
    }
}

void
RemoteServer::handleError(const char *description)
{
    auto msg = "Server Error: " + string(description);
                     
    debug(SRV_DEBUG, "%s\n", msg.c_str());
    
    // Inform the GUI
    retroShell << msg << '\n';
    msgQueue.put(MSG_SRV_ERROR);
}
