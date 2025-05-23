// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VACore/Foundation/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class SerialPortDevice
{
    NONE,
    NULLMODEM,
    LOOPBACK,
    RETROSHELL,
    COMMANDER
};

struct SerialPortDeviceEnum : Reflection<SerialPortDeviceEnum, SerialPortDevice>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(SerialPortDevice::COMMANDER);
    
    static const char *_key(SerialPortDevice value)
    {
        switch (value) {
                
            case SerialPortDevice::NONE:          return "NONE";
            case SerialPortDevice::NULLMODEM:     return "NULLMODEM";
            case SerialPortDevice::LOOPBACK:      return "LOOPBACK";
            case SerialPortDevice::RETROSHELL:    return "RETROSHELL";
            case SerialPortDevice::COMMANDER:     return "COMMANDER";
        }
        return "???";
    }
    static const char *help(SerialPortDevice value)
    {
        switch (value) {
                
            case SerialPortDevice::NONE:          return "No device";
            case SerialPortDevice::NULLMODEM:     return "Nullmodem cable";
            case SerialPortDevice::LOOPBACK:      return "Loopback cable";
            case SerialPortDevice::RETROSHELL:    return "RetroShell monitor";
            case SerialPortDevice::COMMANDER:     return "RetroShell commander";
        }
        return "???";
    }
};


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

}
