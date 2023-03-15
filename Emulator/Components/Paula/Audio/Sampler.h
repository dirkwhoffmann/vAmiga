// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SamplerTypes.h"
#include "Constants.h"
#include "RingBuffer.h"
#include "Reflection.h"

namespace vamiga {

/* This buffer type is used to temporarily store the sound samples produced by
 * the state machine. Keep in mind that the state machine doesn't output
 * at a constant sampling rate. Instead, a new sample is generated whenever the
 * period counter underflows. To preserve this timing information, each sample
 * is tagged by the cycle it was produced.
 */

struct Sampler : util::SortedRingBuffer <i16, VPOS_CNT * HPOS_CNT_PAL> {
    
    // Initializes the ring buffer with a single dummy element
    void reset();

    // Interpolates a sound sample for the specified target cycle
    template <SamplingMethod method> i16 interpolate(Cycle clock);
};

}
