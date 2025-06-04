// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class FilterType : long
{
    NONE,
    A500,
    A1000,
    A1200,
    LOW,
    LED,
    HIGH
};

struct FilterTypeEnum : Reflection<FilterTypeEnum, FilterType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FilterType::HIGH);
    
    static const char *_key(FilterType value)
    {
        switch (value) {
                
            case FilterType::NONE:       return "NONE";
            case FilterType::A500:       return "A500";
            case FilterType::A1000:      return "A1000";
            case FilterType::A1200:      return "A1200";
            case FilterType::LOW:        return "LOW";
            case FilterType::LED:        return "LED";
            case FilterType::HIGH:       return "HIGH";
        }
        return "???";
    }
    static const char *help(FilterType value)
    {
        switch (value) {
                
            case FilterType::NONE:       return "No audio filter";
            case FilterType::A500:       return "Amiga 500 filter pipeline";
            case FilterType::A1000:      return "Amiga 1000 filter pipeline";
            case FilterType::A1200:      return "Amiga 1200 filter pipeline";
            case FilterType::LOW:        return "Low-pass filter only";
            case FilterType::LED:        return "LED filter only. Ignore the LED state";
            case FilterType::HIGH:       return "High-pass filter only";
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
