// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
AudioStream::copyMono(float *buffer, isize n, Volume &vol)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    // Quick path: Volume is stable at 0 or 1
    if (!vol.fading()) {

        if (vol.current == 0) {

            for (isize i = 0; i < n; i++) {
                buffer[i] = 0;
            }
            return;
        }
        if (vol.current == 1.0) {

            for (isize i = 0; i < n; i++) {
                SamplePair pair = read();
                buffer[i] = pair.left + pair.right;
            }
            return;
        }
    }
    
    // Generic path: Modulate the volume
    for (isize i = 0; i < n; i++) {
        vol.shift();
        SamplePair pair = read();
        *buffer++ = (pair.left + pair.right) * vol.current;
    }
}

void
AudioStream::copy(float *left, float *right, isize n, Volume &vol)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    // Quick path: Volume is stable at 0 or 1
    if (!vol.fading()) {

        if (vol.current == 0) {

            for (isize i = 0; i < n; i++) {
                left[i] = 0;
                right[i] = 0;
            }
            return;
        }
        if (vol.current == 1.0) {

            for (isize i = 0; i < n; i++) {
                SamplePair pair = read();
                left[i] = pair.left;
                right[i] = pair.right;
            }
            return;
        }
    }
    
    // Generic path: Modulate the volume
    for (isize i = 0; i < n; i++) {
        vol.shift();
        SamplePair pair = read();
        left[i] = pair.left * vol.current;
        right[i] = pair.right * vol.current;
    }
}

void
AudioStream::copyInterleaved(float *buffer, isize n, Volume &vol)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    // Quick path: Volume is stable at 0 or 1
    if (!vol.fading()) {

        if (vol.current == 0) {

            for (isize i = 0; i < n; i++) {
                *buffer++ = 0;
                *buffer++ = 0;
            }
            return;
        }
        if (vol.current == 1.0) {

            for (isize i = 0; i < n; i++) {
                SamplePair pair = read();
                *buffer++ = pair.left;
                *buffer++ = pair.right;
            }
            return;
        }
    }
    
    // Generic path: Modulate the volume
    for (isize i = 0; i < n; i++) {
        vol.shift();
        SamplePair pair = read();
        *buffer++ = pair.left * vol.current;
        *buffer++ = pair.right * vol.current;
    }
}

float
AudioStream::draw(u32 *buffer, isize width, isize height,
                  bool left, float highestAmplitude, u32 color)
{
    isize dw = cap() / width;
    float newHighestAmplitude = 0.001;
    
    // Clear buffer
    for (isize i = 0; i < width * height; i++) {
        buffer[i] = color & 0xFFFFFF;
    }
    
    // Draw waveform
    for (isize w = 0; w < width; w++) {
        
        // Read samples from ringbuffer
        SamplePair pair = current(w * dw);
        float sample = left ? abs(pair.left) : abs(pair.right);
        
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
