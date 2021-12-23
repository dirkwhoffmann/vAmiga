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
        
    if (category & dump::State) {
        
        os << tab("Listening");
        if (listening) {
            os << bol(listening) << std::endl;
        } else {
            os << "at port " << dec(port) << std::endl;
        }
        os << tab("Connected");
        os << bol(connected) << std::endl;
        os << tab("Received");
        os << dec(numReceived) << " packets" << std::endl;
        os << tab("Transmitted");
        os << dec(numSent) << " packets" << std::endl;
    }
}

void
RemoteServer::start(isize port)
{
    if (listening) throw VAError(ERROR_SERVER_RUNNING);
    
    debug(SRV_DEBUG, "Starting remote server at port %ld...\n", port);

    // Make sure that we continue with a terminated server thread
    if (serverThread.joinable()) serverThread.join();
    
    // Spawn a new thread
    this->port = port;
    serverThread = std::thread(&RemoteServer::main, this);
}

void
RemoteServer::stop()
{
    if (!listening) throw VAError(ERROR_SERVER_NOT_RUNNING);

    debug(SRV_DEBUG, "Stopping remote server...\n");
         
    // Interrupt the server thread
    listening = false;
    disconnect();

    // Wait until the server thread has terminated
    serverThread.join();
}

void
RemoteServer::disconnect()
{
    if (connected) {
        
        // Trigger an exception inside the server thread
        connection.close();
        listener.close();    
    }
}

string
RemoteServer::receive()
{
    string packet;
    
    if (connected) {
        
        packet = _receive();
        numReceived++;
        debug(SRV_DEBUG, "R: '%s'\n", util::makePrintable(packet).c_str());
        msgQueue.put(MSG_SRV_RECEIVE);
    }
    
    return packet;
}

void
RemoteServer::send(const string &packet)
{
    if (connected) {
        
        _send(packet);
        numSent++;
        debug(SRV_DEBUG, "T: '%s'\n", util::makePrintable(packet).c_str());
        msgQueue.put(MSG_SRV_SEND);
    }
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
RemoteServer::process(const string &payload)
{
    _process(payload);
}

void
RemoteServer::main()
{
    numReceived = 0;
    numSent = 0;

    try {
        
        mainLoop();
        
    } catch (std::exception &err) {

        handleError(err.what());
    }
}

void
RemoteServer::mainLoop()
{
    listening = true;
    debug(SRV_DEBUG, "Remote server started\n");
    msgQueue.put(MSG_SRV_START);
        
    while (listening) {
        
        try {
            
            // Create a port listener
            listener = PortListener((u16)port);
            
            // Wait for a client to connect
            connection = listener.accept();

            // Handle the session
            sessionLoop();
            
            // Close the port listener
            listener.close();
            
        } catch (std::exception &err) {
            
            handleError(err.what());
        }
    }
    
    listening = false;
    debug(SRV_DEBUG, "Remote server stopped\n");
    msgQueue.put(MSG_SRV_STOP);
}

void
RemoteServer::sessionLoop()
{
    connected = true;
    debug(SRV_DEBUG, "Client connected\n");
    msgQueue.put(MSG_SRV_CONNECT);

    try {
        
        // Print the startup message
        welcome();
        
        // Receive and process packets
        while (1) { process(receive()); }
        
    } catch (std::exception &err) {
                 
        // If listening is still true, the loop has been terminated by an error
        if (listening) handleError(err.what());
    }
    
    connection.close();
    connected = false;
    debug(SRV_DEBUG, "Client disconnected\n");
    msgQueue.put(MSG_SRV_DISCONNECT);
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
