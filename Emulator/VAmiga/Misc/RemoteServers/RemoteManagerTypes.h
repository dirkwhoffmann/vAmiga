// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga/Foundation/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class ServerType : long
{
    SER,
    RSH,
    PROM,
    GDB
};

struct ServerTypeEnum : Reflection<ServerTypeEnum, ServerType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(ServerType::GDB);
    
    static const char *_key(ServerType value)
    {
        switch (value) {
                
            case ServerType::SER:    return "SER";
            case ServerType::RSH:    return "RSH";
            case ServerType::PROM:   return "PROM";
            case ServerType::GDB:    return "GDB";
        }
        return "???";
    }
    static const char *help(ServerType value)
    {
        switch (value) {
                
            case ServerType::SER:    return "Serial port server";
            case ServerType::RSH:    return "Remote shell server";
            case ServerType::PROM:   return "Prometheus server";
            case ServerType::GDB:    return "Debug server";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    isize numLaunching;
    isize numListening;
    isize numConnected;
    isize numErroneous;
}
RemoteManagerInfo;

}
