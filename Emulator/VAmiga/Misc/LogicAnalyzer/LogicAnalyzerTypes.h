// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BusTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class Probe : long
{
    NONE,
    MEMORY,
    IPL
};

struct ProbeEnum : Reflection<ProbeEnum, Probe>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Probe::IPL);
    
    static const char *_key(Probe value)
    {
        switch (value) {
                
            case Probe::NONE:            return "NONE";
            case Probe::MEMORY:          return "MEMORY";
            case Probe::IPL:             return "IPL";
        }
        return "???";
    }
    static const char *help(Probe value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct
{
    // The logic analyzer channels
    Probe channel[4];
    
    // Probe address (in case of memory probing)
    u32 addr[4];
}
LogicAnalyzerConfig;

typedef struct
{

    const BusOwner *busOwner;
    const u32 *addrBus;
    const u16 *dataBus;
    const isize *channel[4];
}
LogicAnalyzerInfo;

}
