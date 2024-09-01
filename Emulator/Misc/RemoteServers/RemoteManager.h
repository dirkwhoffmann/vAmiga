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
#include "SerServer.h"
#include "RshServer.h"
#include "GdbServer.h"

namespace vamiga {

class RemoteManager final : public SubComponent, public Inspectable<RemoteManagerInfo> {

    Descriptions descriptions = {{

        .name           = "RemoteManager",
        .description    = "Remote Manager",
        .shell          = "server"
    }};

    ConfigOptions options = {

    };

public:
    
    // The remote servers
    SerServer serServer = SerServer(amiga, SERVER_SER);
    RshServer rshServer = RshServer(amiga, SERVER_RSH);
    GdbServer gdbServer = GdbServer(amiga, SERVER_GDB);
    
    // Convenience wrapper
    std::vector <RemoteServer *> servers = {
        &serServer, &rshServer, &gdbServer
    };

    
    //
    // Initializing
    //
    
public:
    
    RemoteManager(Amiga& ref);
    
    RemoteManager& operator= (const RemoteManager& other) {

        CLONE(serServer)
        CLONE(rshServer)
        CLONE(gdbServer)

        return *this;
    }


    //
    // Methods from CoreObject
    //
    
protected:
    
    void _dump(Category category, std::ostream& os) const override;
    
    
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

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(RemoteManagerInfo &result) const override;


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
    // Servicing events
    //
    
public:
    
    void serviceServerEvent();
};

}
