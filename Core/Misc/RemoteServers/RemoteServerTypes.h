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
#include "TransportTypes.h"

namespace vamiga {

//
// Structures
//

typedef struct
{
    // Enable status
    bool enable;

    // The socket port number of this server
    u16 port;

    // The transport used to communicate with clients
    TransportProtocol transport;

    // If true, transmitted packets are shown in RetroShell
    bool verbose;
}
ServerConfig;

typedef struct
{
    SrvState state;
}
RemoteServerInfo;

}
