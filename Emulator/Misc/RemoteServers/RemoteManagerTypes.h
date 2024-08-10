// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(SERVER_TYPE)
{
    SERVER_SER,
    SERVER_RSH,
    SERVER_GDB
};
typedef SERVER_TYPE ServerType;

#ifdef __cplusplus
struct ServerTypeEnum : vamiga::util::Reflection<ServerTypeEnum, ServerType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = SERVER_GDB;
    
    static const char *prefix() { return "SERVER"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case SERVER_SER:    return "SER";
            case SERVER_RSH:    return "RSH";
            case SERVER_GDB:    return "GDB";
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
    isize numLaunching;
    isize numListening;
    isize numConnected;
    isize numErroneous;
}
RemoteManagerInfo;
