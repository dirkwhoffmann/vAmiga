// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RemoteServerTypes.h"
#include "SubComponent.h"
#include "Socket.h"
#include <thread>

class RemoteServer : public SubComponent {
        
    friend class RemoteManager;
    
protected:
    
    // The server thread
    std::thread serverThread;

    // The port number
    isize port = 0;
    
    // The launch arguments (if any)
    std::vector <string> args;
    
    // The current server state
    SrvState state = SRV_STATE_OFF;
    
    // The port listener and it's associated connection
    PortListener listener;
    Socket connection;

    // The number of sent and received packets
    isize numSent = 0;
    isize numReceived = 0;
        
    
    //
    // Initializing
    //
    
public:
    
    RemoteServer(Amiga& ref);
    ~RemoteServer();
    
    
    //
    // Methods from AmigaObject
    //
    
public:
    
    const char *getDescription() const override { return "RemoteServer"; }
    void _dump(dump::Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { }
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override { return 0; }
    isize _save(u8 *buffer) override { return 0; }

    
    //
    // Configuring
    //
    
public:
    
    isize getPort() const { return port; }
    void setPort(isize value) { port = value; }

    const std::vector <string> &getArgs() const { return args; }
    void setArgs(const std::vector <string> &newArgs) { args = newArgs; }
    

    //
    // Examining state
    //
    
public:
        
    bool isOff() const { return state == SRV_STATE_OFF; }
    bool isStarting() const { return state == SRV_STATE_STARTING; }
    bool isListening() const { return state == SRV_STATE_LISTENING; }
    bool isConnected() const { return state == SRV_STATE_CONNECTED; }
    bool isStopping() const { return state == SRV_STATE_STOPPING; }
    bool isErroneous() const { return state == SRV_STATE_ERROR; }

    
    //
    // Changing state
    //
    
public:
        
    // Returns whether the server is ready to start
    virtual bool canStart() { return true; }
    
    // Launch the remote server
    void start() throws;
    
    // Shuts down the remote server
    void stop() throws;
        
    // Disconnects the client
    void disconnect() throws;
         
protected:
        
    // Switches the internal state
    void switchState(SrvState newState);
    
    
    //
    // Running the server
    //
    
private:
    
    // The main thread function
    void main();

    // Inner loops (called from main)
    void mainLoop() throws;
    void sessionLoop();
    
    
    //
    // Transmitting and processing packets
    //
    
public:
    
    // Receives or packet
    string receive() throws;
    
    // Sends a packet
    void send(const string &payload) throws;
    void send(char payload) throws;
    void send(int payload) throws;
    void send(long payload) throws;
    void send(std::stringstream &payload) throws;
    
    // Operator overloads
    RemoteServer &operator<<(char payload) { send(payload); return *this; }
    RemoteServer &operator<<(const string &payload) { send(payload); return *this; }
    RemoteServer &operator<<(int payload) { send(payload); return *this; }
    RemoteServer &operator<<(long payload) { send(payload); return *this; }
    RemoteServer &operator<<(std::stringstream &payload) { send(payload); return *this; }
 
    // Processes a package
    void process(const string &payload) throws;
    
private:
        
    // Reports an error to the GUI
    void handleError(const char *description);
    
    
    //
    // Subclass specific implementations
    //

private:
    
    virtual string doReceive() throws = 0;
    virtual void doSend(const string &payload) throws = 0;
    virtual void doProcess(const string &payload) throws = 0;
    
    
    //
    // Delegation methods
    //

    virtual void didConnect() throws { };
    virtual void didSwitch(SrvState from, SrvState to) { };
};