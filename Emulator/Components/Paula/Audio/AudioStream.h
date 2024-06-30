// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "CoreObject.h"
#include "Concurrency.h"
#include "RingBuffer.h"
#include "Synchronizable.h"

namespace vamiga {

/* About the AudioStream
 *
 * The audio stream is the last element in the audio pipeline. It is a temporary
 * storage for the final audio samples, waiting to be handed over to the audio
 * unit of the host machine.
 *
 * The audio stream is designes as a ring buffer, because samples are written
 * and read asynchroneously. Since reading and writing is carried out in
 * different threads, accesses to the audio stream need to be preceded by a call
 * to lock() and followed by a call to unlock().
*/

struct FloatStereo
{
    float l;
    float r;
    
    FloatStereo() : l(0.0f), r(0.0f) { }
    FloatStereo(float l, float r) : l(l * AUD_SCALE), r(r * AUD_SCALE) { }
};


//
// AudioStream
//

class AudioStream : public CoreObject, public Synchronizable, public util::RingBuffer <FloatStereo, 16384> {

    // Mutex for synchronizing read / write accesses
    // util::ReentrantMutex mutex;

public:
    
    const char *objectName() const override { return "AudioStream"; }

    // Locks or unlocks the mutex
    // void lock() { mutex.lock(); }
    // void unlock() { mutex.unlock(); }

    // Initializes the ring buffer with zeroes
    void wipeOut();
    
    // Rescales the existing samples to gradually fade out (to avoid cracks)
    void fadeOut();

    // Adds a sample to the ring buffer
    void add(const FloatStereo &lr) { this->write(lr); }
    void add(float l, float r) { this->write(FloatStereo(l,r)); }

    // Puts the write pointer somewhat ahead of the read pointer
    void alignWritePtr();
    
    
    //
    // Copying data
    //
    
    /* Copies n audio samples into a memory buffer. These functions mark the
     * final step in the audio pipeline. They are used to copy the generated
     * sound samples into the buffers of the native sound device. In additon
     * to copying, the volume is modulated if the music is supposed to fade
     * in or fade out.
     */
    isize copyMono(float *buffer, isize n);
    isize copyStereo(float *left, float *right, isize n);

    
    //
    // Visualizing the waveform
    //
    
    /* Plots a graphical representation of the waveform. Returns the highest
     * amplitute that was found in the ringbuffer. To implement auto-scaling,
     * pass the returned value as parameter highestAmplitude in the next call
     * to this function.
     */
    float draw(u32 *buffer, isize width, isize height,
               bool left, float highestAmplitude, u32 color) const;
};

}
