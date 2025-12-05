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
#include "RetroShellTypes.h"
#include "Console.h"

namespace vamiga {

namespace RPC {

const long PARSE_ERROR      = -32700; // Invalid JSON was received by the server
const long INVALID_REQUEST  = -32600; // The JSON sent is not a valid Request object
const long METHOD_NOT_FOUND = -32601; // The method does not exist / is not available
const long INVALID_PARAMS   = -32602; // Invalid method parameter(s)
const long INTERNAL_ERROR   = -32603; // Internal JSON-RPC error
const long SERVER_ERROR     = -32000; // Reserved for implementation-defined server-errors

}

class RpcServer final : public SocketServer, public ConsoleDelegate {

public:

    using SocketServer::SocketServer;

    RpcServer& operator= (const RpcServer& other) {

        SocketServer::operator = (other);
        return *this;
    }


    //
    // Methods from CoreObject
    //

protected:

    void _initialize() override;
    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from RemoteServer
    //

    virtual bool canRun() override { return true; }

    
    //
    // Methods from SocketServer
    //

    string doReceive() override;
    void doProcess(const string &packet) override;
    void doSend(const string &packet)  override;
    void didStart() override;


    //
    // Methods from ConsoleDelegate
    //

    void willExecute(const InputLine &input) override;
    void didExecute(const InputLine &input, std::stringstream &ss) override;
    void didExecute(const InputLine &input, std::stringstream &ss, std::exception &e) override;
};

}
