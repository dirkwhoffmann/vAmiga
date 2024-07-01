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
    {   SYNCHRONIZED

        clear(SamplePair{0,0});
    }
}

void
AudioStream::eliminateCracks()
{
    {   SYNCHRONIZED

        debug(AUDVOL_DEBUG, "Eliminating cracks (%ld samples)...\n", count());

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
    }
}

void
AudioStream::alignWritePtr()
{
    {   SYNCHRONIZED

        align(cap() / 2);
    }
}

isize
AudioStream::copyMono(float *buffer, isize n)
{
    {   SYNCHRONIZED

        // If a buffer underflow occurs ...
        if (auto cnt = count(); cnt < n) {

            // ... copy all we have while stepwise lowering the volume ...
            for (isize i = 0; i < cnt; i++) {

                auto pair = read();
                *buffer++ = (pair.l + pair.r) * float(cnt - i) / float(cnt);
            }
            assert(isEmpty());

            // ... and fill the rest with zeroes.
            for (isize i = cnt; i < n; i++) *buffer++ = 0;

            return cnt;
        }

        // The standard case: The buffer contains enough samples
        for (isize i = 0; i < n; i++) {

            auto sample = read();
            buffer[i] = 0.5f * (sample.l + sample.r);
        }

        return n;
    }
}

isize
AudioStream::copyStereo(float *left, float *right, isize n)
{
    {   SYNCHRONIZED

        // If a buffer underflow occurs ...
        if (auto cnt = count(); cnt < n) {

            // ... copy all we have while stepwise lowering the volume ...
            for (isize i = 0; i < cnt; i++) {

                auto pair = read();
                *left++ = pair.l * float(cnt - i) / float(cnt);
                *right++ = pair.r * float(cnt - i) / float(cnt);
            }
            assert(isEmpty());

            // ... and fill the rest with zeroes.
            for (isize i = cnt; i < n; i++) *left++ = *right++ = 0;

            return cnt;
        }

        // The standard case: The buffer contains enough samples
        for (isize i = 0; i < n; i++) {

            auto sample = read();
            left[i] = sample.l;
            right[i] = sample.r;
        }

        return n;
    }
}

isize
AudioStream::copyInterleaved(float *buffer, isize n)
{
    {   SYNCHRONIZED

        // If a buffer underflow occurs ...
        if (auto cnt = count(); cnt < n) {

            // ... copy all we have while stepwise lowering the volume ...
            for (isize i = 0; i < cnt; i++) {

                auto pair = read();
                *buffer++ = pair.l * float(cnt - i) / float(cnt);
                *buffer++ = pair.r * float(cnt - i) / float(cnt);
            }
            assert(isEmpty());

            // ... and fill the rest with zeroes.
            for (isize i = cnt; i < n; i++) *buffer++ = *buffer++ = 0;

            return cnt;
        }

        // The standard case: The buffer contains enough samples
        for (isize i = 0; i < n; i++) {

            auto sample = read();
            *buffer++ = sample.l;
            *buffer++ = sample.r;
        }

        return n;
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
        float sample = left ? std::abs(pair.l) : std::abs(pair.r);
        
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
