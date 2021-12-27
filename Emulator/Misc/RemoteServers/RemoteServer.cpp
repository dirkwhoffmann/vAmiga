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
        
        os << tab("State");
        os << SrvStateEnum::key(state) << std::endl;
        os << tab("Port");
        os << dec(port) << std::endl;
        os << tab("Received packets");
        os << dec(numReceived) << std::endl;
        os << tab("Transmitted packets");
        os << dec(numSent) << std::endl;
    }
}

void
RemoteServer::start()
{
    SUSPENDED
    
    // Only proceed if the server is not running yet
    if (isListening() || isConnected()) throw VAError(ERROR_SERVER_ON);
    
    // Check if the server is ready to launch
    if (canStart()) {
        
        // Make sure we continue with a terminated server thread
        if (serverThread.joinable()) serverThread.join();
        
        // Spawn a new thread
        serverThread = std::thread(&RemoteServer::main, this);
        
    } else {
        
        // Postpone the launch
        switchState(SRV_STATE_STARTING);
    }
}

void
RemoteServer::stop()
{
    SUSPENDED
    
    debug(SRV_DEBUG, "Shutting down server...\n");
    
    // Only proceed if the server is alive
    if (isOff()) throw VAError(ERROR_SERVER_OFF);
    
    switchState(SRV_STATE_STOPPING);
    
    // Interrupt the server thread
    disconnect();
    
    // Wait until the server thread has terminated
    if (serverThread.joinable()) serverThread.join();
    
    switchState(SRV_STATE_OFF);
}

void
RemoteServer::disconnect()
{
    SUSPENDED
    
    debug(SRV_DEBUG, "Disconnecting...\n");
    
    // Trigger an exception inside the server thread
    connection.close();
    listener.close();
}

void
RemoteServer::switchState(SrvState newState)
{
    auto oldState = state;
    
    if (oldState != newState) {
        
        debug(SRV_DEBUG, "Switching state: %s -> %s\n",
              SrvStateEnum::key(state), SrvStateEnum::key(newState));
        
        // Switch state and call the delegation method
        state = newState;
        didSwitch(oldState, newState);
        
        // Inform the GUI
        switch(state) {
                
            case SRV_STATE_OFF:         msgQueue.put(MSG_SRV_OFF); break;
            case SRV_STATE_STARTING:    msgQueue.put(MSG_SRV_STARTING); break;
            case SRV_STATE_LISTENING:   msgQueue.put(MSG_SRV_LISTENING); break;
            case SRV_STATE_CONNECTED:   msgQueue.put(MSG_SRV_CONNECTED); break;
            case SRV_STATE_STOPPING:    msgQueue.put(MSG_SRV_STOPPING); break;
            case SRV_STATE_ERROR:       msgQueue.put(MSG_SRV_ERROR); break;

            default:
                fatalError;
        }
    }
}

string
RemoteServer::receive()
{
    string packet;
    
    if (isConnected()) {
        
        packet = doReceive();
        numReceived++;
        debug(SRV_DEBUG, "R: '%s'\n", util::makePrintable(packet).c_str());
        msgQueue.put(MSG_SRV_RECEIVE);
    }
    
    return packet;
}

void
RemoteServer::send(const string &packet)
{
    if (isConnected()) {
        
        doSend(packet);
        numSent++;
        debug(SRV_DEBUG, "T: '%s'\n", util::makePrintable(packet).c_str());
        msgQueue.put(MSG_SRV_SEND);
    }
}

void
RemoteServer::send(char payload)
{
    send(string(1, payload));
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
    doProcess(payload);
}

void
RemoteServer::main()
{
    try {
        
        mainLoop();
        
    } catch (std::exception &err) {

        debug(SRV_DEBUG, "Server thread interrupted\n");
        handleError(err.what());
    }
}

void
RemoteServer::mainLoop()
{
    switchState(SRV_STATE_LISTENING);
            
    while (isListening()) {
        
        try {
            
            try {
                
                // Try to be a client by connecting to an existing server
                connection.connect((u16)port);
                debug(SRV_DEBUG, "Acting as a client\n");
                
            } catch (...) {
                
                // If there is no existing server, be the server
                debug(SRV_DEBUG, "Acting as a server\n");
                
                // Create a port listener
                listener.bind((u16)port);
                listener.listen();
                
                // Wait for a client to connect
                connection = listener.accept();
            }
            
            // Handle the session
            sessionLoop();
            
            // Close the port listener
            listener.close();
            
        } catch (std::exception &err) {
            
            debug(SRV_DEBUG, "Main loop interrupted\n");

            // Handle error if we haven't been interrupted purposely
            if (!isStopping()) handleError(err.what());
        }
    }
    
    switchState(SRV_STATE_OFF);
}

void
RemoteServer::sessionLoop()
{
    switchState(SRV_STATE_CONNECTED);
    
    numReceived = 0;
    numSent = 0;

    try {
                
        // Receive and process packets
        while (1) { process(receive()); }
        
    } catch (std::exception &err) {
                     
        debug(SRV_DEBUG, "Session loop interrupted\n");

        // Handle error if we haven't been interrupted purposely
        if (!isStopping()) {
            
            handleError(err.what());
            switchState(SRV_STATE_LISTENING);
        }
    }

    numReceived = 0;
    numSent = 0;

    connection.close();
}

void
RemoteServer::handleError(const char *description)
{
    // Switch to error state
    switchState(SRV_STATE_ERROR);

    // Compose the error message
    auto msg = "Server Error: " + string(description);
    debug(SRV_DEBUG, "%s\n", msg.c_str());
    
    // Inform the GUI
    retroShell << msg << '\n';
    msgQueue.put(MSG_SRV_ERROR);
}
