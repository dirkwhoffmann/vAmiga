// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AUDIO_STREAM_H
#define _AUDIO_STREAM_H

#include "HardwareComponent.h"

class AudioStream : public AmigaComponent {

public:
    
    // Audio samples
    RingBuffer <SamplePair, 16384> samples;

    /* Copies n audio samples into a memory buffer. These functions mark the
     * final step in the audio pipeline. They are used to copy the generated
     * sound samples into the buffers of the native sound device. In additon
     * to copying, the volume is modulated and audio filters can be applied.
     */
    void copy(float *left, float *right, size_t n,
              float &volume, float targetVolume, float volumeDelta);

    void copy(float *left, float *right, size_t n,
              float &volume, float targetVolume, float volumeDelta,
              AudioFilter &filterL, AudioFilter &filterR);
    
    void copyMono(float *buffer, size_t n,
                  float &volume, float targetVolume, float volumeDelta);
    
    void copyMono(float *buffer, size_t n,
                  float &volume, float targetVolume, float volumeDelta,
                  AudioFilter &filterL, AudioFilter &filterR);

};

#endif
