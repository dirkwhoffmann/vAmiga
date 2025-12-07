// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "RemoteManagerTypes.h"
#include "RshServer.h"
#include "RpcServer.h"
#include "GdbServer.h"
#include "PromServer.h"
#include "SerServer.h"
#include "utl/wrappers.h"

namespace vamiga {

class RemoteManager final : public SubComponent {

    Descriptions descriptions = {{

        .name           = "RemoteManager",
        .description    = "Remote Manager",
        .shell          = "server"
    }};

    Options options = {

    };

public:

    // Result of the latest inspection
    utl::Memorized<RemoteManagerInfo> info;

private:

    // Frame counter
    usize frame = 0;

public:
    
    // The remote servers
    RshServer rshServer = RshServer(amiga, isize(ServerType::RSH));
    RpcServer rpcServer = RpcServer(amiga, isize(ServerType::RPC));
    GdbServer gdbServer = GdbServer(amiga, isize(ServerType::GDB));
    PromServer promServer = PromServer(amiga, isize(ServerType::PROM));
    SerServer serServer = SerServer(amiga, isize(ServerType::SER));

    // Convenience access
    std::vector <RemoteServer *> servers = {
        &rshServer, &rpcServer, &gdbServer, &promServer, &serServer
    };

    
    //
    // Initializing
    //
    
public:
    
    RemoteManager(Amiga& ref);
    
    RemoteManager& operator= (const RemoteManager& other) {

        CLONE(rshServer)
        CLONE(rpcServer)
        CLONE(gdbServer)
        CLONE(promServer)
        CLONE(serServer)

        return *this;
    }


    //
    // Methods from CoreObject
    //
    
protected:
    
    void _dump(Category category, std::ostream &os) const override;
    
    
    //
    // Methods from CoreComponent
    //
    
private:
    
    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);
        
public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:

    const Options &getOptions() const override { return options; }


    //
    // Analyzing
    //

public:

    RemoteManagerInfo cacheInfo() const;


    //
    // Managing connections
    //
    
public:
    
    // Returns the number of servers being in a certain state
    isize numLaunching() const;
    isize numListening() const;
    isize numConnected() const;
    isize numErroneous() const;


    //
    // Running the launch daemon
    //
    
public:

    void update();

    void serviceServerEvent();
};

}
