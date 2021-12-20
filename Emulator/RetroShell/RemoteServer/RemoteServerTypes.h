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

enum_long(SRVMODE)
{
    SRVMODE_TERMINAL,
    SRVMODE_GDB
};
typedef SRVMODE ServerMode;

#ifdef __cplusplus
struct ServerModeEnum : util::Reflection<ServerModeEnum, ServerMode>
{
    static long minVal() { return 0; }
    static long maxVal() { return SRVMODE_GDB; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "SRV"; }
    static const char *key(ServerMode value)
    {
        switch (value) {
                
            case SRVMODE_TERMINAL:  return "TERMINAL";
            case SRVMODE_GDB:       return "GDB";
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
    ServerMode mode;
    isize port;
}
RemoteServerConfig;
