// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(SRV_STATE)
{
    SRV_STATE_OFF,        // The server is inactive
    SRV_STATE_STARTING,  // The server is starting up
    SRV_STATE_LISTENING,  // The server is waiting for a client to connect
    SRV_STATE_CONNECTED,  // The server is connected to a client
    SRV_STATE_STOPPING,   // The server is shutting down
    SRV_STATE_ERROR       // The server is in an error state
};
typedef SRV_STATE SrvState;

#ifdef __cplusplus
struct SrvStateEnum : util::Reflection<SrvStateEnum, SrvState>
{
    static long minVal() { return 0; }
    static long maxVal() { return SRV_STATE_ERROR; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "SRV"; }
    static const char *key(SrvState value)
    {
        switch (value) {
                
            case SRV_STATE_OFF:         return "OFF";
            case SRV_STATE_STARTING:    return "STARTING";
            case SRV_STATE_LISTENING:   return "LISTENING";
            case SRV_STATE_CONNECTED:   return "CONNECTED";
            case SRV_STATE_STOPPING:    return "STOPPING";
            case SRV_STATE_ERROR:       return "ERROR";
        }
        return "???";
    }
};
#endif

enum_long(SRVPROT)
{
    SRVPROT_DEFAULT
};
typedef SRVPROT ServerProtocol;

#ifdef __cplusplus
struct ServerProtocolEnum : util::Reflection<ServerProtocolEnum, ServerProtocol>
{
    static long minVal() { return 0; }
    static long maxVal() { return SRVPROT_DEFAULT; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "SRVPROT"; }
    static const char *key(SrvState value)
    {
        switch (value) {
                
            case SRVPROT_DEFAULT:   return "DEFAULT";
        }
        return "???";
    }
};
#endif

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
