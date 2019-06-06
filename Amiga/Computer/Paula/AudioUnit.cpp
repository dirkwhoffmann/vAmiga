// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

AudioUnit::AudioUnit()
{
    setDescription("AudioUnit");
    
    // Register subcomponents
    registerSubcomponents(vector<HardwareComponent *> {

        &channel[0],
        &channel[1],
        &channel[2],
        &channel[3],
        &filterL,
        &filterR
    });

    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,           sizeof(clock),           0 },
        { &dmaEnabled,      sizeof(dmaEnabled),      0 },
    });

    for (unsigned i = 0; i < 4; i++) channel[i].setNr(i);
}

void
AudioUnit::_powerOn()
{
    clearRingbuffer();

    bufferUnderflows = 0;
    bufferOverflows = 0;

    volume = 100000;
    targetVolume = 100000;
}

void
AudioUnit::_dump()
{
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
AudioUnit::enableDMA(int nr)
{
    channel[nr].setState(0b000);
    SET_BIT(dmaEnabled, nr);
}

void
AudioUnit::disableDMA(int nr)
{
    channel[nr].setState(0b000);
    CLR_BIT(dmaEnabled, nr);
}

void
AudioUnit::executeUntil(Cycle targetClock)
{
    double dmaCyclesPerSample = MHz(dmaClockFrequency) / sampleRate;

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
                left += channel[0].execute(toExecute);
            }

            // Channel 1 (right)
            if (GET_BIT(dmaEnabled, 1)) {
                right += channel[1].execute(toExecute);
            }

            // Channel 2 (right)
            if (GET_BIT(dmaEnabled, 2)) {
                right += channel[2].execute(toExecute);
            }

            // Channel 3 (left)
            if (GET_BIT(dmaEnabled, 3)) {
                left += channel[3].execute(toExecute);
            }
        }

        // Write sound samples into buffers
        writeData(left, right);
    }
}

double
AudioUnit::getSampleRate()
{
    return sampleRate;
}

void
AudioUnit::setSampleRate(double hz)
{
    debug("Setting sample rate to %f\n", hz);

    sampleRate = hz;
    filterL.setSampleRate(hz);
    filterR.setSampleRate(hz);
}

FilterType
AudioUnit::getFilterType()
{
    assert(filterL.getFilterType() == filterR.getFilterType());
    return filterL.getFilterType();
}

void
AudioUnit::setFilterType(FilterType type)
{
    filterL.setFilterType(type);
    filterR.setFilterType(type);
}

void
AudioUnit::clearRingbuffer()
{
    debug(4,"Clearing ringbuffer\n");
    
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
    float divider = 40000.0f;
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
    if (bufferCapacity() == 0)
        handleBufferOverflow();
/*
    // Convert samples to float values and write them into the ringbuffer
    ringBufferL[writePtr] = float(left) * scale; // filterL.apply(float(left) * scale);
    ringBufferR[writePtr] = float(right) * scale; // filterR.apply(float(right) * scale);
*/
    ringBufferL[writePtr] = filterL.apply(float(left) * scale);
    ringBufferR[writePtr] = filterR.apply(float(right) * scale);

    advanceWritePtr();
}

void
AudioUnit::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer.
    // (2) The producer is halted or not startet yet.
    
    debug(2, "SID RINGBUFFER UNDERFLOW (r: %ld w: %ld)\n", readPtr, writePtr);
    
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
    
    debug(2, "SID RINGBUFFER OVERFLOW (r: %ld w: %ld)\n", readPtr, writePtr);
    
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

void
AudioUnit::pokeAUDxLEN(int x, uint16_t value)
{
    debug(AUD_DEBUG, "pokeAUD%dLEN(%X)\n", x, value);
    assert(x < 4);

    channel[x].audlenLatch = value;
}

void
AudioUnit::pokeAUDxPER(int x, uint16_t value)
{
    debug(AUD_DEBUG, "pokeAUD%dPER(%X)\n", x, value);
    assert(x < 4);

    channel[x].audperLatch = value;
}

void
AudioUnit::pokeAUDxVOL(int x, uint16_t value)
{
    debug(AUD_DEBUG, "pokeAUD%dVOL(%X)\n", x, value);
    assert(x < 4);
    
    /* Behaviour: 1. Only the lowest 7 bits are evaluated.
     *            2. All values greater than 64 are treated as 64 (max volume).
     */
    channel[x].audvolLatch = MIN(value & 0x7F, 64);
}

void
AudioUnit::pokeAUDxDAT(int x, uint16_t value)
{
    debug(AUD_DEBUG, "pokeAUD%dDAT(%X)\n", x, value);
    assert(x < 4);

    channel[x].auddatLatch = value;
}
