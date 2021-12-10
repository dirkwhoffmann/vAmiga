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

enum_long(SPD)
{
    SPD_NONE,
    SPD_LOOPBACK
};
typedef SPD SerialPortDevice;

#ifdef __cplusplus
struct SerialPortDeviceEnum : util::Reflection<SerialPortDeviceEnum, SerialPortDevice>
{
    static long minVal() { return 0; }
    static long maxVal() { return SPD_LOOPBACK; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "SPD"; }
    static const char *key(SerialPortDevice value)
    {
        switch (value) {
                
            case SPD_NONE:      return "NONE";
            case SPD_LOOPBACK:  return "LOOPBACK";
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
