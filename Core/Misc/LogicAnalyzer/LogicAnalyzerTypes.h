// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

/// @file

#pragma once

#include "BusTypes.h"

namespace vamiga {

//
// Enumerations
//

/// Signal that is recorded by the logic analyzer.
enum class Probe : long
{
    NONE,
    MEMORY,
    IPL
};

struct ProbeEnum : utl::Reflectable<ProbeEnum, Probe>
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

/// A sample recorded by the logic analyzer.
typedef struct {
    
    // Position
    i64 frame;
    isize vpos;
    isize hpos;

    // Bus usage
    BusOwner owner;
    u16 dataBus;
    u32 addrBus;
    
    // Probe values
    isize values[4];
    
} LogicAnalyzerSample;

/// Configuration options of the logic analyzer.
typedef struct
{
    // Enable switch
    bool connect;
    
    // The logic analyzer channels
    Probe channel[4];
    
    // Probe address (in case of memory probing)
    u32 addr[4];
}
LogicAnalyzerConfig;

/// State of the logic analyzer.
typedef struct
{

    const BusOwner *busOwner;               ///< Bus owner of each recorded slot
    const u32 *addrBus;                     ///< Address bus value of each recorded slot
    const u16 *dataBus;                     ///< Data bus value of each recorded slot
}
LogicAnalyzerInfo;

}
