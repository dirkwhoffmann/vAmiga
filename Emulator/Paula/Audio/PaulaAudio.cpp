// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

PaulaAudio::PaulaAudio(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("AudioUnit");
    
    subComponents = vector<HardwareComponent *> {

        &channel0,
        &channel1,
        &channel2,
        &channel3,
        &muxer,
        &filterL,
        &filterR
    };
}

void
PaulaAudio::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    clearRingbuffer();
    stats.bufferUnderflows = 0;
    stats.bufferOverflows = 0;
}

long
PaulaAudio::getConfigItem(ConfigOption option)
{
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            return config.samplingMethod;
            
        case OPT_FILTER_TYPE:
            assert(filterL.getFilterType() == config.filterType);
            assert(filterR.getFilterType() == config.filterType);
            return config.filterType;
            
        case OPT_FILTER_ALWAYS_ON:
            return config.filterAlwaysOn;
            
        case OPT_AUDVOLL:
            return (long)(exp2(config.volL) * 100.0);

        case OPT_AUDVOLR:
            return (long)(exp2(config.volR) * 100.0);

        case OPT_AUDVOL0:
            return (long)(exp2(config.vol[0] / 0.0000025) * 100.0);

        case OPT_AUDVOL1:
            return (long)(exp2(config.vol[1] / 0.0000025) * 100.0);
            
        case OPT_AUDVOL2:
            return (long)(exp2(config.vol[2] / 0.0000025) * 100.0);
            
        case OPT_AUDVOL3:
            return (long)(exp2(config.vol[3] / 0.0000025) * 100.0);

        case OPT_AUDPAN0:
            return (long)(config.pan[0] * 100.0);
            
        case OPT_AUDPAN1:
            return (long)(config.pan[1] * 100.0);
            
        case OPT_AUDPAN2:
            return (long)(config.pan[2] * 100.0);
            
        case OPT_AUDPAN3:
            return (long)(config.pan[3] * 100.0);

        default: assert(false);
    }
}

bool
PaulaAudio::setConfigItem(ConfigOption option, long value)
{
    bool wasMuted = isMuted();
    
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            
            if (!isSamplingMethod(value)) {
                warn("Invalid filter activation: %d\n", value);
                return false;
            }
            break;
            
        case OPT_FILTER_TYPE:
            
            if (!isFilterType(value)) {
                warn("Invalid filter type: %d\n", value);
                warn("       Valid values: 0 ... %d\n", FILT_COUNT - 1);
                return false;
            }
            break;
            
        case OPT_AUDVOLL:
        case OPT_AUDVOLR:
        case OPT_AUDVOL0:
        case OPT_AUDVOL1:
        case OPT_AUDVOL2:
        case OPT_AUDVOL3:
            
            if (value < 100 || value > 400) {
                warn("Invalid volumne: %d\n", value);
                warn("       Valid values: 100 ... 400\n");
                return false;
            }
            break;
            
        case OPT_AUDPAN0:
        case OPT_AUDPAN1:
        case OPT_AUDPAN2:
        case OPT_AUDPAN3:
            
            if (value < 0 || value > 100) {
                warn("Invalid pan: %d\n", value);
                warn("       Valid values: 0 ... 100\n");
                return false;
            }
            break;
            
        default:
            break;
    }

    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            
            if (config.samplingMethod == value) {
                return false;
            }
            
            config.samplingMethod = (SamplingMethod)value;
            return true;
            
        case OPT_FILTER_TYPE:
            
            if (config.filterType == value) {
                return false;
            }

            config.filterType = (FilterType)value;
            filterL.setFilterType((FilterType)value);
            filterR.setFilterType((FilterType)value);
            return true;
            
        case OPT_FILTER_ALWAYS_ON:
            
            if (config.filterAlwaysOn == value) {
                return false;
            }
            
            config.filterAlwaysOn = value;
            return true;
            
        case OPT_AUDVOLL:
            
            config.volL = log2((double)value / 100.0);
            if (wasMuted != isMuted())
                messageQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return true;
            
        case OPT_AUDVOLR:            

            config.volR = log2((double)value / 100.0);
            if (wasMuted != isMuted())
                messageQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return true;
            
        case OPT_AUDVOL0:
            
            config.vol[0] = log2((double)value / 100.0) * 0.0000025;
            return true;
            
        case OPT_AUDVOL1:
            
            config.vol[1] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDVOL2:
            
            config.vol[2] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDVOL3:
            
            config.vol[3] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDPAN0:
            
            config.pan[0] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN1:
            config.pan[1] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN2:
            
            config.pan[2] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN3:
            
            config.pan[3] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        default:
            return false;
    }
}

void
PaulaAudio::setSampleRate(double hz)
{
    debug(AUD_DEBUG, "setSampleRate(%f)\n", hz);

    config.sampleRate = hz;
    cyclesPerSample = MHz(masterClockFrequency) / hz;

    filterL.setSampleRate(hz);
    filterR.setSampleRate(hz);
}

size_t
PaulaAudio::didLoadFromBuffer(u8 *buffer)
{
    clearRingbuffer();
    return 0;
}

void
PaulaAudio::_inspect()
{
    synchronized {
        
        info.channel[0] = channel0.getInfo();
        info.channel[1] = channel1.getInfo();
        info.channel[2] = channel2.getInfo();
        info.channel[3] = channel3.getInfo();
    }
}

void
PaulaAudio::_dump()
{
}

void
PaulaAudio::_run()
{
    clearRingbuffer();
}

void
PaulaAudio::_pause()
{
    clearRingbuffer();
}

void
PaulaAudio::executeUntil(Cycle target)
{
    if (target > 0) clock = muxer.synthesize(clock, target);
    /*
    switch (config.samplingMethod) {
        case SMP_NONE:    executeUntil<SMP_NONE>   (target); return;
        case SMP_NEAREST: executeUntil<SMP_NEAREST>(target); return;
        case SMP_LINEAR:  executeUntil<SMP_LINEAR> (target); return;
    }
    */
}

/*

template <SamplingMethod method> void
PaulaAudio::executeUntil(Cycle targetClock)
{
    while (clock < targetClock) {

        double ch0 = muxer.sampler[0].interpolate<method>((Cycle)clock) * config.vol[0];
        double ch1 = muxer.sampler[1].interpolate<method>((Cycle)clock) * config.vol[1];
        double ch2 = muxer.sampler[2].interpolate<method>((Cycle)clock) * config.vol[2];
        double ch3 = muxer.sampler[3].interpolate<method>((Cycle)clock) * config.vol[3];

        double l =
        ch0 * config.pan[0] + ch1 * config.pan[1] +
        ch2 * config.pan[2] + ch3 * config.pan[3];
        
        double r =
        ch0 * (1 - config.pan[0]) + ch1 * (1 - config.pan[1]) +
        ch2 * (1 - config.pan[2]) + ch3 * (1 - config.pan[3]);
        
        writeData((float)(l * config.volL), (float)(r * config.volR));

        clock += cyclesPerSample;
    }
}
 */

void
PaulaAudio::pokeAUDxPER(int nr, u16 value)
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
PaulaAudio::pokeAUDxVOL(int nr, u16 value)
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
PaulaAudio::rampUp()
{
    // Only proceed if the emulator is not running in warp mode
    if (warpMode) return;
    
    targetVolume = maxVolume;
    volumeDelta = 3;
    ignoreNextUnderOrOverflow();
}

void
PaulaAudio::rampUpFromZero()
{
    volume = 0;
    rampUp();
}
 
void
PaulaAudio::rampDown()
{    
    targetVolume = 0;
    volumeDelta = 50;
    ignoreNextUnderOrOverflow();
}

void
PaulaAudio::clearRingbuffer()
{
    debug(AUDBUF_DEBUG, "Clearing ringbuffer\n");
    
    muxer.clear();
    
    // Wipe out the ringbuffer
    outStream.clear(SamplePair {0, 0});
    outStream.align(samplesAhead);
    
    // Wipe out the filter buffers
    filterL.clear();
    filterR.clear();
}

void
PaulaAudio::readMonoSamples(float *target, size_t n)
{
    // Check for a buffer underflow
    if (outStream.count() < n) handleBufferUnderflow();
    
    // Read sound samples
    muxer.stream.copyMono(target, n, volume, targetVolume, volumeDelta);
}

void
PaulaAudio::readStereoSamples(float *target1, float *target2, size_t n)
{
    // Check for a buffer underflow
    if (outStream.count() < n) handleBufferUnderflow();
    
    // Read sound samples
    muxer.stream.copy(target1, target2, n, volume, targetVolume, volumeDelta);
}

void
PaulaAudio::writeData(float left, float right)
{
    assert(false);
    
    // Check for buffer overflow
    if (outStream.isFull()) handleBufferOverflow();
    
    // Apply audio filter if applicable
    if (ciaa.powerLED() || config.filterAlwaysOn) {
        left = filterL.apply(left);
        right = filterR.apply(right);
    }

    // Write sample into ringbuffer
    outStream.write( SamplePair { left, right } );
    
    // Report sample to the screen recorder
    denise.recorder.addSample(left, right);
}

void
PaulaAudio::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer
    // (2) The producer is halted or not startet yet
    
    debug(AUDBUF_DEBUG, "UNDERFLOW (r: %d w: %d)\n", outStream.r, outStream.w);
    
    // Determine the elapsed seconds since the last pointer adjustment
    u64 now = mach_absolute_time();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime > 10.0) {

        stats.bufferUnderflows++;
        
        // Increase the sample rate based on what we've measured
        int offPerSecond = (int)(samplesAhead / elapsedTime);
        setSampleRate(getSampleRate() + offPerSecond);
    }
    
    // Reset the write pointer
    outStream.align(samplesAhead);
}

void
PaulaAudio::handleBufferOverflow()
{
    // There are two common scenarios in which buffer overflows occur:
    //
    // (1) The consumer runs slightly slower than the producer
    // (2) The consumer is halted or not startet yet
    
    debug(AUDBUF_DEBUG, "OVERFLOW (r: %d w: %d)\n", outStream.r, outStream.w);
    
    // Determine the elapsed seconds since the last pointer adjustment
    u64 now = mach_absolute_time();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime > 10.0) {
        
        stats.bufferOverflows++;
        
        // Decrease the sample rate based on what we've measured
        int offPerSecond = (int)(samplesAhead / elapsedTime);
        setSampleRate(getSampleRate() - offPerSecond);
    }
    
    // Reset the write pointer
    outStream.align(samplesAhead);
}

template<> u8 PaulaAudio::getState<0>() { return channel0.state; }
template<> u8 PaulaAudio::getState<1>() { return channel1.state; }
template<> u8 PaulaAudio::getState<2>() { return channel2.state; }
template<> u8 PaulaAudio::getState<3>() { return channel3.state; }
