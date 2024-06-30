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

template <class T> void
AudioStream<T>::wipeOut()
{
    lock();
    this->clear(T(0,0));
    unlock();
}

template <class T> void
AudioStream<T>::fadeOut()
{
    lock();

    debug(AUDVOL_DEBUG, "Fading out (%ld samples)...\n", this->count());

    float scale = 1.0f;
    float delta = 1.0f / this->count();

    // Rescale the existing samples
    for (isize i = this->begin(); i != this->end(); i = this->next(i)) {

        scale -= delta;
        assert(scale >= -0.1 && scale < 1.0);

        this->elements[i].l *= scale;
        this->elements[i].r *= scale;
    }

    // Wipe out the rest of the buffer
    for (isize i = this->end(); i != this->begin(); i = this->next(i)) {

        this->elements[i] = { 0, 0 };
    }

    unlock();
}

template <class T> void
AudioStream<T>::alignWritePtr()
{
    lock();
    this->align(this->cap() / 2);
    unlock();
}

template <class T> void
AudioStream<T>::copy(float *buffer, isize n)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(this->count() >= n);

    for (isize i = 0; i < n; i++) {
        T sample = this->read();
        sample.copy(buffer, i);
    }
    return;
}

template <class T> void
AudioStream<T>::copy(float *buffer1, float *buffer2, isize n)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(this->count() >= n);

    for (isize i = 0; i < n; i++) {
        T sample = this->read();
        sample.copy(buffer1, buffer2, i);
    }
}

template <class T> float
AudioStream<T>::draw(u32 *buffer, isize width, isize height,
                     bool left, float highestAmplitude, u32 color) const
{
    isize dw = this->cap() / width;
    float newHighestAmplitude = 0.001f;
    
    // Clear buffer
    for (isize i = 0; i < width * height; i++) {
        buffer[i] = color & 0xFFFFFF;
    }
    
    // Draw waveform
    for (isize w = 0; w < width; w++) {
        
        // Read samples from ringbuffer
        T pair = this->current(w * dw);
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

//
// Instantiate template functions
//

template void AudioStream<SAMPLE_T>::wipeOut();
template void AudioStream<SAMPLE_T>::fadeOut();
template void AudioStream<SAMPLE_T>::alignWritePtr();
template void AudioStream<SAMPLE_T>::copy(float *, isize);
template void AudioStream<SAMPLE_T>::copy(float *, float *, isize);
template float AudioStream<SAMPLE_T>::draw(u32 *, isize, isize, bool, float, u32) const;

}
