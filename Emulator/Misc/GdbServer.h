// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "GdbServerTypes.h"
#include "SubComponent.h"
#include "Socket.h"
#include <thread>

class GdbServer : public SubComponent {
    
    // Current configuration
    GdbServerConfig config = {};

    // The server thread
    std::thread serverThread;

    // Indicates if the server is currently running
    bool listening = false;

    // Indicates whether received packets should be acknowledged
    bool ackMode = false;
    
    // The port listener and it's associated connection
    PortListener listener;
    Socket connection;
            
    
    //
    // Initializing
    //
    
public:
    
    GdbServer(Amiga& ref);
    ~GdbServer();

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "GdbServer"; }
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
    
    static GdbServerConfig getDefaultConfig();
    const GdbServerConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    

    //
    // Running the server
    //
    
private:
    
    void main();

    
    //
    // Turning the server on and off
    //
    
public:
    
    void start() throws;
    void stop();
    
    bool isListening() { return listening; }
    
    
    //
    // Transmitting packets
    //
    
    string receive() throws;
    void send(const string &packet) throws;
    
    
    //
    // Processing packets
    //

public:
    
    void process(string cmd) throws;
    
private:
    
    void process(char cmd, string arg) throws;
    template <char cmd> void process(string arg) throws;
    
    string checksum(const string &s);
    std::vector<string> split(const string &s, char delimiter);
    
    
    //
    // Reading the emulator state
    //
    
    // Reads a register value
    string readRegister(isize nr);

    // Reads a byte from memory
    string readMemory(isize addr);

    
    //
    // Handling emulator events
    //

public:
    
    void breakpointReached();
    
};
