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

/* This buffer type temporarily stores the sound samples produced by the state
 * machine. The state machine doesn't output at a constant sampling rate.
 * Instead, it generates a new sample whenever the period counter underflows.
 * Each sample is tagged with the cycle in which the underflow occurred to
 * preserve the timing information.
 */

struct Sampler : util::SortedRingBuffer <i16, VPOS_CNT * HPOS_CNT_PAL> {
    
    // Initializes the ring buffer with a single dummy element
    void reset();

    // Interpolates a sound sample for the specified target cycle
    template <SamplingMethod method> i16 interpolate(Cycle clock);

    // Returns true if there are at least two sound samples
    bool isActive() { return count() != 1; }
};

}
