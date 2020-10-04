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

    // Audio filters
    AudioFilter filterL = AudioFilter(amiga);
    AudioFilter filterR = AudioFilter(amiga);

    
    //
    // Properties
    //
    
private:

    // The component has been executed up to this clock cycle.
    double clock = 0;

    // Master clock cycles per audio sample, set in setSampleRate()
    double cyclesPerSample = 0;

    // Time stamp of the last write pointer alignment
    Cycle lastAlignment = 0;

    
    //
    // Sample buffers
    //

private:

    // Number of sound samples stored in ringbuffer
    static constexpr size_t bufferSize = 16384;
    
    /* The audio sample ringbuffer. This ringbuffer is used to transfer sound
     * samples from the emulator to the sound device of the host machine.
     */
    /*
    float ringBufferL[bufferSize];
    float ringBufferR[bufferSize];
    */
    SamplePair ringBuffer[bufferSize];
    
    // Read and write pointer of the ringbuffer
    u32 readPtr = 0;
    u32 writePtr = 0;
    
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
     * simulate a smooth audio fade in.
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
            
    // Clears the ringbuffer and resets the read and write pointer
    void clearRingbuffer();

    // Reads sound samples from the sample buffers
    void readMonoSample(float *mono);
    void readStereoSample(float *left, float *right);
    
    // Reads samples from the ringbuffer (mono stream format)
    void readMonoSamples(float *target, size_t n);
    
    // Reads samples from the ringbuffer (stereo stream format)
    void readStereoSamples(float *target1, float *target2, size_t n);
    
    // Reads samples from the ringbuffer (interleaved stereo stream format)
    void readStereoSamplesInterleaved(float *target, size_t n);
    
    // Writes a stereo sample into the ringbuffer
    void writeData(float left, float right);
    
    /* Handles a buffer underflow condition. A buffer underflow occurs if the
     * audio device of the host needs more sound samples than Paula has
     * produced yet.
     */
    void handleBufferUnderflow();
    
    /* Handles a buffer overflow condition. A buffer overflow occurs if Paula
     * has produced more samples than the audio device of the host is able to
     * consume.
     */
    void handleBufferOverflow();
    
    // Signals to ignore the next underflow or overflow condition
    void ignoreNextUnderOrOverflow() { lastAlignment = mach_absolute_time(); }
    
    // Moves the read pointer forward
    void advanceReadPtr() { readPtr = (readPtr + 1) % bufferSize; }
    void advanceReadPtr(int steps) {
        readPtr = (readPtr + bufferSize + steps) % bufferSize; }
    
    // Moves the write pointer forward
    void advanceWritePtr() { writePtr = (writePtr + 1) % bufferSize; }
    void advanceWritePtr(int steps) {
        writePtr = (writePtr + bufferSize + steps) % bufferSize; }
    
    // Returns number of stored samples in the ringbuffer
    unsigned samplesInBuffer() {
        return (writePtr + bufferSize - readPtr) % bufferSize; }
    
    // Returns the remaining storage capacity of the ringbuffer
    unsigned bufferCapacity() {
        return (readPtr + bufferSize - writePtr) % bufferSize; }
    
    // Returns the fill level as a percentage value
    double fillLevel() {
        return (double)samplesInBuffer() / (double)bufferSize; }
    
    /* Aligns the write pointer. This function puts the write pointer somewhat
     * ahead of the read pointer. With a standard sample rate of 44100 Hz,
     * 735 samples is 1/60 sec.
     */
    const u32 samplesAhead = 8 * 735;
    void alignWritePtr() { writePtr = (readPtr  + samplesAhead) % bufferSize; }

    /* Plots a graphical representation of the waveform. Returns the highest
     * amplitute that was found in the ringbuffer. To implement auto-scaling,
     * pass the returned value as parameter highestAmplitude in the next call
     * to this function.
     */
    float drawWaveform(unsigned *buffer, int width, int height,
                       bool left, float highestAmplitude, unsigned color);

    
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
