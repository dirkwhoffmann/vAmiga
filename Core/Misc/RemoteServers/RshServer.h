// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SocketServer.h"
#include "Console.h"

namespace vamiga {

class RshServer final : public SocketServer, public ConsoleDelegate {

public:
    
    using SocketServer::SocketServer;

    RshServer& operator= (const RshServer& other) {

        SocketServer::operator = (other);
        return *this;
    }


    //
    // Methods from CoreObject
    //
    
private:

    void _initialize() override;
    void _dump(Category category, std::ostream &os) const override;

    //
    // Methods from RemoteServer
    //

    virtual bool canRun() override { return true; }


    //
    // Methods from SocketServer
    //

    string doReceive() throws override;
    void doProcess(const string &packet) throws override;
    void doSend(const string &packet)throws  override;
    void didStart() override;
    void didConnect() override;


    //
    // Methods from ConsoleDelegate
    //

    void didActivate() override;
    void didDeactivate() override;
    void willExecute(const InputLine &input) override;
    void didExecute(const InputLine &input, std::stringstream &ss) override;
    void didExecute(const InputLine &input, std::stringstream &ss, std::exception &e) override;
};

}
