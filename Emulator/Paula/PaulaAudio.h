// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _PAULA_AUDIO_INC
#define _PAULA_AUDIO_INC

#include "AmigaComponent.h"
#include "StateMachine.h"
#include "AudioFilter.h"

class PaulaAudio : public AmigaComponent {

    // Current configuration
    AudioConfig config;

    // Result of the latest inspection
    AudioInfo info;

    // Collected statistical information
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

private:
    
    //
    // Audio ringbuffer
    //
    
    // Number of sound samples stored in ringbuffer
    static constexpr size_t bufferSize = 16384;
    
    /* The audio sample ringbuffer.
     * This ringbuffer serves as the data interface between the emulation code
     * and the audio API (CoreAudio on Mac OS X).
     */
    float ringBufferL[bufferSize];
    float ringBufferR[bufferSize];

    /* Scaling value for sound samples
     * All sound samples produced by reSID are scaled by this value before they
     * are written into the ringBuffer.
     */
    // static constexpr float scale = 0.000005f;
    static constexpr float scale = 0.0000025f;
    
    /* Ring buffer read pointer
     */
    u32 readPtr = 0;
    
    /* Ring buffer write pointer
     */
    u32 writePtr = 0;
    
    /* Current volume
     * A value of 0 or below silences the audio playback.
     */
    i32 volume = 0;
    
    /* Target volume
     * Whenever an audio sample is written, the volume is increased or
     * decreased by volumeDelta to make it reach the target volume eventually.
     * This feature simulates a fading effect.
     */
    i32 targetVolume;
    
    /* Maximum volume
     */
    const static i32 maxVolume = 100000;
    
    /* Volume offset
     * If the current volume does not match the target volume, it is increased
     * or decreased by the specified amount. The increase or decrease takes
     * place whenever an audio sample is generated.
     */
    i32 volumeDelta = 0;


    //
    // Constructing and serializing
    //
    
public:
    
    PaulaAudio(Amiga& ref);
    
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
    void applyToResetItems(T& worker)
    {
        worker

        & clock;
    }


    //
    // Configuring
    //

    AudioConfig getConfig() { return config; }

    double getSampleRate() { return config.sampleRate; }
    void setSampleRate(double hz);

    SamplingMethod getSamplingMethod() { return config.samplingMethod; }
    void setSamplingMethod(SamplingMethod  method);

    FilterType getFilterType();
    void setFilterType(FilterType type);

    bool getFilterAlwaysOn() { return config.filterAlwaysOn; }
    void setFilterAlwaysOn(bool val);

    double getVol(unsigned nr) { assert(nr < 4); return config.vol[nr]; }
    void setVol(unsigned nr, double val);

    double getPan(unsigned nr) { assert(nr < 4); return config.pan[nr]; }
    void setPan(unsigned nr, double val);

    double getVolL() { return config.volL; }
    void setVolL(double val) { config.volL = val; }

    double getVolR() { return config.volR; }
    void setVolR(double val) { config.volR = val; }

    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _run() override;
    void _pause() override;
    void _reset(bool hard) override;
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(u8 *buffer) override;

public:

    // Returns the result of the most recent call to inspect()
    AudioInfo getInfo() { return HardwareComponent::getInfo(info); }

    // Returns statistical information about this device
    AudioStats getStats() { return stats; }


    //
    // Accessing the state machines
    //
    
    void pokeAUDxPER(int nr, u16 value);
    void pokeAUDxVOL(int nr, u16 value);


    //
    // Controlling the volume
    //
    
    /* Sets the current volume
     */
    void setVolume(i32 vol) { volume = vol; }
    
    /* Triggers volume ramp up phase
     * Configures volume and targetVolume to simulate a smooth audio fade in
     */
    void rampUp();
    void rampUpFromZero();
    
    /* Triggers volume ramp down phase
     * Configures volume and targetVolume to simulate a quick audio fade out
     */
    void rampDown();
    
    
    //
    // Managing the ringbuffer
    //
    
    // Returns the size of the ringbuffer
    size_t ringbufferSize() { return bufferSize; }
    
    // Returns the position of the read pointer
    u32 getReadPtr() { return readPtr; }
    
    // Returns the position of the write pointer
    u32 getWritePtr() { return writePtr; }
    
    // Clears the ringbuffer and resets the read and write pointer
    void clearRingbuffer();

    // Reads sound samples from the sample buffers
    void readMonoSample(float *mono);
    void readStereoSample(float *left, float *right);

    // Reads a single audio sample without moving the read pointer
    float ringbufferDataL(size_t offset);
    float ringbufferDataR(size_t offset);
    float ringbufferData(size_t offset);
    
    /* Reads a certain amount of samples from ringbuffer
     * Samples are stored in a single mono stream
     */
    void readMonoSamples(float *target, size_t n);
    
    /* Reads a certain amount of samples from ringbuffer
     * Samples are stored in two seperate mono streams
     */
    void readStereoSamples(float *target1, float *target2, size_t n);
    
    /* Reads a certain amount of samples from ringbuffer
     * Samples are stored in an interleaved stereo stream
     */
    void readStereoSamplesInterleaved(float *target, size_t n);
    
    /* Writes a stereo sample into the ringbuffer
     */
    void writeData(float left, float right);
    
    /* Handles a buffer underflow condition.
     * A buffer underflow occurs when the computer's audio device needs sound
     * samples than SID hasn't produced, yet.
     */
    void handleBufferUnderflow();
    
    /* Handles a buffer overflow condition
     * A buffer overflow occurs when SID is producing more samples than the
     * computer's audio device is able to consume.
     */
    void handleBufferOverflow();
    
    // Signals to ignore the next underflow or overflow condition.
    void ignoreNextUnderOrOverflow() { lastAlignment = mach_absolute_time(); }
    
    // Moves the read pointer forward
    void advanceReadPtr() { readPtr = (readPtr + 1) % bufferSize; }
    void advanceReadPtr(int steps) { readPtr = (readPtr + bufferSize + steps) % bufferSize; }
    
    // Moves the write pointer forward.
    void advanceWritePtr() { writePtr = (writePtr + 1) % bufferSize; }
    void advanceWritePtr(int steps) { writePtr = (writePtr + bufferSize + steps) % bufferSize; }
    
    // Returns number of stored samples in the ringbuffer.
    unsigned samplesInBuffer() { return (writePtr + bufferSize - readPtr) % bufferSize; }
    
    // Returns the remaining storage capacity of the ringbuffer.
    unsigned bufferCapacity() { return (readPtr + bufferSize - writePtr) % bufferSize; }
    
    // Returns the fill level as a percentage value.
    double fillLevel() { return (double)samplesInBuffer() / (double)bufferSize; }
    
    /* Aligns the write pointer.
     * This function puts the write pointer somewhat ahead of the read pointer.
     * With a standard sample rate of 44100 Hz, 735 samples is 1/60 sec.
     */
    const u32 samplesAhead = 8 * 735;
    void alignWritePtr() { writePtr = (readPtr  + samplesAhead) % bufferSize; }

    /* Plots a graphical representation of the waveform.
     * Returns the highest amplitute that was found in the ringbuffer. To
     * implement auto-scaling, pass the returned value as parameter
     * highestAmplitude in the next call to this function.
     */
    float drawWaveform(unsigned *buffer, int width, int height,
                       bool left, float highestAmplitude, unsigned color);

    //
    // Accessing the state machines
    //

    template <int channel> u8 getState();

    
    //
    // Running the device
    //

public:
    
    // Executes the device until the given master clock cycle has been reached.
    void executeUntil(Cycle targetClock);
    template <SamplingMethod method> void executeUntil(Cycle targetClock);
};

#endif
