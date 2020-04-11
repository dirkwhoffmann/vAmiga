// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

AudioUnit::AudioUnit(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("AudioUnit");
    
    subComponents = vector<HardwareComponent *> {

        &channel0,
        &channel1,
        &channel2,
        &channel3,
        &filterL,
        &filterR
    };
}

void
AudioUnit::setSampleRate(double hz)
{
    debug(AUD_DEBUG, "setSampleRate(%f)\n", hz);

    config.sampleRate = hz;
    cyclesPerSample = MHz(masterClockFrequency) / hz;

    filterL.setSampleRate(hz);
    filterR.setSampleRate(hz);
}

void
AudioUnit::setSamplingMethod(SamplingMethod  method)
{
    debug(AUD_DEBUG, "setSamplingMethod(%d)\n", method);
    assert(isSamplingMethod(method));

    config.samplingMethod = method;
}

void
AudioUnit::setFilterActivation(FilterActivation activation)
{
    debug(AUD_DEBUG, "setFilterActivation(%d)\n", activation);
    assert(isFilterActivation(activation));

    config.filterActivation = activation;
}

FilterType
AudioUnit::getFilterType()
{
    assert(filterL.getFilterType() == config.filterType);
    assert(filterR.getFilterType() == config.filterType);

    return config.filterType;
}

void
AudioUnit::setFilterType(FilterType type)
{
    debug(AUD_DEBUG, "setFilterType(%d)\n", type);
    assert(isFilterType(type));

    config.filterType = type;
    filterL.setFilterType(type);
    filterR.setFilterType(type);
}

void
AudioUnit::_powerOn()
{
}

void
AudioUnit::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);

    info.channel[0] = channel0.getInfo();
    info.channel[1] = channel1.getInfo();
    info.channel[2] = channel2.getInfo();
    info.channel[3] = channel3.getInfo();

    pthread_mutex_unlock(&lock);
}

void
AudioUnit::_dump()
{
}

size_t
AudioUnit::didLoadFromBuffer(u8 *buffer)
{
    clearRingbuffer();
    return 0;
}

void
AudioUnit::_run()
{
    clearRingbuffer();
}

void
AudioUnit::_pause()
{
    clearRingbuffer();
}

void
AudioUnit::_reset()
{
   RESET_SNAPSHOT_ITEMS

    clearRingbuffer();

    stats.bufferUnderflows = 0;
    stats.bufferOverflows = 0;

    volume = 100000;
    targetVolume = 100000;
}

void
AudioUnit::executeUntil(Cycle targetClock)
{
    switch (config.samplingMethod) {
        case SMP_NONE:    executeUntil<SMP_NONE>   (targetClock); return;
        case SMP_NEAREST: executeUntil<SMP_NEAREST>(targetClock); return;
        case SMP_LINEAR:  executeUntil<SMP_LINEAR> (targetClock); return;
    }
}

template <SamplingMethod method> void
AudioUnit::executeUntil(Cycle targetClock)
{
    while (clock < targetClock) {

        short left1  = channel0.interpolate<method>(clock);
        short right1 = channel1.interpolate<method>(clock);
        short right2 = channel2.interpolate<method>(clock);
        short left2  = channel3.interpolate<method>(clock);

        writeData(left1 + left2, right1 + right2);

        clock += cyclesPerSample;
    }
}

AudioInfo
AudioUnit::getInfo()
{
    AudioInfo result;

    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);

    return result;
}

void
AudioUnit::pokeAUDxPER(int nr, u16 value)
{
    switch (nr) {
        case 0: channel0.pokeAUDxPER(value); return;
        case 1: channel1.pokeAUDxPER(value); return;
        case 2: channel2.pokeAUDxPER(value); return;
        case 3: channel3.pokeAUDxPER(value); return;
    }
    assert(false);
}

void
AudioUnit::pokeAUDxVOL(int nr, u16 value)
{
    switch (nr) {
         case 0: channel0.pokeAUDxVOL(value); return;
         case 1: channel1.pokeAUDxVOL(value); return;
         case 2: channel2.pokeAUDxVOL(value); return;
         case 3: channel3.pokeAUDxVOL(value); return;
     }
     assert(false);
}

void
AudioUnit::rampUp()
{
    // Only proceed if the emulator is not running in warp mode
    if (warp) return;
    
    targetVolume = maxVolume;
    volumeDelta = 3;
    ignoreNextUnderOrOverflow();
}

void
AudioUnit::rampUpFromZero()
{
    volume = 0;
    rampUp();
}
 
void
AudioUnit::rampDown()
{    
    targetVolume = 0;
    volumeDelta = 50;
    ignoreNextUnderOrOverflow();
}

void
AudioUnit::clearRingbuffer()
{
    debug(AUDBUF_DEBUG, "Clearing ringbuffer\n");
    
    // Wipe out the ringbuffers
    for (unsigned i = 0; i < bufferSize; i++) {
        ringBufferL[i] = ringBufferR[i] = 0.0;
    }

    // Wipe out the filter buffers
    filterL.clear();
    filterR.clear();

    // Put the write pointer ahead of the read pointer
    alignWritePtr();
}

void
AudioUnit::readMonoSample(float *mono)
{
    float left, right;

    readStereoSample(&left, &right);
    *mono = left + right;
}

void
AudioUnit::readStereoSample(float *left, float *right)
{
    // Read sound samples
    float l = ringBufferL[readPtr];
    float r = ringBufferR[readPtr];

    // Modify volume
    if (volume != targetVolume) {
        if (volume < targetVolume) {
            volume += MIN(volumeDelta, targetVolume - volume);
        } else {
            volume -= MIN(volumeDelta, volume - targetVolume);
        }
    }

    // Apply volume
    float divider = 10000.0f;
    if (volume > 0) {
        l *= (float)volume / divider;
        r *= (float)volume / divider;
    } else {
        l = 0.0;
        r = 0.0;
    }

    // Advance read pointer
    advanceReadPtr();

    // Write result
    *left = l;
    *right = r;
}

float
AudioUnit::ringbufferDataL(size_t offset)
{
    return ringBufferL[(readPtr + offset) % bufferSize];
}

float
AudioUnit::ringbufferDataR(size_t offset)
{
    return ringBufferR[(readPtr + offset) % bufferSize];
}

float
AudioUnit::ringbufferData(size_t offset)
{
    return ringbufferDataL(offset) + ringbufferDataR(offset);
}

void
AudioUnit::readMonoSamples(float *target, size_t n)
{
    // Check for a buffer underflow
    if (samplesInBuffer() < n) {
        handleBufferUnderflow();
    }
    
    // Read sound samples
    for (size_t i = 0; i < n; i++) {
        readMonoSample(target + i);
    }
}

void
AudioUnit::readStereoSamples(float *target1, float *target2, size_t n)
{
    // Check for a buffer underflow
    if (samplesInBuffer() < n)
        handleBufferUnderflow();
    
    // Read sound samples
    for (unsigned i = 0; i < n; i++)
        readStereoSample(target1 + i, target2 + i);
}

void
AudioUnit::readStereoSamplesInterleaved(float *target, size_t n)
{
    // Check for a buffer underflow
    if (samplesInBuffer() < n)
        handleBufferUnderflow();
    
    // Read sound samples
    for (unsigned i = 0; i < n; i++)
        readStereoSample(target + 2*i, target + 2*i + 1);
}

void
AudioUnit::writeData(short left, short right)
{
    // Check for buffer overflow
    if (bufferCapacity() == 0) handleBufferOverflow();

    // Convert samples to floating point values
    float fl = float(left) * scale;
    float fr = float(right) * scale;

    // Apply audio filter if applicable
    if ((config.filterActivation == FILTACT_POWER_LED && ciaa.powerLED()) ||
        (config.filterActivation == FILTACT_ALWAYS)) {
        fl = filterL.apply(fl);
        fr = filterR.apply(fr);
    }

    // Write samples into ringbuffer
    ringBufferL[writePtr] = fl;
    ringBufferR[writePtr] = fr;
    advanceWritePtr();
}

void
AudioUnit::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer.
    // (2) The producer is halted or not startet yet.
    
    debug(AUDBUF_DEBUG, "SID RINGBUFFER UNDERFLOW (r: %ld w: %ld)\n", readPtr, writePtr);
    
    // Determine the elapsed seconds since the last pointer adjustment.
    u64 now = mach_absolute_time();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    
    // Adjust the sample rate, if condition (1) holds.
    if (elapsedTime > 10.0) {

        stats.bufferUnderflows++;
        
        // Increase the sample rate based on what we've measured.
        int offPerSecond = (int)(samplesAhead / elapsedTime);
        setSampleRate(getSampleRate() + offPerSecond);
    }
    
    // Reset the write pointer
    alignWritePtr();
}

void
AudioUnit::handleBufferOverflow()
{
    // There are two common scenarios in which buffer overflows occur:
    //
    // (1) The consumer runs slightly slower than the producer.
    // (2) The consumer is halted or not startet yet.
    
    debug(AUDBUF_DEBUG, "SID RINGBUFFER OVERFLOW (r: %ld w: %ld)\n", readPtr, writePtr);
    
    // Determine the elapsed seconds since the last pointer adjustment.
    u64 now = mach_absolute_time();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    
    // Adjust the sample rate, if condition (1) holds.
    if (elapsedTime > 10.0) {
        
        stats.bufferOverflows++;
        
        // Decrease the sample rate based on what we've measured.
        int offPerSecond = (int)(samplesAhead / elapsedTime);
        setSampleRate(getSampleRate() - offPerSecond);
    }
    
    // Reset the write pointer
    alignWritePtr();
}

float
AudioUnit::drawWaveform(unsigned *buffer, int width, int height,
                        bool left, float highestAmplitude, unsigned color)
{
    int dw = bufferSize / width;
    float newHighestAmplitude = 0.001;
    float *ringBuffer = left ? ringBufferL : ringBufferR;
    
    // Clear buffer
    for (int i = 0; i < width * height; i++) {
        buffer[i] = color & 0xFFFFFF;
    }
    
    // Draw waveform
    for (int w = 0; w < width; w++) {
        
        // Read samples from ringbuffer
        float sample = abs(ringBuffer[w * dw]);
        
        // Remember the highest amplitude
        if (sample > newHighestAmplitude) newHighestAmplitude = sample;
        
        // Scale the sample
        int scaled = sample * height / highestAmplitude;
        if (scaled > height) scaled = height;
        
        scaled = 0;
        if (scaled == 0) {

            // Draw some noise to make it look sexy
            unsigned *ptr = buffer + width * height / 2 + w;
            *ptr = color;
            if (rand() % 2) *(ptr + width) = color;
            if (rand() % 2) *(ptr - width) = color;

        } else {
            
            // Draw vertical line
            unsigned *ptr = buffer + width * ((height - scaled) / 2) + w;
            for (int j = 0; j < scaled; j++, ptr += width) *ptr = color;
        }
    }
    return newHighestAmplitude;
}

template<> u8 AudioUnit::getState<0>() { return channel0.state; }
template<> u8 AudioUnit::getState<1>() { return channel1.state; }
template<> u8 AudioUnit::getState<2>() { return channel2.state; }
template<> u8 AudioUnit::getState<3>() { return channel3.state; }

