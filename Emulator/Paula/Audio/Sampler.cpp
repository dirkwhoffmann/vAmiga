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
    write( TaggedSample { 0, 0 } );
}

void
Sampler::clone(const Sampler &other)
{
    *this = other;
}

template <SamplingMethod method> i16
Sampler::interpolate(Cycle clock)
{
    assert(!isEmpty());

    isize r1 = r;
    isize r2 = next(r1);

    // Remove all outdated entries
    while (r2 != w && elements[r2].tag <= clock) {
        
        skip(1);
        r1 = r2;
        r2 = next(r1);
    }
    assert(!isEmpty());

    // If the buffer contains a single element, return that element
    if (r2 == w) return elements[r1].sample;

    // Make sure that we've selected the right sample pair
    assert(clock >= elements[r1].tag && clock < elements[r2].tag);

    // Interpolate between position r1 and r2
    if constexpr (method == SMP_NONE) {

        return elements[r1].sample;
    }
    
    if constexpr (method == SMP_NEAREST) {
        
        const auto &e1 = elements[r1];
        const auto &e2 = elements[r2];

        return ((clock - e1.tag) < (e2.tag - clock)) ? e1.sample : e2.sample;
    }
    
    if constexpr (method == SMP_LINEAR) {

        const auto &e1 = elements[r1];
        const auto &e2 = elements[r2];

        double dx = (double)(e2.tag - e1.tag);
        double dy = (double)(e2.sample - e1.sample);
        double weight = (double)(clock - e1.tag) / dx;
        
        return (i16)(e1.sample + weight * dy);
    }
}

template i16 Sampler::interpolate<SMP_NONE>(Cycle clock);
template i16 Sampler::interpolate<SMP_NEAREST>(Cycle clock);
template i16 Sampler::interpolate<SMP_LINEAR>(Cycle clock);
