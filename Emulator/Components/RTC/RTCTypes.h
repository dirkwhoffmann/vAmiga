// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(RTC_REVISION)
{
    RTC_NONE,
    RTC_OKI,
    RTC_RICOH
};
typedef RTC_REVISION RTCRevision;

#ifdef __cplusplus
struct RTCRevisionEnum : util::Reflection<RTCRevisionEnum, RTCRevision>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = RTC_RICOH;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "RTC"; }
    static const char *key(RTCRevision value)
    {
        switch (value) {
                
            case RTC_NONE:   return "NONE";
            case RTC_OKI:    return "OKI";
            case RTC_RICOH:  return "RICOH";
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
    RTCRevision model;
}
RTCConfig;
