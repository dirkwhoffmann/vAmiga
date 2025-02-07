// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga/Foundation/Reflection.h"

//
// Enumerations
//

namespace vamiga {

enum class SamplingMethod
{
    NONE,
    NEAREST,
    LINEAR
};

struct SamplingMethodEnum : Reflection<SamplingMethodEnum, SamplingMethod>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(SamplingMethod::LINEAR);
    
    static const char *_key(SamplingMethod value)
    {
        switch (value) {
                
            case SamplingMethod::NONE:     return "NONE";
            case SamplingMethod::NEAREST:  return "NEAREST";
            case SamplingMethod::LINEAR:   return "LINEAR";
        }
        return "???";
    }
    static const char *help(SamplingMethod value)
    {
        switch (value) {
                
            case SamplingMethod::NONE:     return "Latest sample";
            case SamplingMethod::NEAREST:  return "Nearest neighbor";
            case SamplingMethod::LINEAR:   return "Linear interpolation";
        }
        return "???";
    }
};

}
