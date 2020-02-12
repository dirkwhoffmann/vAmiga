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
    filterL.setSampleRate(hz);
    filterR.setSampleRate(hz);
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
AudioUnit::didLoadFromBuffer(uint8_t *buffer)
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

    bufferUnderflows = 0;
    bufferOverflows = 0;

    volume = 100000;
    targetVolume = 100000;
}

void
AudioUnit::enableDMA(int nr)
{
    // executeUntil(agnus.clock);

    switch (nr) {
        case 0: channel0.setState(0b000); break;
        case 1: channel1.setState(0b000); break;
        case 2: channel2.setState(0b000); break;
        case 3: channel3.setState(0b000); break;
        default: assert(false);
    }
    SET_BIT(dmaEnabled, nr);
}

void
AudioUnit::disableDMA(int nr)
{
    // executeUntil(agnus.clock);

    switch (nr) {
        case 0: channel0.setState(0b000); break;
        case 1: channel1.setState(0b000); break;
        case 2: channel2.setState(0b000); break;
        case 3: channel3.setState(0b000); break;
        default: assert(false);
    }
    CLR_BIT(dmaEnabled, nr);
}

/*
void
AudioUnit::executeUntil(Cycle targetClock)
{
    double dmaCyclesPerSample = MHz(dmaClockFrequency) / config.sampleRate;

    dmaCycleCounter1 += AS_DMA_CYCLES(targetClock - clock);
    dmaCycleCounter2 += AS_DMA_CYCLES(targetClock - clock);
    clock = targetClock;

    while (dmaCycleCounter1 > 0) {

        short left = 0;
        short right = 0;

        // Compute number of DMA cycles in the next sampling interval
        dmaCycleCounter1 -= dmaCyclesPerSample;
        Cycle toExecute = (Cycle)(dmaCycleCounter2 - dmaCycleCounter1);
        dmaCycleCounter2 -= toExecute;

        // Execute the state machines for all four channels
        if (dmaEnabled) {

            // Channel 0 (left)
            if (GET_BIT(dmaEnabled, 0)) {
                left += channel0.execute(toExecute);
            }

            // Channel 1 (right)
            if (GET_BIT(dmaEnabled, 1)) {
                right += channel1.execute(toExecute);
            }

            // Channel 2 (right)
            if (GET_BIT(dmaEnabled, 2)) {
                right += channel2.execute(toExecute);
            }

            // Channel 3 (left)
            if (GET_BIT(dmaEnabled, 3)) {
                left += channel3.execute(toExecute);
            }
        }

        // Write sound samples into buffers
        writeData(left, right);
    }
}
*/

void
AudioUnit::executeUntil(Cycle targetClock)
{
    Cycle dmaCyclesPerSample = MHz(dmaClockFrequency) / config.sampleRate;

    while (clock + dmaCyclesPerSample < targetClock) {

        clock += DMA_CYCLES(dmaCyclesPerSample);

        channel0.execute(dmaCyclesPerSample);
        channel1.execute(dmaCyclesPerSample);
        channel2.execute(dmaCyclesPerSample);
        channel3.execute(dmaCyclesPerSample);

        short left1  = channel0.pickSample(clock);
        short right1 = channel1.pickSample(clock);
        short right2 = channel2.pickSample(clock);
        short left2  = channel3.pickSample(clock);

        writeData(left1 + left2, right1 + right2);
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
AudioUnit::pokeAUDxPER(int nr, uint16_t value)
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
AudioUnit::pokeAUDxVOL(int nr, uint16_t value)
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
    uint64_t now = mach_absolute_time();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    
    // Adjust the sample rate, if condition (1) holds.
    if (elapsedTime > 10.0) {
        
        bufferUnderflows++;
        
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
    uint64_t now = mach_absolute_time();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    
    // Adjust the sample rate, if condition (1) holds.
    if (elapsedTime > 10.0) {
        
        bufferOverflows++;
        
        // Decrease the sample rate based on what we've measured.
        int offPerSecond = (int)(samplesAhead / elapsedTime);
        setSampleRate(getSampleRate() - offPerSecond);
    }
    
    // Reset the write pointer
    alignWritePtr();
}
