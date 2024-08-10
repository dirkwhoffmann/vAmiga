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

enum_long(SPD)
{
    SPD_NONE,
    SPD_NULLMODEM,
    SPD_LOOPBACK,
    SPD_RETROSHELL,
    SPD_COMMANDER
};
typedef SPD SerialPortDevice;

#ifdef __cplusplus
struct SerialPortDeviceEnum : vamiga::util::Reflection<SerialPortDeviceEnum, SerialPortDevice>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = SPD_COMMANDER;
    
    static const char *prefix() { return "SPD"; }
    static const char *_key(long value)
    {
        switch (value) {

            case SPD_NONE:          return "NONE";
            case SPD_NULLMODEM:     return "NULLMODEM";
            case SPD_LOOPBACK:      return "LOOPBACK";
            case SPD_RETROSHELL:    return "RETROSHELL";
            case SPD_COMMANDER:     return "COMMANDER";
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
    SerialPortDevice device;
    bool verbose;
}
SerialPortConfig;

typedef struct
{
    u32 port;

    bool txd;
    bool rxd;
    bool rts;
    bool cts;
    bool dsr;
    bool cd;
    bool dtr;
}
SerialPortInfo;
