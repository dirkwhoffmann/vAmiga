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
        
        { &cycles,          sizeof(cycles),          0 },
        
        { &audlen,          sizeof(audlen),          WORD_ARRAY },
        { &audlenInternal,  sizeof(audlenInternal),  WORD_ARRAY },
        { &audper,          sizeof(audper),          WORD_ARRAY },
        { &audperInternal,  sizeof(audperInternal),  WORD_ARRAY },
        { &audvol,          sizeof(audvol),          WORD_ARRAY },
        { &audvolInternal,  sizeof(audvolInternal),  WORD_ARRAY },
        { &auddat,          sizeof(auddat),          WORD_ARRAY },
        { &auddatInternal,  sizeof(auddatInternal),  WORD_ARRAY },
        { &audlcLatch,      sizeof(audlcLatch),      WORD_ARRAY },

        { &dmaEnabled,      sizeof(dmaEnabled),      WORD_ARRAY },
        { &currentState,    sizeof(currentState),    WORD_ARRAY },
    });
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
AudioUnit::enableDMA(int channel)
{
    currentState[channel] = 0;
    dmaEnabled[channel] = true;
}

void
AudioUnit::disableDMA(int channel)
{
    currentState[channel] = 0;
    dmaEnabled[channel] = false;
}

void
AudioUnit::hsyncHandler()
{
    double dmaCyclesPerSample = 50 * (HPOS_MAX + 1) * (VPOS_MAX + 1) / 44100.0;
    int executed = 0;
    short sample;
    
    while (dmaCycleCounter < (HPOS_MAX + 1)) {
        
        sample = 0;
        dmaCycleCounter += dmaCyclesPerSample;
        
        int toExecute = (int)dmaCycleCounter;
        int missing = toExecute - executed;
        
        // debug("Executing state machine: %d %d cycles %f %f\n", toExecute, missing, dmaCycleCounter);
        for (unsigned i = 0; i < 4; i++) {
            
            if (dmaEnabled[i]) {
                executeStateMachine(i, missing);
                sample += (int8_t)(auddatInternal[i]) * audvol[i];
            }
        }
        
        // if (dmaEnabled[0]) printf("%d ", sample);
        writeData(&sample, 1);
        executed += missing;
    }
    
    dmaCycleCounter -= executed;
    // debug("dmaCycleCounter = %f\n", dmaCycleCounter);
}

void
AudioUnit::executeStateMachine(int channel, DMACycle cycles)
{
    // if (dmaEnabled[channel]) return;
    // debug("State machine channel %d: current state = %d\n", channel, currentState[channel]);
    
    switch(currentState[channel]) {
            
        case 0:
            
            debug("state = %d\n", currentState[channel]);
            
            audlenInternal[channel] = audlen[channel];
            _agnus->audlc[channel] = audlcLatch[channel];
            audperInternal[channel] = 0;
            currentState[channel] = 1;
            break;
            
        case 1:
            
            debug("state = %d\n", currentState[channel]);

            if (audlenInternal[channel] > 1) audlenInternal[channel]--;
            
            // Trigger Audio interrupt
            _paula->pokeINTREQ(0x8000 | (0x80 << channel));
            
            currentState[channel] = 5;
            
            break;
            
        case 2:
            
            audperInternal[channel] -= cycles;

            if (audperInternal[channel] < 0) {
                
                audperInternal[channel] += audper[channel];
                audvolInternal[channel] = audvol[channel];
                
                // Put out the high byte
                auddatInternal[channel] = HI_BYTE(auddat[channel]);
                
                // Switch forth to state 3
                currentState[channel] = 3;
            }
            break;
            
        case 3:
            
            audperInternal[channel] -= cycles;
            
            if (audperInternal[channel] < 0) {
                
                audperInternal[channel] += audper[channel];
                audvolInternal[channel] = audvol[channel];
                
                // Put out the low byte
                auddatInternal[channel] = LO_BYTE(auddat[channel]);

                // Read the next two samples from memory
                auddat[channel] = _mem->peekChip16(_agnus->audlc[channel]);
                INC_DMAPTR(_agnus->audlc[channel]);
                
                // Decrease the length counter
                if (audlenInternal[channel] > 1) {
                    audlenInternal[channel]--;
                } else {
                    audlenInternal[channel] = audlen[channel];
                    _agnus->audlc[channel] = audlcLatch[channel];
                    
                    // Trigger Audio interrupt
                    _paula->pokeINTREQ(0x8000 | (0x80 << channel));
                }
                    
                // Switch back to state 2
                currentState[channel] = 2;
                
            }
            break;
            
        case 5:
            
            debug("state = %d\n", currentState[channel]);

            audvolInternal[channel] = audvol[channel];
            audperInternal[channel] = 0;
            
            // Read the next two samples from memory
            auddat[channel] = _mem->peekChip16(_agnus->audlc[channel]);
            INC_DMAPTR(_agnus->audlc[channel]);
            
            if (audlenInternal[channel] > 1) {
                audlenInternal[channel]--;
            } else {
                audlenInternal[channel] = audlen[channel];
                _agnus->audlc[channel] = audlcLatch[channel];
                
                // Trigger Audio interrupt
                _paula->pokeINTREQ(0x8000 | (0x80 << channel));
            }
            
            // Transition to state 2
            currentState[channel] = 2;
            break;

        default:
            assert(false);
            break;
            
    }
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

void
AudioUnit::pokeAUDxLEN(int x, uint16_t value)
{
    debug(2, "pokeAUD%dLEN(%X)\n", x, value);
    assert(x < 4);
    
    audlen[x] = value;
}

void
AudioUnit::pokeAUDxPER(int x, uint16_t value)
{
    debug(2, "pokeAUD%dPER(%X)\n", x, value);
    assert(x < 4);
    
    audper[x] = value;
}

void
AudioUnit::pokeAUDxVOL(int x, uint16_t value)
{
    debug(2, "pokeAUD%dVOL(%X)\n", x, value);
    assert(x < 4);
    
    // Behaviour: 1. Only the lowest 7 bits are evaluated.
    //            2. All values greater than 64 are treated as 64 (max volume).
    audvol[x] = MIN(value & 0x7F, 64);
}

void
AudioUnit::pokeAUDxDAT(int x, uint16_t value)
{
    debug(2, "pokeAUD%dDAT(%X)\n", x, value);
    assert(x < 4);
    
    auddat[x] = value;
}
