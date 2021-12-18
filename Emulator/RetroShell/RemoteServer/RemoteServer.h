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
    
    // The current configuration
    RemoteServerConfig config = {};

    // The server thread
    std::thread serverThread;

    // Indicates if the server is running
    bool listening = false;
    
    // Indicates if a client is connected
    bool connected = false;

    // The port listener and it's associated connection
    PortListener listener;
    Socket connection;
                
    // The most recently processed command string
    string latestCmd;
    
    
    //
    // Initializing
    //
    
public:
    
    RemoteServer(Amiga& ref);
    ~RemoteServer();
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
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
    // Configuring
    //

public:
    
    static RemoteServerConfig getDefaultConfig();
    const RemoteServerConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    

    //
    // Running the server
    //
    
private:

    // The main thread function
    void main();

    
    //
    // Turning the server on and off
    //
    
public:
    
    void start() throws;
    void stop();
    
    bool isListening() { return listening; }
    bool isConnected() { return connected; }
    
    
    //
    // Transmitting packets
    //
    
    string receive() throws;
    void send(const string &packet) throws;
    
    RemoteServer &operator<<(char value);
    RemoteServer &operator<<(const string &value);
    RemoteServer &operator<<(int value);
    RemoteServer &operator<<(long value);
    RemoteServer &operator<<(std::stringstream &stream);
};
