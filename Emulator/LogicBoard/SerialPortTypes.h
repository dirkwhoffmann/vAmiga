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
    static long min() { return 0; }
    static long max() { return SPD_LOOPBACK; }
    static bool isValid(long value) { return value >= min() && value <= max(); }
    
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
