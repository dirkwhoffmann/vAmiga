// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <ws2tcpip.h>

#else

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET  -1

#endif

class Socket : public AmigaObject {

    SOCKET socket;

public:
    
    // Size of the communication buffer
    static constexpr isize BUFFER_SIZE = 512;
    
    
    //
    // Initializing
    //
    
public:

    Socket() : socket(INVALID_SOCKET) { }
    Socket(SOCKET id) : socket(id) { }
    
    SOCKET init();
    SOCKET getSocket() { return socket; }
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "Socket"; }
    void _dump(dump::Category category, std::ostream& os) const override { };

    
    //
    // Establishing and terminating a connection
    //
    
public:
    
    void bind(u16 port);
    void listen();
    Socket accept();
    void close();

    
    //
    // Transfering data
    //
    
public:
    
    string recv();
    void send(const string &s);
};

class PortListener {
    
    Socket server;
    
public:
    
    PortListener();
    PortListener(u16 port);
    Socket accept();
    void close();
};
