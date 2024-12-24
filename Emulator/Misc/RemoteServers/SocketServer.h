// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RemoteServer.h"

namespace vamiga {

class SocketServer : public RemoteServer {

    using RemoteServer::RemoteServer;

protected:

    SocketServer& operator= (const SocketServer& other) {

        RemoteServer::operator = (other);
        return *this;
    }
    
    // Sockets
    Socket listener;
    Socket connection;

    // Number of transmitted packages
    isize numSent = 0;
    isize numReceived = 0;


    //
    // Methods from CoreObject
    //

protected:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from RemoteServer
    //

public:

    virtual void disconnect() override;


    //
    // Running the server
    //

private:

    // The main thread function
    void main() override;

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
    SocketServer &operator<<(char payload) { send(payload); return *this; }
    SocketServer &operator<<(const string &payload) { send(payload); return *this; }
    SocketServer &operator<<(int payload) { send(payload); return *this; }
    SocketServer &operator<<(long payload) { send(payload); return *this; }
    SocketServer &operator<<(std::stringstream &payload) { send(payload); return *this; }

    // Processes a package
    void process(const string &payload) throws;


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


};

}
