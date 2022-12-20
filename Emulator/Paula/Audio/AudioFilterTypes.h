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
    FILTER_BUTTERWORTH
};
typedef FILTER_TYPE FilterType;

#ifdef __cplusplus
struct FilterTypeEnum : util::Reflection<FilterTypeEnum, FilterType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FILTER_BUTTERWORTH;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "FILTER"; }
    static const char *key(FilterType value)
    {
        switch (value) {
                
            case FILTER_NONE:         return "NONE";
            case FILTER_BUTTERWORTH:  return "BUTTERWORTH";
        }
        return "???";
    }
};
#endif

enum_long(FILTER_ACTIVATION)
{
    FILTER_AUTO_ENABLE,
    FILTER_ALWAYS_ON,
    FILTER_ALWAYS_OFF,
};
typedef FILTER_TYPE FilterActivation;

#ifdef __cplusplus
struct FilterActivationEnum : util::Reflection<FilterActivationEnum, FilterActivation>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FILTER_ALWAYS_OFF;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "FILTER"; }
    static const char *key(FilterType value)
    {
        switch (value) {

            case FILTER_AUTO_ENABLE:    return "AUTO_ENABLE";
            case FILTER_ALWAYS_ON:      return "ALWAYS_ON";
            case FILTER_ALWAYS_OFF:     return "ALWAYS_OFF";
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

    // Filter activation mode
    FilterActivation filterActivation;
}
AudioFilterConfig;
