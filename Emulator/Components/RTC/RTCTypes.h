// -----------------------------------------------------------------------------
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

/*
enum_long(RTC_REVISION)
{
    RTC_NONE,
    RTC_OKI,
    RTC_RICOH
};
typedef RTC_REVISION RTCRevision;
*/
enum class RTCRevision : long
{
    NONE,
    OKI,
    RICOH
};

struct RTCRevisionEnum : util::Reflection<RTCRevisionEnum, RTCRevision>
{
    static constexpr auto minVal = (long)RTCRevision::NONE;
    static constexpr auto maxVal = (long)RTCRevision::RICOH;
    
    static const char *prefix() { return "RTC"; }
    static const char *_key(RTCRevision value)
    {
        switch (value) {
                
            case RTCRevision::NONE:   return "NONE";
            case RTCRevision::OKI:    return "OKI";
            case RTCRevision::RICOH:  return "RICOH";
        }
        return "???";
    }
    static const char *help(RTCRevision value)
    {
        switch (value) {
                
            case RTCRevision::NONE:   return "No real-time clock";
            case RTCRevision::OKI:    return "MSM6242B";
            case RTCRevision::RICOH:  return "RP5C01A";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    RTCRevision model;
}
RTCConfig;

}
