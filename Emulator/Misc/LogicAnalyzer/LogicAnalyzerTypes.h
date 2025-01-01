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

enum_long(PROBE)
{
    PROBE_NONE,
    PROBE_BUS_OWNER,
    PROBE_ADDR_BUS,
    PROBE_DATA_BUS,
    PROBE_MEMORY
};
typedef PROBE Probe;

#ifdef __cplusplus
struct ProbeEnum : vamiga::util::Reflection<ProbeEnum, Probe>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = PROBE_MEMORY;

    static const char *prefix() { return "PROBE"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case PROBE_NONE:            return "NONE";
            case PROBE_BUS_OWNER:       return "BUS_OWNER";
            case PROBE_ADDR_BUS:        return "ADDR_BUS";
            case PROBE_DATA_BUS:        return "DATA_BUS";
            case PROBE_MEMORY:          return "MEMORY";
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
    // The logic analyzer channels
    Probe channel[4];
    
    // Probe address (in case of memory probing)
    u32 addr[4];
}
LogicAnalyzerConfig;
