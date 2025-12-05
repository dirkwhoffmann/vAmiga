// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"
#include "RemoteServerTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class ServerType : long
{
    RSH,
    RPC,
    GDB,
    PROM,
    SER
};

struct ServerTypeEnum : Reflectable<ServerTypeEnum, ServerType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(ServerType::SER);

    static const char *_key(ServerType value)
    {
        switch (value) {
                
            case ServerType::RSH:    return "RSH";
            case ServerType::RPC:    return "RPC";
            case ServerType::GDB:    return "GDB";
            case ServerType::PROM:   return "PROM";
            case ServerType::SER:    return "SER";
        }
        return "???";
    }
    static const char *help(ServerType value)
    {
        switch (value) {
                
            case ServerType::RSH:    return "Remote shell server";
            case ServerType::RPC:    return "JSON RPC server";
            case ServerType::GDB:    return "Debug server";
            case ServerType::PROM:   return "Prometheus server";
            case ServerType::SER:    return "Serial port server";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    RemoteServerInfo rshInfo;
    RemoteServerInfo rpcInfo;
    RemoteServerInfo gdbInfo;
    RemoteServerInfo promInfo;
    RemoteServerInfo serInfo;
}
RemoteManagerInfo;

}
