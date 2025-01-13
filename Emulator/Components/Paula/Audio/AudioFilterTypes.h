// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum_long(FILTER_TYPE)
{
    FILTER_NONE,
    FILTER_A500,
    FILTER_A1000,
    FILTER_A1200,
    FILTER_LOW,
    FILTER_LED,
    FILTER_HIGH
};
typedef FILTER_TYPE FilterType;

struct FilterTypeEnum : util::Reflection<FilterTypeEnum, FilterType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FILTER_HIGH;
    
    static const char *prefix() { return "FILTER"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case FILTER_NONE:       return "NONE";
            case FILTER_A500:       return "A500";
            case FILTER_A1000:      return "A1000";
            case FILTER_A1200:      return "A1200";
            case FILTER_LOW:        return "LOW";
            case FILTER_LED:        return "LED";
            case FILTER_HIGH:       return "HIGH";
        }
        return "???";
    }
    static const char *help(long value)
    {
        switch (value) {
                
            case FILTER_NONE:       return "No audio filter";
            case FILTER_A500:       return "Amiga 500 filter pipeline";
            case FILTER_A1000:      return "Amiga 1000 filter pipeline";
            case FILTER_A1200:      return "Amiga 1200 filter pipeline";
            case FILTER_LOW:        return "Low-pass filter only";
            case FILTER_LED:        return "LED filter only. Ignore the LED state";
            case FILTER_HIGH:       return "High-pass filter only";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    // The selected audio filter type
    FilterType filterType;
}
AudioFilterConfig;

}
