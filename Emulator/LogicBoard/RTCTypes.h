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

#include "Aliases.h"

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

inline bool isRTCRevision(long value)
{
    return (unsigned long)value <= RTC_RICOH;
}

inline const char *RTCRevisionName(RTCRevision value)
{
    switch (value) {
            
        case RTC_NONE:   return "RTC_NONE";
        case RTC_OKI:    return "RTC_OKI";
        case RTC_RICOH:  return "RTC_RICOH";
    }
    return "???";
}

typedef struct
{
    RTCRevision model;
}
RTCConfig;

#endif
