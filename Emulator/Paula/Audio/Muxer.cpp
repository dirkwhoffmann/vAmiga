// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Muxer.h"
#include "CIA.h"
#include "IO.h"
#include "MsgQueue.h"
#include <cmath>
#include <algorithm>

Muxer::Muxer(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<AmigaComponent *> {

        &filterL,
        &filterR
    };
    
    setSampleRate(44100);
}

void
Muxer::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
        os << tab("Sampling method");
        os << SamplingMethodEnum::key(config.samplingMethod) << std::endl;
        os << tab("Filter type");
        os << FilterTypeEnum::key(config.filterType) << std::endl;
        os << tab("Filter always on");
        os << bol(config.filterAlwaysOn) << std::endl;
        os << tab("Channel 1 pan");
        os << dec(config.pan[0]) << std::endl;
        os << tab("Channel 2 pan");
        os << dec(config.pan[1]) << std::endl;
        os << tab("Channel 3 pan");
        os << dec(config.pan[2]) << std::endl;
        os << tab("Channel 4 pan");
        os << dec(config.pan[3]) << std::endl;
        os << tab("Channel 1 volume");
        os << dec(config.vol[0]) << std::endl;
        os << tab("Channel 2 volume");
        os << dec(config.vol[1]) << std::endl;
        os << tab("Channel 3 volume");
        os << dec(config.vol[2]) << std::endl;
        os << tab("Channel 4 volume");
        os << dec(config.vol[3]) << std::endl;
        os << tab("Left master volume");
        os << dec(config.volL) << std::endl;
        os << tab("Right master volume");
        os << dec(config.volR) << std::endl;
    }
}

void
Muxer::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    stats = { };
    
    for (isize i = 0; i < 4; i++) sampler[i].reset();
    clear();
}

void
Muxer::clear()
{
    debug(AUDBUF_DEBUG, "clear()\n");
    
    // Wipe out the ringbuffer
    stream.lock();
    stream.wipeOut();
    stream.alignWritePtr();
    stream.unlock();
    
    // Wipe out the filter buffers
    filterL.clear();
    filterR.clear();
}

MuxerConfig
Muxer::getDefaultConfig()
{
    MuxerConfig defaults;

    defaults.samplingMethod = SMP_NONE;
    defaults.filterType = FILTER_BUTTERWORTH;
    defaults.filterAlwaysOn = false;
    defaults.volL = 50;
    defaults.volR = 50;
    defaults.vol[0] = 100;
    defaults.vol[1] = 100;
    defaults.vol[2] = 100;
    defaults.vol[3] = 100;
    defaults.pan[0] = 170;
    defaults.pan[1] = 30;
    defaults.pan[2] = 30;
    defaults.pan[3] = 170;

    return defaults;
}

void
Muxer::resetConfig()
{
    MuxerConfig defaults = getDefaultConfig();
    
    setConfigItem(OPT_SAMPLING_METHOD, defaults.samplingMethod);
    setConfigItem(OPT_FILTER_TYPE, defaults.filterType);
    setConfigItem(OPT_FILTER_ALWAYS_ON, defaults.filterAlwaysOn);
    setConfigItem(OPT_AUDVOLL, defaults.volL);
    setConfigItem(OPT_AUDVOLR, defaults.volR);

    for (isize i = 0; i < 4; i++) {

        setConfigItem(OPT_AUDVOL, i, defaults.vol[i]);
        setConfigItem(OPT_AUDPAN, i, defaults.pan[i]);
    }
}

i64
Muxer::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            return config.samplingMethod;
            
        case OPT_FILTER_TYPE:
            assert(filterL.type == config.filterType);
            assert(filterR.type == config.filterType);
            return config.filterType;
                        
        case OPT_FILTER_ALWAYS_ON:
            return config.filterAlwaysOn;

        case OPT_AUDVOLL:
            return config.volL;

        case OPT_AUDVOLR:
            return config.volR;

        default:
            fatalError;
    }
}

i64
Muxer::getConfigItem(Option option, long id) const
{
    switch (option) {
            
        case OPT_AUDVOL:
            return config.vol[id];

        case OPT_AUDPAN:
            return config.pan[id];
            
        default:
            fatalError;
    }
}

void
Muxer::setConfigItem(Option option, i64 value)
{
    bool wasMuted = isMuted();
    
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            
            if (!SamplingMethodEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, SamplingMethodEnum::keyList());
            }
            
            config.samplingMethod = (SamplingMethod)value;
            return;
            
        case OPT_FILTER_TYPE:
            
            if (!FilterTypeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, FilterTypeEnum::keyList());
            }

            config.filterType = (FilterType)value;
            filterL.type = ((FilterType)value);
            filterR.type = ((FilterType)value);
            return;
                        
        case OPT_FILTER_ALWAYS_ON:
                        
            config.filterAlwaysOn = value;
            return;

        case OPT_AUDVOLL:
            
            config.volL = std::clamp(value, 0LL, 100LL);
            volL = powf((float)value / 50, 1.4f);
                        
            if (wasMuted != isMuted())
                msgQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return;
            
        case OPT_AUDVOLR:

            config.volR = std::clamp(value, 0LL, 100LL);
            volR = powf((float)value / 50, 1.4f);

            if (wasMuted != isMuted())
                msgQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return;
            
        default:
            fatalError;
    }
}

void
Muxer::setConfigItem(Option option, long id, i64 value)
{
    switch (option) {
                        
        case OPT_AUDVOL:
    
            assert(id >= 0 && id <= 3);
                        
            config.vol[id] = std::clamp(value, 0LL, 100LL);
            vol[id] = powf((float)value / 100, 1.4f);
            return;
            
        case OPT_AUDPAN:
                        
            assert(id >= 0 && id <= 3);

            config.pan[id] = value;
            pan[id] = float(0.5 * (sin(config.pan[id] * M_PI / 200.0) + 1));
            return;

        default:
            fatalError;
    }
}

void
Muxer::setSampleRate(double hz)
{
    trace(AUD_DEBUG, "setSampleRate(%f)\n", hz);

    sampleRate = hz;
    cyclesPerSample = MHz(MASTER_FREQUENCY) / hz;

    filterL.setSampleRate(hz);
    filterR.setSampleRate(hz);
}

isize
Muxer::didLoadFromBuffer(const u8 *buffer)
{
    for (isize i = 0; i < 4; i++) sampler[i].reset();
    return 0;
}

void
Muxer::rampUp()
{    
    volume.target = 1.0;
    volume.delta = 3;
    
    ignoreNextUnderOrOverflow();
}

void
Muxer::rampUpFromZero()
{
    volume.current = 0.0;
    
    rampUp();
}
 
void
Muxer::rampDown()
{
    volume.target = 0.0;
    volume.delta = 50;
    
    ignoreNextUnderOrOverflow();
}

void
Muxer::synthesize(Cycle clock, Cycle target, long count)
{
    assert(target > clock);
    assert(count > 0);

    // Determine the number of elapsed cycles per audio sample
    double cyclesPerSample = (double)(target - clock) / (double)count;
                
    switch (config.samplingMethod) {
            
        case SMP_NONE:
            
            synthesize <SMP_NONE> (clock, count, cyclesPerSample);
            break;
            
        case SMP_NEAREST:
            
            synthesize <SMP_NEAREST> (clock, count, cyclesPerSample);
            break;
            
        case SMP_LINEAR:
            
            synthesize<SMP_LINEAR> (clock, count, cyclesPerSample);
            break;
            
        default:
            fatalError;
    }
}

void
Muxer::synthesize(Cycle clock, Cycle target)
{
    assert(target > clock);
    assert(cyclesPerSample > 0);
    
    // Determine how many samples we need to produce
    double exact = (double)(target - clock) / cyclesPerSample + fraction;
    long count = (long)exact;
    fraction = exact - (double)count;
             
    switch (config.samplingMethod) {
        case SMP_NONE:
            
            synthesize <SMP_NONE> (clock, count, cyclesPerSample);
            break;
            
        case SMP_NEAREST:
            
            synthesize <SMP_NEAREST> (clock, count, cyclesPerSample);
            break;
            
        case SMP_LINEAR:
            
            synthesize <SMP_LINEAR> (clock, count, cyclesPerSample);
            break;
            
        default:
            fatalError;

    }
}

template <SamplingMethod method> void
Muxer::synthesize(Cycle clock, long count, double cyclesPerSample)
{
    assert(count > 0);

    stream.lock();
    
    // Check for a buffer overflow
    if (stream.count() + count >= stream.cap()) handleBufferOverflow();

    double cycle = clock;
    bool filter = ciaa.powerLED() || config.filterAlwaysOn;

    for (long i = 0; i < count; i++) {

        float ch0 = sampler[0].interpolate <method> ((Cycle)cycle) * vol[0];
        float ch1 = sampler[1].interpolate <method> ((Cycle)cycle) * vol[1];
        float ch2 = sampler[2].interpolate <method> ((Cycle)cycle) * vol[2];
        float ch3 = sampler[3].interpolate <method> ((Cycle)cycle) * vol[3];
        
        // Compute left channel output
        float l =
        ch0 * (1 - pan[0]) + ch1 * (1 - pan[1]) +
        ch2 * (1 - pan[2]) + ch3 * (1 - pan[3]);

        // Compute right channel output
        float r =
        ch0 * pan[0] + ch1 * pan[1] +
        ch2 * pan[2] + ch3 * pan[3];

        // Apply audio filter
        if (filter) { l = filterL.apply(l); r = filterR.apply(r); }
        
        // Apply master volume
        l *= volL;
        r *= volR;
        
        // Write sample into ringbuffer
        stream.add(l, r);
        stats.producedSamples++;
        
        cycle += cyclesPerSample;
    }
    
    stream.unlock();
}

void
Muxer::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer
    // (2) The producer is halted or not startet yet
    
    debug(AUDBUF_DEBUG, "UNDERFLOW (r: %zd w: %zd)\n", stream.r, stream.w);
    
    // Reset the write pointer
    stream.alignWritePtr();

    // Determine the elapsed seconds since the last pointer adjustment
    auto elapsedTime = util::Time::now() - lastAlignment;
    lastAlignment = util::Time::now();
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime.asSeconds() > 10.0) {

        stats.bufferUnderflows++;
        
        // Increase the sample rate based on what we've measured
        auto offPerSec = (stream.cap() / 2) / elapsedTime.asSeconds();
        setSampleRate(getSampleRate() + (isize)offPerSec);
    }
}

void
Muxer::handleBufferOverflow()
{
    // There are two common scenarios in which buffer overflows occur:
    //
    // (1) The consumer runs slightly slower than the producer
    // (2) The consumer is halted or not startet yet
    
    debug(AUDBUF_DEBUG, "OVERFLOW (r: %zd w: %zd)\n", stream.r, stream.w);
    
    // Reset the write pointer
    stream.alignWritePtr();

    // Determine the number of elapsed seconds since the last adjustment
    auto elapsedTime = util::Time::now() - lastAlignment;
    lastAlignment = util::Time::now();
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime.asSeconds() > 10.0) {
        
        stats.bufferOverflows++;
        
        // Decrease the sample rate based on what we've measured
        auto offPerSec = (stream.cap() / 2) / elapsedTime.asSeconds();
        double newSampleRate = getSampleRate() - (isize)offPerSec;

        debug(AUDBUF_DEBUG, "Changing sample rate to %f\n", newSampleRate);
        setSampleRate(newSampleRate);
    }
}

void
Muxer::ignoreNextUnderOrOverflow()
{
    lastAlignment = util::Time::now();
}

void
Muxer::copy(void *buffer, isize n)
{
    stream.lock();
    
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();
    
    // Copy sound samples
    stream.copy(buffer, n, volume);
    stats.consumedSamples += n;
    
    stream.unlock();
}

void
Muxer::copy(void *buffer1, void *buffer2, isize n)
{
    stream.lock();
    
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();
    
    // Copy sound samples
    stream.copy(buffer1, buffer2, n, volume);
    stats.consumedSamples += n;

    stream.unlock();
}

SampleType *
Muxer::nocopy(isize n)
{
    stream.lock();
    
    if (stream.count() < n) handleBufferUnderflow();
    SampleType *addr = stream.currentAddr();
    stream.skip(n);
    stats.consumedSamples += n;

    stream.unlock();
    return addr;
}
