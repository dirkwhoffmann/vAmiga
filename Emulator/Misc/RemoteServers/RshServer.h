// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RemoteServer.h"

namespace vamiga {

class RshServer : public RemoteServer {

public:
    
    RshServer(Amiga& ref);

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
