// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "PortPublicTypes.h"
#include "Reflection.h"

struct SerialPortDeviceEnum : Reflection<SerialPortDeviceEnum, SerialPortDevice> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < SPD_COUNT;
    }
    
    static const char *prefix() { return "SPD"; }
    static const char *key(SerialPortDevice value)
    {
        switch (value) {
                
            case SPD_NONE:      return "NONE";
            case SPD_LOOPBACK:  return "LOOPBACK";
            case SPD_COUNT:     return "???";
        }
        return "???";
    }
};

struct PortNrEnum : Reflection<PortNrEnum, PortNr> {
    
    static bool isValid(long value)
    {
        return value == PORT_1 || PORT_2;
    }
    
    static const char *prefix() { return ""; }
    static const char *key(PortNr value)
    {
        switch (value) {
                
            case PORT_1:  return "PORT_1";
            case PORT_2:  return "PORT_2";
        }
        return "???";
    }
};
