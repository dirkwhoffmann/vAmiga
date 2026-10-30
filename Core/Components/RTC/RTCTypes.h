// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {

//
// Enumerations
//

/// Real-time clock chip revision.
enum class RTCRevision : long
{
    NONE,
    OKI,
    RICOH
};

struct RTCRevisionEnum : utl::Reflectable<RTCRevisionEnum, RTCRevision>
{
    static constexpr auto minVal = (long)RTCRevision::NONE;
    static constexpr auto maxVal = (long)RTCRevision::RICOH;
    
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

/// Configuration options of the real-time clock.
typedef struct
{
    RTCRevision model;                      ///< Chip revision
}
RTCConfig;

}
