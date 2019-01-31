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
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &cycles,          sizeof(cycles),         0 }});
}

void
AudioUnit::_powerOn()
{
    clearRingbuffer();
  
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
AudioUnit::executeUntil(uint64_t targetCycle)
{
    uint64_t missingCycles = targetCycle - cycles;
    
    if (missingCycles > 1000000 /* some decent number here */) {
        msg("Far too many Audio cycles are missing.\n");
        missingCycles = 1000000;
    }
    
    execute(missingCycles);
    cycles = targetCycle;
}

void
AudioUnit::execute(uint64_t numCycles)
{
    // debug("Execute audio for %lld cycles (%d samples in buffer)\n", numCycles, samplesInBuffer());
    if (numCycles == 0)
        return;
}

uint32_t
AudioUnit::getSampleRate()
{
    return 44100;
}

void
AudioUnit::setSampleRate(double rate)
{
}

void
AudioUnit::clearRingbuffer()
{
    debug(4,"Clearing ringbuffer\n");
    
    // Reset ringbuffer contents
    for (unsigned i = 0; i < bufferSize; i++) {
        ringBuffer[i] = 0.0f;
    }
    
    // Put the write pointer ahead of the read pointer
    alignWritePtr();
}

float
AudioUnit::readData()
{
    // Read sound sample
    float value = ringBuffer[readPtr];
    
    // Adjust volume
    if (volume != targetVolume) {
        if (volume < targetVolume) {
            volume += MIN(volumeDelta, targetVolume - volume);
        } else {
            volume -= MIN(volumeDelta, volume - targetVolume);
        }
    }
    // float divider = 75000.0f; // useReSID ? 100000.0f : 150000.0f;
    float divider = 40000.0f;
    value = (volume <= 0) ? 0.0f : value * (float)volume / divider;
    
    // Advance read pointer
    advanceReadPtr();
    
    return value;
}

float
AudioUnit::ringbufferData(size_t offset)
{
    return ringBuffer[(readPtr + offset) % bufferSize];
}

void
AudioUnit::readMonoSamples(float *target, size_t n)
{
    // Check for buffer underflow
    if (samplesInBuffer() < n) {
        handleBufferUnderflow();
    }
    
    // Read samples
    for (size_t i = 0; i < n; i++) {
        float value = readData();
        target[i] = value;
    }
}

void
AudioUnit::readStereoSamples(float *target1, float *target2, size_t n)
{
    // debug("read: %d write: %d Reading %d\n", readPtr, writePtr, n);
    
    // Check for buffer underflow
    if (samplesInBuffer() < n) {
        handleBufferUnderflow();
    }
    
    // Read samples
    for (unsigned i = 0; i < n; i++) {
        float value = readData();
        target1[i] = target2[i] = value;
    }
}

void
AudioUnit::readStereoSamplesInterleaved(float *target, size_t n)
{
    // Check for buffer underflow
    if (samplesInBuffer() < n) {
        handleBufferUnderflow();
    }
    
    // Read samples
    for (unsigned i = 0; i < n; i++) {
        float value = readData();
        target[i*2] = value;
        target[i*2+1] = value;
    }
}

void
AudioUnit::writeData(short *data, size_t count)
{
    // debug("  read: %d write: %d Writing %d (%d)\n", readPtr, writePtr, count, bufferCapacity());
    
    // Check for buffer overflow
    if (bufferCapacity() < count) {
        handleBufferOverflow();
    }
    
    // Convert sound samples to floating point values and write into ringbuffer
    for (unsigned i = 0; i < count; i++) {
        ringBuffer[writePtr] = float(data[i]) * scale;
        advanceWritePtr();
    }
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
