// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

template <class T> void
AudioStream<T>::copyMono(float *buffer, isize n, Volume &vol)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(this->count() >= n);

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
                SamplePair pair = this->read();
                buffer[i] = pair.left + pair.right;
            }
            return;
        }
    }
    
    // Generic path: Modulate the volume
    for (isize i = 0; i < n; i++) {
        vol.shift();
        SamplePair pair = this->read();
        *buffer++ = (pair.left + pair.right) * vol.current;
    }
}

template <class T> void
AudioStream<T>::copy(float *left, float *right, isize n, Volume &vol)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(this->count() >= n);

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
                SamplePair pair = this->read();
                left[i] = pair.left;
                right[i] = pair.right;
            }
            return;
        }
    }
    
    // Generic path: Modulate the volume
    for (isize i = 0; i < n; i++) {
        vol.shift();
        SamplePair pair = this->read();
        left[i] = pair.left * vol.current;
        right[i] = pair.right * vol.current;
    }
}

template <class T> void
AudioStream<T>::copyInterleaved(float *buffer, isize n, Volume &vol)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(this->count() >= n);

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

            SamplePair *p = (SamplePair *)buffer;
            
            for (isize i = 0; i < n; i++) {
                p[i] = this->read();
            }
            return;
        }
    }
    
    // Generic path: Modulate the volume
    for (isize i = 0; i < n; i++) {
        vol.shift();
        SamplePair pair = this->read();
        *buffer++ = pair.left * vol.current;
        *buffer++ = pair.right * vol.current;
    }
}

template <class T> float
AudioStream<T>::draw(u32 *buffer, isize width, isize height,
                     bool left, float highestAmplitude, u32 color)
{
    isize dw = this->cap() / width;
    float newHighestAmplitude = 0.001;
    
    // Clear buffer
    for (isize i = 0; i < width * height; i++) {
        buffer[i] = color & 0xFFFFFF;
    }
    
    // Draw waveform
    for (isize w = 0; w < width; w++) {
        
        // Read samples from ringbuffer
        SamplePair pair = this->current(w * dw);
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

//
// Instantiate template functions
//

template void AudioStream<SamplePair>::copyMono(float *, isize, Volume &);
template void AudioStream<SamplePair>::copy(float *, float *, isize, Volume &);
template void AudioStream<SamplePair>::copyInterleaved(float *, isize, Volume &vol);
template float AudioStream<SamplePair>::draw(u32 *, isize, isize, bool, float, u32);
