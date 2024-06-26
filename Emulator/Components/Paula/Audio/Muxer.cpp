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
#include "Emulator.h"
#include "CIA.h"
#include "IOUtils.h"
#include "MsgQueue.h"
#include <cmath>
#include <algorithm>

namespace vamiga {

Muxer::Muxer(Amiga& ref, isize id) : SubComponent(ref, id)
{
    subComponents = std::vector<CoreComponent *> {

        &filter
    };
}

void
Muxer::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        dumpConfig(os);
    }

    if (category == Category::State) {

        paula.channel0.dump(category, os);
        os << std::endl;
        paula.channel1.dump(category, os);
        os << std::endl;
        paula.channel2.dump(category, os);
        os << std::endl;
        paula.channel3.dump(category, os);
        os << std::endl;
        os << tab("Fill level");
        os << fillLevelAsString(stream.fillLevel()) << std::endl;
    }
}

void
Muxer::_initialize()
{
    CoreComponent::_initialize();

    setSampleRate(44100);
}

void
Muxer::_didReset(bool hard)
{
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
    filter.clear();
}

i64
Muxer::getOption(Option option) const
{
    switch (option) {
            
        case OPT_AUD_SAMPLING_METHOD:   return config.samplingMethod;
        case OPT_AUD_PAN0:           return config.pan[0];
        case OPT_AUD_PAN1:           return config.pan[1];
        case OPT_AUD_PAN2:           return config.pan[2];
        case OPT_AUD_PAN3:           return config.pan[3];
        case OPT_AUD_VOL0:           return config.vol[0];
        case OPT_AUD_VOL1:           return config.vol[1];
        case OPT_AUD_VOL2:           return config.vol[2];
        case OPT_AUD_VOL3:           return config.vol[3];
        case OPT_AUD_VOLL:           return config.volL;
        case OPT_AUD_VOLR:           return config.volR;
        case OPT_AUD_FASTPATH:      return config.idleFastPath;
        case OPT_AUD_FILTER_TYPE:       return filter.getOption(option);

        default:
            fatalError;
    }
}

void
Muxer::setOption(Option option, i64 value)
{
    bool wasMuted = isMuted();
    isize id = 0;

    switch (option) {
            
        case OPT_AUD_SAMPLING_METHOD:
            
            if (!SamplingMethodEnum::isValid(value)) {
                throw Error(ERROR_OPT_INV_ARG, SamplingMethodEnum::keyList());
            }
            
            config.samplingMethod = (SamplingMethod)value;
            return;
            
        case OPT_AUD_VOL3: id++;
        case OPT_AUD_VOL2: id++;
        case OPT_AUD_VOL1: id++;
        case OPT_AUD_VOL0:

            config.vol[id] = std::clamp(value, 0LL, 100LL);
            vol[id] = powf((float)value / 100, 1.4f);
            return;

        case OPT_AUD_VOLL:
            
            config.volL = std::clamp(value, 0LL, 100LL);
            volL = powf((float)value / 50, 1.4f);

            if (wasMuted != isMuted())
                msgQueue.put(MSG_MUTE, isMuted());
            return;
            
        case OPT_AUD_PAN3: id++;
        case OPT_AUD_PAN2: id++;
        case OPT_AUD_PAN1: id++;
        case OPT_AUD_PAN0:

            config.pan[id] = value;
            pan[id] = float(0.5 * (sin(config.pan[id] * M_PI / 200.0) + 1));
            return;

        case OPT_AUD_VOLR:

            config.volR = std::clamp(value, 0LL, 100LL);
            volR = powf((float)value / 50, 1.4f);

            if (wasMuted != isMuted())
                msgQueue.put(MSG_MUTE, isMuted());
            return;

        case OPT_AUD_FASTPATH:

            config.idleFastPath = (bool)value;
            return;

        case OPT_AUD_FILTER_TYPE:

            filter.setOption(option, value);
            return;

        default:
            fatalError;
    }
}

void
Muxer::setSampleRate(double hz)
{
    trace(AUD_DEBUG, "setSampleRate(%f)\n", hz);

    filter.setup(hz);
}

void
Muxer::_didLoad()
{
    for (isize i = 0; i < 4; i++) sampler[i].reset();
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
    double cps = (double)(target - clock) / (double)count;

    switch (config.samplingMethod) {
            
        case SMP_NONE:      synthesize<SMP_NONE>(clock, count, cps); break;
        case SMP_NEAREST:   synthesize<SMP_NEAREST>(clock, count, cps); break;
        case SMP_LINEAR:    synthesize<SMP_LINEAR>(clock, count, cps); break;
            
        default:
            fatalError;
    }
}

void
Muxer::synthesize(Cycle clock, Cycle target)
{
    assert(target > clock);

    // Determine the number of elapsed cycles per audio sample
    double cps = double(amiga.masterClockFrequency()) / double(emulator.host.getOption(OPT_HOST_SAMPLE_RATE));

    // Determine how many samples we need to produce
    double exact = (double)(target - clock) / cps + fraction;

    // Extract the integer part and remember the rest
    double count; fraction = std::modf(exact, &count);

    switch (config.samplingMethod) {

        case SMP_NONE:      synthesize<SMP_NONE>(clock, long(count), cps); break;
        case SMP_NEAREST:   synthesize<SMP_NEAREST>(clock, long(count), cps); break;
        case SMP_LINEAR:    synthesize<SMP_LINEAR>(clock, long(count), cps); break;
            
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

    // Check if we need to interpolate samples (audio is playing)
    if (sampler[0].isActive() || sampler[1].isActive() ||
        sampler[2].isActive() || sampler[3].isActive() || !config.idleFastPath) {

        double cycle = (double)clock;
        bool loEnabled = filter.loFilterEnabled();
        bool ledEnabled = filter.ledFilterEnabled();
        bool hiEnabled = filter.hiFilterEnabled();
        bool legacyEnabled = filter.legacyFilterEnabled();

        for (isize i = 0; i < count; i++) {

            float ch0 = sampler[0].interpolate <method> ((Cycle)cycle) * vol[0];
            float ch1 = sampler[1].interpolate <method> ((Cycle)cycle) * vol[1];
            float ch2 = sampler[2].interpolate <method> ((Cycle)cycle) * vol[2];
            float ch3 = sampler[3].interpolate <method> ((Cycle)cycle) * vol[3];

            // Compute left channel output
            double l =
            ch0 * (1 - pan[0]) + ch1 * (1 - pan[1]) +
            ch2 * (1 - pan[2]) + ch3 * (1 - pan[3]);

            // Compute right channel output
            double r =
            ch0 * pan[0] + ch1 * pan[1] +
            ch2 * pan[2] + ch3 * pan[3];

            // Run the audio filter pipeline
            if (loEnabled) filter.loFilter.applyLP(l, r);
            if (ledEnabled) filter.ledFilter.applyLP(l, r);
            if (hiEnabled) filter.hiFilter.applyHP(l, r);

            // Apply the legacy filter if applicable
            if (legacyEnabled) {
                l = filter.butterworthL.apply(float(l));
                r = filter.butterworthR.apply(float(r));
            }

            // Apply master volume
            l *= volL;
            r *= volR;

            // Write sample into ringbuffer
            stream.add(float(l), float(r));

            cycle += cyclesPerSample;
        }
    } else {

        // Fast path: Repeat the most recent sample
        auto latest = stream.isEmpty() ? SAMPLE_T() : stream.latest();
        for (isize i = 0; i < count; i++) {
            stream.add(latest);
        }
    }

    stats.producedSamples += count;

    stream.unlock();
}

void
Muxer::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer
    // (2) The producer is halted or not startet yet
    
    debug(AUDBUF_DEBUG, "UNDERFLOW (r: %ld w: %ld)\n", stream.r, stream.w);
    
    // Reset the write pointer
    stream.alignWritePtr();

    // Determine the elapsed seconds since the last pointer adjustment
    auto elapsedTime = util::Time::now() - lastAlignment;
    lastAlignment = util::Time::now();
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime.asSeconds() > 10.0) {

        stats.bufferUnderflows++;
        
        // Increase the sample rate based on what we've measured
        /*
        auto offPerSec = (stream.cap() / 2) / elapsedTime.asSeconds();
        setSampleRate(host.getSampleRate() + (isize)offPerSec);
        */
    }
}

void
Muxer::handleBufferOverflow()
{
    // There are two common scenarios in which buffer overflows occur:
    //
    // (1) The consumer runs slightly slower than the producer
    // (2) The consumer is halted or not startet yet
    
    debug(AUDBUF_DEBUG, "OVERFLOW (r: %ld w: %ld)\n", stream.r, stream.w);
    
    // Reset the write pointer
    stream.alignWritePtr();

    // Determine the number of elapsed seconds since the last adjustment
    auto elapsedTime = util::Time::now() - lastAlignment;
    lastAlignment = util::Time::now();
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime.asSeconds() > 10.0) {
        
        stats.bufferOverflows++;
        
        // Decrease the sample rate based on what we've measured
        /*
        auto offPerSec = (stream.cap() / 2) / elapsedTime.asSeconds();
        setSampleRate(host.getSampleRate() - (isize)offPerSec);
        */
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

SAMPLE_T *
Muxer::nocopy(isize n)
{
    stream.lock();
    
    if (stream.count() < n) handleBufferUnderflow();
    SAMPLE_T *addr = stream.currentAddr();
    stream.skip(n);
    stats.consumedSamples += n;

    stream.unlock();
    return addr;
}

}
