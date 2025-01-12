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

namespace vamiga {

//
// Enumerations
//

enum_long(PROBE)
{
    PROBE_NONE,
    PROBE_MEMORY,
    PROBE_IPL
};
typedef PROBE Probe;

struct ProbeEnum : util::Reflection<ProbeEnum, Probe>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = PROBE_IPL;
    
    static const char *prefix() { return "PROBE"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case PROBE_NONE:            return "NONE";
            case PROBE_MEMORY:          return "MEMORY";
            case PROBE_IPL:             return "IPL";
        }
        return "???";
    }
    static const char *help(long value)
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

}
