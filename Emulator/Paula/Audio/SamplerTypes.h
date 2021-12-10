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

enum_long(SMP_METHOD)
{
    SMP_NONE,
    SMP_NEAREST,
    SMP_LINEAR
};
typedef SMP_METHOD SamplingMethod;

#ifdef __cplusplus
struct SamplingMethodEnum : util::Reflection<SamplingMethodEnum, SamplingMethod>
{
    static long minVal() { return 0; }
    static long maxVal() { return SMP_LINEAR; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "SMP"; }
    static const char *key(SamplingMethod value)
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
