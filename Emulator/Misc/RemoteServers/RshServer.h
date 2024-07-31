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

class RshServer : public RemoteServer {

    Descriptions descriptions = {{

        .name           = "RshServer",
        .description    = "Serial Port Server",
        .shell          = "server rshell"
    }};

public:
    
    using RemoteServer::RemoteServer;

    RshServer& operator= (const RshServer& other) {

        RemoteServer::operator = (other);
        return *this;
    }


    //
    // Methods from CoreObject
    //
    
private:

    void _dump(Category category, std::ostream& os) const override;

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from RemoteServer
    //
    
    string doReceive() throws override;
    void doProcess(const string &packet) throws override;
    void doSend(const string &packet)throws  override;
    void didStart() override;
    void didConnect() override;
};

}
