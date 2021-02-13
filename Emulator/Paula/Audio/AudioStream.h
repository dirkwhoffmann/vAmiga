// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HardwareComponent.h"
#include "Concurrency.h"

struct Volume {

    // Maximum volume
    // constexpr const static float maxVolume = 1.0;

    // Current volume (will eventually reach the target volume)
    float current = 1.0;

    // Target volume
    float target = 1.0;

    // Delta steps (added to volume until the target volume is reached)
    float delta = 0;

    bool fading() { return current != target; }
    bool silent() { return current == 0.0; }
    
    // Shifts the current volume towards the target volume
    void shift() {
        if (current < target) {
            current += MIN(delta, target - current);
        } else {
            current -= MIN(delta, current - target);
        }
    }
};

typedef struct
{
    float left;
    float right;
}
SamplePair;

class AudioStream : public RingBuffer <SamplePair, 16384> {

    // Mutex for synchronizing read / write accesses 
    Mutex mutex;

public:
    
    // Locks or unlocks the synchronization mutex
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    /* Aligns the write pointer. This function puts the write pointer somewhat
     * ahead of the read pointer. With a standard sample rate of 44100 Hz,
     * 735 samples is 1/60 sec.
     */
    static constexpr u32 samplesAhead() { return 8 * 735; }
    void alignWritePtr() { align(samplesAhead()); }
    
    
    //
    // Copying data
    //
    
    /* Copies n audio samples into a memory buffer. These functions mark the
     * final step in the audio pipeline. They are used to copy the generated
     * sound samples into the buffers of the native sound device. In additon
     * to copying, the volume is modulated and audio filters can be applied.
     */
    void copyMono(float *buffer, isize n, Volume &vol);
    void copy(float *left, float *right, isize n, Volume &vol);
    void copyInterleaved(float *buffer, isize n, Volume &vol);
    
    
    //
    // Visualizing the waveform
    //
    
    /* Plots a graphical representation of the waveform. Returns the highest
     * amplitute that was found in the ringbuffer. To implement auto-scaling,
     * pass the returned value as parameter highestAmplitude in the next call
     * to this function.
     */
    float draw(u32 *buffer, isize width, isize height,
               bool left, float highestAmplitude, u32 color);
};
