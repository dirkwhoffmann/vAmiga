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
#include "RemoteManagerTypes.h"
#include "SerServer.h"
#include "RshServer.h"
#include "GdbServer.h"

class RemoteManager : public SubComponent {

public:
    
    // The remote servers
    SerServer serServer = SerServer(amiga);
    RshServer rshServer = RshServer(amiga);
    GdbServer gdbServer = GdbServer(amiga);
    
    // Convenience wrapper
    std::vector <const RemoteServer *> servers = {
        &serServer, &rshServer, &gdbServer
    };
     
    
    //
    // Initializing
    //
    
public:
    
    RemoteManager(Amiga& ref);
    ~RemoteManager();
    
    
    //
    // Methods from AmigaObject
    //
    
protected:
    
    const char *getDescription() const override { return "RemoteManager"; }
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
    // Managing remote servers
    //
    
public:
    
    // Returns a remote server specified by type
    RemoteServer &getServer(ServerType type);
    
    // Returns a default port for server of a certain type
    isize defaultPort(ServerType type) const;
    
    
    //
    // Managing connections
    //
    
    // Returns the number of started or connected servers
    isize numListening() const;
    isize numConnected() const;

    // Starts up a remote server
    void start(ServerType type, isize port) throws;
    void start(ServerType type) throws { start(type, defaultPort(type)); }

    // Shuts down a remote server
    void stop(ServerType type) throws;

    // Disconnects a client (if any)
    void disconnect(ServerType type) throws;
};
