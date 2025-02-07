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

enum class SrvState : long
{
    OFF,        // The server is inactive
    STARTING,   // The server is starting up
    LISTENING,  // The server is waiting for a client to connect
    CONNECTED,  // The server is connected to a client
    STOPPING,   // The server is shutting down
    INVALID     // The server is in an error state
};

struct SrvStateEnum : Reflection<SrvStateEnum, SrvState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(SrvState::INVALID);
    
    static const char *_key(SrvState value)
    {
        switch (value) {
                
            case SrvState::OFF:         return "OFF";
            case SrvState::STARTING:    return "STARTING";
            case SrvState::LISTENING:   return "LISTENING";
            case SrvState::CONNECTED:   return "CONNECTED";
            case SrvState::STOPPING:    return "STOPPING";
            case SrvState::INVALID:     return "INVALID";
        }
        return "???";
    }
    static const char *help(SrvState value)
    {
        return "";
    }
};

enum class ServerProtocol
{
    DEFAULT
};

struct ServerProtocolEnum : Reflection<ServerProtocolEnum, ServerProtocol>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(ServerProtocol::DEFAULT);
    
    static const char *_key(ServerProtocol value)
    {
        switch (value) {
                
            case ServerProtocol::DEFAULT:   return "DEFAULT";
        }
        return "???";
    }
    static const char *help(ServerProtocol value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct
{
    // The socket port number of this server
    u16 port;
    
    // Indicates special operation modes (if not DEFAULT)
    ServerProtocol protocol;
    
    // If true, the lauch manager starts and stops the server automatically
    bool autoRun;
    
    // If true, transmitted packets are shown in RetroShell
    bool verbose;
}
ServerConfig;

}
