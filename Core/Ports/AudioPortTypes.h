// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AudioFilterTypes.h"
#include "SamplerTypes.h"

namespace vamiga {

//
// Structures
//

typedef struct
{
    // Indicates whether the final volume is zero
    bool isMuted;
}
AudioPortInfo;

typedef struct
{
    // Master volume (left and right channel)
    i64 volL;
    i64 volR;

    // Channel volumes and pan factors
    i64 vol[4];
    i64 pan[4];

    // Sample buffer capacity
    isize bufferSize;

    // Selects how the audio buffer is resampled to match the target frequency
    SamplingMethod samplingMethod;

    // Adaptive Sample Rate enable switch
    bool asr;

    // Performance booster
    bool idleFastPath;
}
AudioPortConfig;

typedef struct
{
    isize bufferUnderflows;
    isize bufferOverflows;
    i64 producedSamples;
    i64 idleSamples;
    i64 consumedSamples;
    double fillLevel;
}
AudioPortMetrics;

}
