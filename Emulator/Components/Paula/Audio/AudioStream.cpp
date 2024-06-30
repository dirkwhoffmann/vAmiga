// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AudioStream.h"
#include <algorithm>

namespace vamiga {

void
AudioStream::wipeOut()
{
    lock();
    clear(FloatStereo(0,0));
    unlock();
}

void
AudioStream::fadeOut()
{
    lock();

    debug(AUDVOL_DEBUG, "Fading out (%ld samples)...\n", count());

    float scale = 1.0f;
    float delta = 1.0f / count();

    // Rescale the existing samples
    for (isize i = begin(); i != end(); i = next(i)) {

        scale -= delta;
        assert(scale >= -0.1 && scale < 1.0);

        elements[i].l *= scale;
        elements[i].r *= scale;
    }

    // Wipe out the rest of the buffer
    for (isize i = end(); i != begin(); i = next(i)) {

        elements[i] = { 0, 0 };
    }

    unlock();
}

void
AudioStream::alignWritePtr()
{
    lock();
    align(cap() / 2);
    unlock();
}

void
AudioStream::copy(float *buffer, isize n)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    for (isize i = 0; i < n; i++) {

        auto sample = read();
        sample.copy(buffer, i);
    }
    return;
}

void
AudioStream::copy(float *buffer1, float *buffer2, isize n)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    for (isize i = 0; i < n; i++) {

        auto sample = read();
        sample.copy(buffer1, buffer2, i);
    }
}

float
AudioStream::draw(u32 *buffer, isize width, isize height,
                     bool left, float highestAmplitude, u32 color) const
{
    isize dw = cap() / width;
    float newHighestAmplitude = 0.001f;
    
    // Clear buffer
    for (isize i = 0; i < width * height; i++) {
        buffer[i] = color & 0xFFFFFF;
    }
    
    // Draw waveform
    for (isize w = 0; w < width; w++) {
        
        // Read samples from ringbuffer
        auto pair = current(w * dw);
        float sample = pair.magnitude(left);
        
        if (sample == 0) {
            
            // Draw some noise to make it look sexy
            unsigned *ptr = buffer + width * height / 2 + w;
            *ptr = color;
            if (rand() % 2) *(ptr + width) = color;
            if (rand() % 2) *(ptr - width) = color;
            
        } else {
            
            // Remember the highest amplitude
            if (sample > newHighestAmplitude) newHighestAmplitude = sample;
            
            // Scale the sample
            isize scaled = isize(sample * height / highestAmplitude);
            if (scaled > height) scaled = height;
            
            // Draw vertical line
            u32 *ptr = buffer + width * ((height - scaled) / 2) + w;
            for (isize j = 0; j < scaled; j++, ptr += width) *ptr = color;
        }
    }
    return newHighestAmplitude;
}

}
