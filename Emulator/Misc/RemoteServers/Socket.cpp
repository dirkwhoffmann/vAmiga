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
#include "MemUtils.h"

Socket::Socket() : socket(INVALID_SOCKET)
{
    debug(SCK_DEBUG, "Socket constructor\n");
}

Socket::Socket(SOCKET id) : socket(id)
{
    debug(SCK_DEBUG, "Wrapping socket %d\n", id);
}

Socket::Socket(Socket&& other)
{
    socket = other.socket;
    other.socket = INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other)
{
    assert(socket != other.socket);
    
    if (socket != other.socket) {

        ::close(socket);
        socket = other.socket;
        other.socket = INVALID_SOCKET;        
    }
    return *this;
}

Socket::~Socket()
{
    debug(SCK_DEBUG, "Socket destructor\n");
    
    if (socket != INVALID_SOCKET) {

        debug(SCK_DEBUG, "Closing socket %d\n", socket);
        ::close(socket);
    }
}

void Socket::create()
{
    assert(socket == INVALID_SOCKET);
    
#ifdef WIN32
    static struct WSAInit {
        
        WSAInit() {
            
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData))
                throw VAError(ERROR_SOCK_CANT_CREATE);
        }
        ~WSAInit() {
            
            WSACleanup();
        }
    } wsaInit;
#endif
    
    socket = ::socket(AF_INET, SOCK_STREAM, 0);
    
    if (socket == INVALID_SOCKET) {
        throw VAError(ERROR_SOCK_CANT_CREATE);
    }
    
    debug(SCK_DEBUG, "Created new socket %d\n", socket);
}

int
Socket::connect(u16 port)
{
    printf("Socket::connect()\n");
    struct sockaddr_in address;
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = util::bigEndian(port);
    
    if (::connect(socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        // throw VAError(ERROR_SOCK_CANT_CONNECT);
        printf("Socket::connect() failed\n");
        return -1;
    }

    printf("Socket::connect() successful\n");
    return 0;
}

void
Socket::bind(u16 port)
{
    struct sockaddr_in address;
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = util::bigEndian(port);
    
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
    
    if (s == INVALID_SOCKET) {
        throw VAError(ERROR_SOCK_CANT_CONNECT);
    }
    
    return Socket(s);
}

std::string
Socket::recv()
{    
    char buffer[BUFFER_SIZE + 1] = {};
    auto n = ::recv(socket, buffer, BUFFER_SIZE, 0);
    
    if (n > 0) {
        
        string result = string(buffer);
        
        // Remove LF and CR
        while (!result.empty() && (result.back() == 10 || result.back() == 13)) {
            result.pop_back();
        }
        
        return result;
    }

    throw VAError(ERROR_SOCK_DISCONNECTED);
}

void
Socket::send(const string &s)
{
    ::send(socket, s.c_str(), (int)s.length(), 0);
}

void
Socket::close()
{    
    if (socket != INVALID_SOCKET) {

        debug(SCK_DEBUG, "Closing socket %d\n", socket);
#ifdef _WIN32
        closesocket(socket);
#else
        ::close(socket);
#endif
        socket = INVALID_SOCKET;
    }
}

PortListener::PortListener()
{
}

PortListener::PortListener(u16 port) {

    server.create();
    
    int opt = 1;
    auto success = setsockopt(server.getSocket(),
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              (const char *)&opt,
                              sizeof(opt));
    
    if (success < 0) throw VAError(ERROR_SOCK_CANT_CONNECT);
    
    server.bind(port);
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

