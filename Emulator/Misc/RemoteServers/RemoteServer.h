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
#include "SuspendableThread.h"
#include <thread>

class RemoteServer : public SubComponent {
        
    friend class RemoteManager;
    
    // Current configuration
    ServerConfig config = {};
    
protected:
    
    // Sockets
    Socket listener;
    Socket connection;

    // The server thread
    std::thread serverThread;
        
    // The current server state
    SrvState state = SRV_STATE_OFF;
    
    // The number of sent and received packets
    isize numSent = 0;
    isize numReceived = 0;
        
    
    //
    // Initializing
    //
    
public:
    
    RemoteServer(Amiga& ref);
    ~RemoteServer() { shutDownServer(); }
    void shutDownServer();
    
    
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
    void _powerOff() override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.port
        << config.protocol
        << config.verbose;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    void _didLoad() override;
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring
    //
    
public:
        
    virtual ServerConfig getDefaultConfig() = 0;
    const ServerConfig &getConfig() const { return config; }
    void resetConfig() override;
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
        

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
    // Starting and stopping the server
    //
    
public:
                     
    // Launch the remote server
    void start() throws { SUSPENDED _start(); }
    
    // Shuts down the remote server
    void stop() throws { SUSPENDED _stop(); }

    // Disconnects the client
    void disconnect() throws { SUSPENDED _disconnect(); }

protected:

    // Called from disconnect(), start() and stop()
    void _start() throws;
    void _stop() throws;
    void _disconnect() throws;
    
    // Switches the internal state
    void switchState(SrvState newState);
    
private:
    
    // Used by the launch manager to determine if actions should be taken
    virtual bool shouldRun() { return true; }
    
    // Indicates if the server is able to run
    // virtual bool canRun() { return true; }
    
    
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

    virtual void didSwitch(SrvState from, SrvState to) { };
};
