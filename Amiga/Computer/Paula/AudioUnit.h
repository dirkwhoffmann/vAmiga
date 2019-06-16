// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AUDIO_UNIT_INC
#define _AUDIO_UNIT_INC

#include "StateMachine.h"
#include "AudioFilter.h"

class AudioUnit : public HardwareComponent {

    //
    // Configuration
    //

    // The currently set filter activation method
    FilterActivation filterActivation;

    
    //
    // Bookkeeping
    //

    // Information shown in the GUI inspector panel
    AudioInfo info;


    // Sub components
    //
    
public:

    // State machines
    StateMachine channel[4];

    // Audio filters
    AudioFilter filterL;
    AudioFilter filterR;

    
    //
    // Properties
    //
    
private:

    // The component has been executed up to this clock cycle.
    Cycle clock = 0;

    // The sample rate in Hz
    double sampleRate;

    // Time stamp of the last write pointer alignment
    Cycle lastAlignment = 0;
    
public:
    
    // Number of buffer underflows since power up
    int64_t bufferUnderflows;
    
    // Number of buffer overflows since power up
    int64_t bufferOverflows;
    
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
    uint32_t readPtr = 0;
    
    /* Ring buffer write pointer
     */
    uint32_t writePtr = 0;
    
    /* Current volume
     * A value of 0 or below silences the audio playback.
     */
    int32_t volume = 0;
    
    /* Target volume
     * Whenever an audio sample is written, the volume is increased or
     * decreased by volumeDelta to make it reach the target volume eventually.
     * This feature simulates a fading effect.
     */
    int32_t targetVolume;
    
    /* Maximum volume
     */
    const static int32_t maxVolume = 100000;
    
    /* Volume offset
     * If the current volume does not match the target volume, it is increased
     * or decreased by the specified amount. The increase or decrease takes
     * place whenever an audio sample is generated.
     */
    int32_t volumeDelta = 0;
    
    
    //
    // State machine
    //
    
private:

    // Indicates the enabled sound DMA channels (Bit n = channel n)
    uint8_t dmaEnabled;

    // Used in executeUntil() to compute the number of samples to generate.
    double dmaCycleCounter1 = 0;
    double dmaCycleCounter2 = 0;
    
    //
    // Constructing and destructing
    //
    
public:
    
    AudioUnit();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _run() override;
    void _pause() override;
    void _inspect() override;
    void _dump() override;
    
    // void setClockFrequency(uint32_t frequency);
    void didLoadFromBuffer(uint8_t **buffer) override { clearRingbuffer(); }


    //
    // Accessing properties
    //

public:

    // Returns the latest internal state recorded by inspect()
    AudioInfo getInfo();

    
    //
    // Configuring the device
    //
    
public:
    
    // Returns the current sample rate in Hz
    double getSampleRate();
    
    // Sets the sample rate in Hz
    void setSampleRate(double hz);

    // Returns the filter activation method
    FilterActivation getFilterActivation() { return filterActivation; }

    // Sets the filter activation method
    void setFilterActivation(FilterActivation activation);

    // Returns the filter type
    FilterType getFilterType();

    // Sets the filter type
    void setFilterType(FilterType type);


    //
    // Controlling the volume
    //
    
    /* Sets the current volume
     */
    void setVolume(int32_t vol) { volume = vol; }
    
    /* Triggers volume ramp up phase
     * Configures volume and targetVolume to simulate a smooth audio fade in
     */
    void rampUp() { targetVolume = maxVolume; volumeDelta = 3; ignoreNextUnderOrOverflow(); }
    void rampUpFromZero() { volume = 0; rampUp(); }
    
    /* Triggers volume ramp down phase
     * Configures volume and targetVolume to simulate a quick audio fade out
     */
    void rampDown() { targetVolume = 0; volumeDelta = 50; ignoreNextUnderOrOverflow(); }
    
    
    //
    // Managing the ringbuffer
    //
    
    // Returns the size of the ringbuffer
    size_t ringbufferSize() { return bufferSize; }
    
    // Returns the position of the read pointer
    uint32_t getReadPtr() { return readPtr; }
    
    // Returns the position of the write pointer
    uint32_t getWritePtr() { return writePtr; }
    
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
    void writeData(short left, short right);
    
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
    const uint32_t samplesAhead = 8 * 735;
    void alignWritePtr() { writePtr = (readPtr  + samplesAhead) % bufferSize; }


    //
    // Running the device
    //

public:

    // Starts or ends DMA for a certain audio channel (called by pokeDMACON)
    void enableDMA(int nr);
    void disableDMA(int nr);
    
    // Executes the device until the given master clock cycle has been reached.
    void executeUntil(Cycle targetClock);

    
    //
    // Accessing registers
    //
    
public:

    // OCS registers 0xA4, 0xB4, 0x0C4, 0xD4 (w)
    void pokeAUDxLEN(int x, uint16_t value) { channel[x].pokeAUDxLEN(value); }
    
    // OCS registers 0xA6, 0xB6, 0x0B6, 0xD6 (w)
    void pokeAUDxPER(int x, uint16_t value) { channel[x].pokeAUDxPER(value); }
    
    // OCS registers 0xA8, 0xB8, 0x0C8, 0xD8 (w)
    void pokeAUDxVOL(int x, uint16_t value) { channel[x].pokeAUDxVOL(value); }
    
    // OCS registers 0xAA, 0xBA, 0x0CA, 0xDA (w)
    void pokeAUDxDAT(int x, uint16_t value) { channel[x].pokeAUDxDAT(value); }
    
};

#endif
