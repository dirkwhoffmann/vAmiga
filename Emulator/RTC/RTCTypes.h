// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _RTC_TYPES_H
#define _RTC_TYPES_H

#include "VA_ENUM.h"

//
// Enumerations
//

typedef VA_ENUM(long, RTCModel)
{
    RTC_NONE,
    RTC_OKI,
    RTC_RICOH
};

inline bool isRTCModel(long value)
{
    return value >= RTC_NONE && value <= RTC_RICOH;
}

inline const char *RTCModelName(RTCModel model)
{
    assert(isRTCModel(model));

    switch (model) {
        case RTC_NONE:   return "RTC_NONE";
        case RTC_OKI:    return "RTC_OKI";
        case RTC_RICOH:  return "RTC_RICOH";
        default:         return "???";
    }
}

typedef struct
{
    RTCModel model;
}
RTCConfig;

#endif
