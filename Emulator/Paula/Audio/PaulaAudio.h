// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _PAULA_AUDIO_H
#define _PAULA_AUDIO_H

#include "StateMachine.h"
#include "AudioFilter.h"
#include "Muxer.h"
#include "AudioStream.h"
#include "Buffers.h"

class PaulaAudio : public AmigaComponent {
    
    //
    // Sub components
    //
    
public:

    // State machines
    StateMachine<0> channel0 = StateMachine<0>(amiga);
    StateMachine<1> channel1 = StateMachine<1>(amiga);
    StateMachine<2> channel2 = StateMachine<2>(amiga);
    StateMachine<3> channel3 = StateMachine<3>(amiga);

    // Muxer
    Muxer muxer = Muxer(amiga);
    
    
    //
    // Properties
    //
    
private:

    // The component has been executed up to this clock cycle
    Cycle clock = 0;


    //
    // Initializing
    //
    
public:
    
    PaulaAudio(Amiga& ref);
    
    void _reset(bool hard) override;
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }
    
    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker
        
        & clock;
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Managing the ringbuffer
    //
    
public:
                
    // Reads samples from the ringbuffer (mono stream format)
    void readMonoSamples(float *buffer, size_t n);
    
    // Reads samples from the ringbuffer (stereo stream format)
    void readStereoSamples(float *left, float *right, size_t n);
    

    //
    // Emulating the device
    //

public:
    
    // Emulates the device until the given master clock cycle has been reached
    void executeUntil(Cycle targetClock);
};

#endif
