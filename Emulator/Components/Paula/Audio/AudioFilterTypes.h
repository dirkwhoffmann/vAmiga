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

enum_long(FILTER_TYPE)
{
    FILTER_NONE,
    FILTER_A500,
    FILTER_A1200,
    FILTER_VAMIGA,
    FILTER_LP,
    FILTER_LED,
    FILTER_HP,
    FILTER_LP_HP,
    FILTER_LP_LED_HP
};
typedef FILTER_TYPE FilterType;

#ifdef __cplusplus
struct FilterTypeEnum : util::Reflection<FilterTypeEnum, FilterType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FILTER_LP_LED_HP;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "FILTER"; }
    static const char *key(FilterType value)
    {
        switch (value) {
                
            case FILTER_NONE:       return "NONE";
            case FILTER_A500:       return "A500";
            case FILTER_A1200:      return "A1200";
            case FILTER_VAMIGA:     return "VAMIGA";
            case FILTER_LP:         return "LP";
            case FILTER_LED:        return "LED";
            case FILTER_HP:         return "HP";
            case FILTER_LP_HP:      return "LP_HP";
            case FILTER_LP_LED_HP:  return "LP_LED_HP";
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
    // The selected audio filter type
    FilterType filterType;
}
AudioFilterConfig;
