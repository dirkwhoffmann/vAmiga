// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SocketServer.h"
#include "OSDebugger.h"

namespace vamiga {

enum class GdbCmd : long
{
    Attached,
    C,
    ContQ,
    Cont,
    MustReplyEmpty,
    CtrlC,
    Offset,
    StartNoAckMode,
    sThreadInfo,
    Supported,
    Symbol,
    TfV,
    TfP,
    TStatus,
    fThreadInfo,
};

class GdbServer final : public SocketServer {

    // The name of the process to be debugged
    string processName;
    
    // The segment list of the debug process
    os::SegList segList;
    
    // The most recently processed command string
    string latestCmd;
    
    // Indicates whether received packets should be acknowledged
    bool ackMode = true;
    
    
    //
    // Initializing
    //
    
public:
    
    using SocketServer::SocketServer;

    GdbServer& operator= (const GdbServer& other) {

        SocketServer::operator = (other);
        return *this;
    }


    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream &os) const override;
    
    
    //
    // Methods from RemoteServer
    //
    
public:
    
    bool shouldRun() override;
    string doReceive() throws override;
    void doSend(const string &payload) throws override;
    void doProcess(const string &payload) throws override;
    void didStart() override;
    void didStop() override;
    void didConnect() override;
    
    
    //
    // Attaching and detaching processes
    //
    
    // Attach a process to the GDB server
    bool attach(const string &name);
    
    // Detaches the attached process
    void detach();
    
    // Tries to reads the segList for the attached process
    bool readSegList();
    
    // Queries segment information about the attached process
    u32 codeSeg() const;
    u32 dataSeg() const;
    u32 bssSeg() const;
    
    
    //
    // Managing checksums
    //
    
    // Computes a checksum for a given string
    string computeChecksum(const string &s);
    
    // Verifies the checksum for a given string
    bool verifyChecksum(const string &s, const string &chk);
    
    
    //
    // Handling packets
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
    
    // Sends a packet with control characters and a checksum attached
    void reply(const string &payload);
    
    
    //
    // Reading the emulator state
    //
    
    // Reads a register value
    string readRegister(isize nr);
    
    // Reads a byte from memory
    string readMemory(isize addr);
    
    
    //
    // Delegation methods
    //
    
public:
    
    void breakpointReached();
};

}
