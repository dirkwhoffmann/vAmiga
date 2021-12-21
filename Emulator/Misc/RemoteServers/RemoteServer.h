// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Socket.h"
#include <thread>

class RemoteServer : public SubComponent {
    
protected:
    
    // The server thread
    std::thread serverThread;

    // Port number
    isize port = 0;
    
    // Indicates if the server is running
    bool listening = false;
    
    // Indicates if a client is connected
    bool connected = false;

    // The port listener and it's associated connection
    PortListener listener;
    Socket connection;
                
        
    //
    // Initializing
    //
    
public:
    
    RemoteServer(Amiga& ref);
    ~RemoteServer();
    
    
    //
    // Methods from AmigaObject
    //
    
protected:
    
    const char *getDescription() const override { return "RemoteServer"; }
    void _dump(dump::Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { }
    
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }

    
    //
    // Running the server
    //
    
protected:

    // The main thread function
    void main();

private:
    
    // Called inside main
    void mainLoop();
    
    
    //
    // Turning the server on and off
    //
    
public:
    
    // Starts or stops the remote server
    virtual void start(isize port) throws;
    virtual void stop();
    
    // Indicates if the remote server has been started
    bool isListening() { return listening; }
    
    // Indicates if a client has connected
    bool isConnected() { return connected; }
    
    // Disconnects the remote client
    void disconnect();
            
    
    //
    // Transmitting and processing packets
    //
    
public:
    
    // Receives or transmits a string
    virtual string receive() = 0;
    virtual void send(const string &payload) = 0;
    
    // Convenience wrappers
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
 
private:

    // Prints the welcome message
    virtual void welcome() { }
        
    // Reports an error to the GUI
    void handleError(const char *description);
};
