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
        os << tab("Received");
        os << dec(numReceived) << " packets" << std::endl;
        os << tab("Transmitted");
        os << dec(numSent) << " packets" << std::endl;
    }
}

void
RemoteServer::start(isize port, const std::vector <string> &args)
{
    // Only proceed if the server is not running yet
    if (isListening() || isConnected()) throw VAError(ERROR_SERVER_ON);

    this->port = port;
    this->args = args;

    // Check if the server is ready to launch
    if (_launchable()) {
        startThread();
    } else {
        switchState(SRV_STATE_LAUNCHING);
    }
}

void
RemoteServer::startThread()
{
    // Make sure we continue with a terminated server thread
    if (serverThread.joinable()) serverThread.join();

    // Spawn a new thread
    serverThread = std::thread(&RemoteServer::main, this);    
}

void
RemoteServer::stop()
{
    // Only proceed if the server is alive
    if (isOff()) throw VAError(ERROR_SERVER_OFF);
             
    // Interrupt the server thread
    disconnect();

    // Wait until the server thread has terminated
    if (serverThread.joinable()) serverThread.join();

    // Switch state and inform the GUI
    switchState(SRV_STATE_OFF);
}

void
RemoteServer::disconnect()
{
    if (isConnected()) {
        
        // Trigger an exception inside the server thread
        connection.close();
        listener.close();
    }
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
            case SRV_STATE_LAUNCHING:   msgQueue.put(MSG_SRV_LAUNCHING); break;
            case SRV_STATE_LISTENING:   msgQueue.put(MSG_SRV_LISTENING); break;
            case SRV_STATE_CONNECTED:   msgQueue.put(MSG_SRV_CONNECTED); break;
                
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
    if (isConnected()) {
        
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
    try {
        
        mainLoop();
        
    } catch (std::exception &err) {

        handleError(err.what());
    }
}

void
RemoteServer::mainLoop()
{
    switchState(SRV_STATE_LISTENING);
            
    while (isListening()) {
        
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
    
    switchState(SRV_STATE_OFF);
}

void
RemoteServer::sessionLoop()
{
    switchState(SRV_STATE_CONNECTED);
    
    numReceived = 0;
    numSent = 0;

    try {
        
        // Welcome the client
        didConnect();
        
        // Receive and process packets
        while (1) { process(receive()); }
        
    } catch (std::exception &err) {
                 
        // If the server is alive, the loop has been terminated due to an error
        if (!isOff()) handleError(err.what());
    }

    numReceived = 0;
    numSent = 0;

    connection.close();
    switchState(SRV_STATE_LISTENING);
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
