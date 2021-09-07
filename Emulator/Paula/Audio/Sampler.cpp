// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Sampler.h"

void
Sampler::reset()
{
    // Clear the ringbuffer
    clear();

    // Add a dummy element to ensure the buffer is not empty
    append(0,0);
}

template <SamplingMethod method> i16
Sampler::interpolate(Cycle clock)
{
    /* Interploation involves two major steps. In the first step, the function
     * computes index position r1 with the following property:
     *
     *     Cycle of sample at r1 <= Target cycle < Cycle of sample at r1 + 1
     *
     * In the second step, the function interpolated between the two samples at
     * r1 and r1 + 1 based on the requested method.
     */

    assert(!isEmpty());

    isize r1 = r;
    isize r2 = next(r1);

    // Remove all outdated entries
    while (r2 != w && keys[r2] <= clock) {
        
        skip();
        r1 = r2;
        r2 = next(r1);
    }
    assert(!isEmpty());

    // If the buffer contains a single element, return that element
    if (r2 == w) return elements[r1];

    // Make sure that we've selected the right sample pair
    assert(clock >= keys[r1] && clock < keys[r2]);

    // Interpolate between position r1 and r2
    if constexpr (method == SMP_NONE) {

        return elements[r1];
    }
    
    if constexpr (method == SMP_NEAREST) {
        
        return ((clock - keys[r1]) < (keys[r2] - clock)) ? elements[r1] : elements[r2];
    }
    
    if constexpr (method == SMP_LINEAR) {

        double dx = (double)(keys[r2] - keys[r1]);
        double dy = (double)(elements[r2] - elements[r1]);
        double weight = (double)(clock - keys[r1]) / dx;
        
        return (i16)(elements[r1] + weight * dy);
    }
}

template i16 Sampler::interpolate<SMP_NONE>(Cycle clock);
template i16 Sampler::interpolate<SMP_NEAREST>(Cycle clock);
template i16 Sampler::interpolate<SMP_LINEAR>(Cycle clock);
