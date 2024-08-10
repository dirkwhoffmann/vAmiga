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

//
// Enumerations
//

enum_long(SMP_METHOD)
{
    SMP_NONE,
    SMP_NEAREST,
    SMP_LINEAR
};
typedef SMP_METHOD SamplingMethod;

#ifdef __cplusplus
struct SamplingMethodEnum : vamiga::util::Reflection<SamplingMethodEnum, SamplingMethod>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = SMP_LINEAR;

    static const char *prefix() { return "SMP"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case SMP_NONE:     return "NONE";
            case SMP_NEAREST:  return "NEAREST";
            case SMP_LINEAR:   return "LINEAR";
        }
        return "???";
    }
};
#endif
