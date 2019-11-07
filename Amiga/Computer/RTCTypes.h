// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _RTC_T_INC
#define _RTC_T_INC

//
// Enumerations
//

typedef enum : long
{
    RTC_NONE,
    RTC_M6242B
}
RTCModel;

inline bool isRTCModel(long value)
{
    return value >= RTC_NONE && value <= RTC_M6242B;
}

inline const char *RTCModelName(RTCModel model)
{
    assert(isRTCModel(model));

    switch (model) {
        case RTC_NONE:   return "RTC_NONE";
        case RTC_M6242B: return "RTC_M6242B";
        default:         return "???";
    }
}

typedef struct
{
    RTCModel model;
}
RTCConfig;

#endif
