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
#include "RemoteServer.h"

class GdbServer : public RemoteServer {

    // The most recently processed command string
    string latestCmd;
    
    // Indicates whether received packets should be acknowledged
    bool ackMode = true;

    
    //
    // Initializing
    //
    
public:

    using RemoteServer::RemoteServer;

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "GdbServer"; }
    
    
    //
    // Methods from RemoteServer
    //
    
public:
    
    void start(isize port) override throws;
    string _receive() override throws;
    void _send(const string &payload) override throws;
    void _process(const string &payload) override throws;

    // Sends a packet with control characters and a checksum attached
    void reply(const string &payload);
 
    
    //
    // Managing checksums
    //

    // Computes a checksum for a given string
    string computeChecksum(const string &s);

    // Verifies the checksum for a given string
    bool verifyChecksum(const string &s, const string &chk);

        
    //
    // Processing packets
    //

public:
        
    // Processes a packet in the format used by GDB
    void process(string packet) throws;

    // Processes a checksum-free packet with the first letter stripped off
    void process(char letter, string packet) throws;

private:
        
    // Processes a single command (GdbServerCmds.cpp)
    template <char letter> void process(string arg) throws;
    template <char letter, GdbCmd cmd> void process(string arg) throws;
    
    
    //
    // Reading the emulator state
    //
    
    // Reads a register value
    string readRegister(isize nr);

    // Reads a byte from memory
    string readMemory(isize addr);
};
