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
#include "AudioFilterTypes.h"
#include "SamplerTypes.h"

//
// Structures
//

typedef struct
{
    // Selects how the audio buffer is resampled to match the target frequency
    SamplingMethod samplingMethod;

    // Master volume (left and right channel)
    i64 volL;
    i64 volR;

    // Channel volumes and pan factors
    i64 vol[4];
    i64 pan[4];

    // Performance booster
    bool idleFastPath;
}
MuxerConfig;

typedef struct
{
    isize bufferUnderflows;
    isize bufferOverflows;
    i64 producedSamples;
    i64 consumedSamples;
    double fillLevel;
}
MuxerStats;
