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
        
        &filterL,
        &filterR
    });
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        // { &cycles,          sizeof(cycles),          0 },
        
        { &audlen,          sizeof(audlen),          WORD_ARRAY },
        { &audlenInternal,  sizeof(audlenInternal),  WORD_ARRAY },
        { &audper,          sizeof(audper),          WORD_ARRAY },
        { &audperInternal,  sizeof(audperInternal),  WORD_ARRAY },
        { &audvol,          sizeof(audvol),          WORD_ARRAY },
        { &audvolInternal,  sizeof(audvolInternal),  WORD_ARRAY },
        { &auddat,          sizeof(auddat),          WORD_ARRAY },
        { &auddatInternal,  sizeof(auddatInternal),  WORD_ARRAY },
        { &audlcLatch,      sizeof(audlcLatch),      WORD_ARRAY },

        { &dmaEnabled,      sizeof(dmaEnabled),      0 },
        { &currentState,    sizeof(currentState),    WORD_ARRAY },
    });
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
AudioUnit::enableDMA(int channel)
{
    currentState[channel] = 0;
    SET_BIT(dmaEnabled, channel);
}

void
AudioUnit::disableDMA(int channel)
{
    currentState[channel] = 0;
    CLR_BIT(dmaEnabled, channel);
}

void
AudioUnit::hsyncHandler()
{
    double dmaCyclesPerSample = MHz(dmaClockFrequency) / sampleRate;
    double dmaCyclesPerLine = MHz(dmaClockFrequency) / (313.0 * 50.0);

    dmaCycleCounter1 += dmaCyclesPerLine;
    dmaCycleCounter2 += dmaCyclesPerLine;

    while (dmaCycleCounter1 > 0) {

        dmaCycleCounter1 -= dmaCyclesPerSample;
        Cycle toExecute = (Cycle)(dmaCycleCounter2 - dmaCycleCounter1);
        dmaCycleCounter2 -= toExecute;

        // Generate sound samples
        short left = 0;
        short right = 0;

        if (dmaEnabled) {

            // Channel 0 (left)
            if (GET_BIT(dmaEnabled, 0)) {
                executeStateMachine(0, toExecute);
                left += (int8_t)(auddatInternal[0]) * audvol[0];
            }

            // Channel 1 (right)
            if (GET_BIT(dmaEnabled, 1)) {
                executeStateMachine(1, toExecute);
                right += (int8_t)(auddatInternal[1]) * audvol[1];
            }

            // Channel 2 (right)
            if (GET_BIT(dmaEnabled, 2)) {
                executeStateMachine(2, toExecute);
                right += (int8_t)(auddatInternal[2]) * audvol[2];
            }

            // Channel 3 (left)
            if (GET_BIT(dmaEnabled, 3)) {
                executeStateMachine(3, toExecute);
                left += (int8_t)(auddatInternal[3]) * audvol[3];
            }
        }

        // Write sound samples into buffers
        writeData(left, right);
    }
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
AudioUnit::ringbufferData(size_t offset)
{
    return
    ringBufferL[(readPtr + offset) % bufferSize] +
    ringBufferR[(readPtr + offset) % bufferSize];
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
    // debug("read: %d write: %d Reading %d\n", readPtr, writePtr, n);
    
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

    // Convert sound samples to floating point values and write into ringbuffer
    ringBufferL[writePtr] = float(left) * scale; // filterL.apply(float(left) * scale);
    ringBufferR[writePtr] = float(right) * scale; // filterR.apply(float(right) * scale);

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
