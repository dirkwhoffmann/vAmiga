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

public:
    
    using RemoteServer::RemoteServer;

    //
    // Methods from CoreObject
    //
    
protected:
    
    const char *getDescription() const override { return "RshServer"; }
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from CoreComponent
    //

    void resetConfig() override;
    
    
    //
    // Methods from RemoteServer
    //
    
    string doReceive() override throws;
    void doProcess(const string &packet) override throws;
    void doSend(const string &packet) override throws;
    void didStart() override;
    void didConnect() override;
};

}
