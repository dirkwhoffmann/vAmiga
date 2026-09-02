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


enum class TrafficDirection : long
{
    RECEIVED,       // The packet was received from a client
    SENT            // The packet was sent to a client
};

struct TrafficDirectionEnum : Reflectable<TrafficDirectionEnum, TrafficDirection>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(TrafficDirection::SENT);

    static const char *_key(TrafficDirection value)
    {
        switch (value) {

            case TrafficDirection::RECEIVED:    return "RECEIVED";
            case TrafficDirection::SENT:        return "SENT";
        }
        return "???";
    }
    static const char *help(TrafficDirection value)
    {
        switch (value) {

            case TrafficDirection::RECEIVED:    return "Received from a client";
            case TrafficDirection::SENT:        return "Sent to a client";
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

/* A single entry of the traffic log. The remote servers record all transmitted
 * packets in a bounded log inside the RemoteManager. Each recorded packet is
 * assigned a monotonically increasing sequence number which is passed to the
 * GUI in the Msg::SRV_RECEIVE and Msg::SRV_SEND notification messages.
 */
typedef struct
{
    // Sequence number (unique, monotonically increasing)
    isize nr;

    // The server that received or sent the packet
    ServerType server;

    // Tells whether the packet was received or sent
    TrafficDirection direction;

    // Host time stamp (milliseconds since epoch)
    i64 time;

    // Packet contents
    string payload;
}
TrafficEntry;

}
