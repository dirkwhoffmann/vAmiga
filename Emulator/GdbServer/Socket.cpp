// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Socket.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

/*
Socket::Socket(int socket)
{
    this->socket = socket;
    
    if (socket < 0) {
        throw VAError(ERROR_SOCK_CANT_CREATE);
    }
}

Socket::~Socket()
{
    printf("~Socket(%d)\n", socket);
    // if (socket != -1) close();
}

Socket::Socket() : Socket(::socket(AF_INET, SOCK_STREAM, 0))
{
    
}
*/

int Socket::init()
{
    if (auto id = ::socket(AF_INET, SOCK_STREAM, 0)) {
        
        socket = id;
        return id;
    }

    throw VAError(ERROR_SOCK_CANT_CREATE);
}

void
Socket::bind(isize port)
{
    struct sockaddr_in address;
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (::bind(socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        throw VAError(ERROR_SOCK_CANT_CONNECT);
    }
}

void
Socket::listen()
{
    if (::listen(socket, 3) < 0) {
        throw VAError(ERROR_SOCK_CANT_CONNECT);
    }
}

Socket
Socket::accept()
{
    struct sockaddr_in address;
    auto addrlen = (socklen_t)sizeof(struct sockaddr_in);

    auto s = ::accept(socket, (struct sockaddr *)&address, &addrlen);
    
    if (s < 0) {
        throw VAError(ERROR_SOCK_CANT_CONNECT);
    }
    
    return Socket(s);
}

std::string
Socket::recv()
{    
    char buffer[8096];
    auto n = ::recv(socket, buffer, sizeof(buffer) - 1, 0);
    
    if (n > 0) {
        
        string result = string(buffer, n);
        return result;
    }

    throw VAError(ERROR_SOCK_DISCONNECTED);
}

void
Socket::send(std::string s)
{
    ::send(socket, s.c_str(), s.length(), 0);
}

void
Socket::close()
{
    if (socket != -1) {
        
        ::close(socket);
        socket = -1;
    }
}

PortListener::PortListener()
{
}

PortListener::PortListener(isize port) {

    auto socket = server.init();
    
    int opt = 1;
    auto success = setsockopt(socket,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              &opt,
                              sizeof(opt));
    
    if (success < 0) throw VAError(ERROR_SOCK_CANT_CONNECT);
    
    printf("Calling bind\n");
    server.bind(port);
    printf("Calling listen\n");
    server.listen();
}

Socket
PortListener::accept()
{
    return server.accept();
}

void
PortListener::close()
{
    server.close();
}

