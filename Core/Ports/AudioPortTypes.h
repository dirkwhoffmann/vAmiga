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

#include "AudioFilterTypes.h"
#include "SamplerTypes.h"

namespace vamiga {

//
// Structures
//

/// State of the audio port.
typedef struct
{
    // Indicates whether the final volume is zero
    bool isMuted;
}
AudioPortInfo;

/// Configuration options of the audio port.
typedef struct
{
    // Master volume (left and right channel)
    i64 volL;
    i64 volR;                               ///< Master volume (right)

    // Channel volumes and pan factors
    i64 vol[4];
    i64 pan[4];                             ///< Pan of each audio channel

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

/// Statistics collected by the audio port.
typedef struct
{
    isize bufferUnderflows;                 ///< Number of buffer underflows
    isize bufferOverflows;                  ///< Number of buffer overflows
    i64 producedSamples;                    ///< Number of generated samples
    i64 idleSamples;                        ///< Number of samples produced while no audio was playing
    i64 consumedSamples;                    ///< Number of samples consumed by the audio backend
    double fillLevel;                       ///< Current fill level of the audio buffer
}
AudioPortMetrics;

}
