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

    // Current configuration
    AudioConfig config;

    // Result of the latest inspection
    AudioInfo info;

    // Current workload
    AudioStats stats;


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
    
    // Audio filters (DEPRECATED. WILL BE PART OF THE MUXER)
    AudioFilter filterL = AudioFilter(amiga);
    AudioFilter filterR = AudioFilter(amiga);

    
    //
    // Properties
    //
    
private:

    // The component has been executed up to this clock cycle
    double clock = 0;

    // Master clock cycles per audio sample, set in setSampleRate()
    double cyclesPerSample = 0;

    // Time stamp of the last write pointer alignment
    Cycle lastAlignment = 0;

    
    //
    // Sample buffers
    //

public:
    
    /* The audio sample ringbuffer. This ringbuffer is used to transfer sound
     * samples from the emulator to the sound device of the host machine.
     * DEPRECATED. REPLACE BY AUDIO STREAM
     */
    // RingBuffer <SamplePair, 16384> ringBuffer;
    // AudioStream outStream;

private:

    // Current volume (a value of 0 or below silences the audio playback)
    const static i32 maxVolume = 100000;
    i32 volume = maxVolume;

    /* Target volume and delta steps. Whenever an audio sample is written into
     * the buffer, the volume is increased or decreased by volumeDelta to make
     * it reach the target volume eventually. This feature is used to eliminate
     * cracks when the emulator is started or stopped.
     */
    i32 targetVolume = maxVolume;
    i32 volumeDelta = 0;


    //
    // Initializing
    //
    
public:
    
    PaulaAudio(Amiga& ref);
    
    void _reset(bool hard) override;
    
    
    //
    // Configuring
    //
    
public:
    
    AudioConfig getConfig() { return config; }

    long getConfigItem(ConfigOption option);
    bool setConfigItem(ConfigOption option, long value) override;
    
    double getSampleRate() { return config.sampleRate; }
    void setSampleRate(double hz);
        
    bool isMuted() { return config.volL == 0 && config.volR == 0; }

        
    //
    // Analyzing
    //
    
public:
    
    // Returns the result of the most recent call to inspect()
    AudioInfo getInfo() { return HardwareComponent::getInfo(info); }

    // Returns information about the current workload
    AudioStats getStats() { return stats; }

private:
    
    void _inspect() override;
    void _dump() override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker
        
        & config.samplingMethod
        & config.filterType
        & config.filterAlwaysOn
        & config.vol
        & config.pan
        & config.volL
        & config.volR;
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
    size_t didLoadFromBuffer(u8 *buffer) override;
    
    
    //
    // Controlling
    //
    
private:
    
    void _run() override;
    void _pause() override;


    //
    // Accessing registers
    //
    
public:
    
    void pokeAUDxPER(int nr, u16 value);
    void pokeAUDxVOL(int nr, u16 value);


    //
    // Controlling the volume
    //
    
public:
    
    // Sets the current volume
    void setVolume(i32 vol) { volume = vol; }
    
    /* Triggers volume ramp up phase. Configures volume and targetVolume to
     * simulate a smooth audio fade in
     */
    void rampUp();
    void rampUpFromZero();
    
    /* Triggers volume ramp down phase. Configures volume and targetVolume to
     * simulate a quick audio fade out
     */
    void rampDown();
    
    
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
    template <SamplingMethod method> void executeUntil(Cycle targetClock);

    // Returns the current state of the state machine
    template <int channel> u8 getState();
};

#endif
