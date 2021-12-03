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

class Socket : public AmigaObject {

    int socket;
    
    
    //
    // Initializing
    //
    
public:

    Socket() : socket(-1) { }
    Socket(int id) : socket(id) { }
    /*
    Socket();
    ~Socket();
    */
    
    int init();
    
    int getSocket() { return socket; }
    
    
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
    
    void bind(isize port);
    void listen();
    Socket accept();
    void close();

    
    //
    // Transfering data
    //
    
public:
    
    string recv();
    void send(string s);
};

class PortListener {
    
    Socket server;
    
public:
    
    PortListener();
    PortListener(isize port);
    Socket accept();
    void close();
};
