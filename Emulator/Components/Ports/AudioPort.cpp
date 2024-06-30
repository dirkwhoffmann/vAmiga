// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AudioPort.h"
#include "Emulator.h"
#include "CIA.h"
#include "IOUtils.h"
#include "MsgQueue.h"
#include <cmath>
#include <algorithm>

namespace vamiga {

AudioPort::AudioPort(Amiga& ref, isize objid) : SubComponent(ref, objid)
{
    subComponents = std::vector<CoreComponent *> {

        &filter
    };
}

void
AudioPort::_dump(Category category, std::ostream& os) const
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
        os << tab("Master volume left");
        os << flt(volL.current) << " (0 ... " << flt(volL.maximum) << ")" << std::endl;
        os << tab("Master volume right");
        os << flt(volR.current) << " (0 ... " << flt(volR.maximum) << ")" << std::endl;
        os << tab("Channel 0 volume");
        os << flt(vol[0]) << std::endl;
        os << tab("Channel 1 volume");
        os << flt(vol[1]) << std::endl;
        os << tab("Channel 2 volume");
        os << flt(vol[2]) << std::endl;
        os << tab("Channel 3 volume");
        os << flt(vol[3]) << std::endl;
        os << tab("Channel 0 pan");
        os << flt(pan[0]) << std::endl;
        os << tab("Channel 1 pan");
        os << flt(pan[1]) << std::endl;
        os << tab("Channel 2 pan");
        os << flt(pan[2]) << std::endl;
        os << tab("Channel 3 pan");
        os << flt(pan[3]) << std::endl;
        os << tab("Sample rate correction");
        os << flt(sampleRateCorrection) << " Hz" << std::endl;
    }
}

void
AudioPort::_initialize()
{
    CoreComponent::_initialize();

    setSampleRate(44100);
}

void
AudioPort::_didReset(bool hard)
{
    stats = { };
    for (isize i = 0; i < 4; i++) sampler[i].reset();
    clear();
}

void
AudioPort::_powerOn()
{
    sampleRateCorrection = 0.0;
}

void
AudioPort::_run()
{
    unmute(10000);
}

void
AudioPort::_pause()
{
    fadeOut();
    mute();
}

void
AudioPort::_warpOn()
{
    fadeOut();
    mute();
}

void
AudioPort::_warpOff()
{
    unmute(10000);
}

void
AudioPort::_focus()
{
    unmute(100000);
}

void
AudioPort::_unfocus()
{
    mute(100000);
}

void
AudioPort::clear()
{
    debug(AUDBUF_DEBUG, "clear()\n");
    
    // Wipe out the ringbuffer
    stream.wipeOut();
    stream.alignWritePtr();

    // Wipe out the filter buffers
    filter.clear();
}

i64
AudioPort::getOption(Option option) const
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
AudioPort::setOption(Option option, i64 value)
{
    bool wasMuted = isMuted();
    isize channel = 0;

    switch (option) {
            
        case OPT_AUD_SAMPLING_METHOD:
            
            if (!SamplingMethodEnum::isValid(value)) {
                throw Error(ERROR_OPT_INV_ARG, SamplingMethodEnum::keyList());
            }
            
            config.samplingMethod = (SamplingMethod)value;
            return;
            
        case OPT_AUD_VOL3: channel++;
        case OPT_AUD_VOL2: channel++;
        case OPT_AUD_VOL1: channel++;
        case OPT_AUD_VOL0:

            config.vol[channel] = std::clamp(value, 0LL, 100LL);
            vol[channel] = float(pow(config.vol[channel] / 100.0, 1.4) * 0.000025);
            if (emscripten) vol[channel] *= 0.15f;
            return;

        case OPT_AUD_VOLL:
            
            config.volL = std::clamp(value, 0LL, 100LL);
            volL = float(pow(value / 50.0, 1.4));

            if (wasMuted != isMuted())
                msgQueue.put(MSG_MUTE, isMuted());
            return;
            
        case OPT_AUD_VOLR:

            config.volR = std::clamp(value, 0LL, 100LL);
            volR = float(pow(value / 50.0, 1.4));

            if (wasMuted != isMuted())
                msgQueue.put(MSG_MUTE, isMuted());
            return;

        case OPT_AUD_PAN3: channel++;
        case OPT_AUD_PAN2: channel++;
        case OPT_AUD_PAN1: channel++;
        case OPT_AUD_PAN0:

            config.pan[channel] = value;
            pan[channel] = float(0.5 * (sin(config.pan[channel] * M_PI / 200.0) + 1));
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
AudioPort::setSampleRate(double hz)
{
    trace(AUD_DEBUG, "setSampleRate(%f)\n", hz);

    filter.setup(hz);
}

void
AudioPort::_didLoad()
{
    for (isize i = 0; i < 4; i++) sampler[i].reset();
}

void
AudioPort::fadeOut()
{
    stream.fadeOut();
    volL.current = 0;
    volR.current = 0;
}

void
AudioPort::synthesize(Cycle clock, Cycle target, long count)
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
AudioPort::synthesize(Cycle clock, Cycle target)
{
    assert(target > clock);

    /*
    static int cnt = 0;
    if (cnt++ % 15 == 0) printf("Volume: L: %f (%f) R:%f (%f)\n",
                                (float)volL, volL.maximum, (float)volR, volR.maximum);
    */

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
AudioPort::synthesize(Cycle clock, long count, double cyclesPerSample)
{
    assert(count > 0);

    float vol0 = vol[0]; float pan0 = pan[0];
    float vol1 = vol[1]; float pan1 = pan[1];
    float vol2 = vol[2]; float pan2 = pan[2];
    float vol3 = vol[3]; float pan3 = pan[3];
    bool fading = volL.isFading() || volR.isFading();

    stream.mutex.lock();

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

            float ch0 = sampler[0].interpolate <method> ((Cycle)cycle) * vol0;
            float ch1 = sampler[1].interpolate <method> ((Cycle)cycle) * vol1;
            float ch2 = sampler[2].interpolate <method> ((Cycle)cycle) * vol2;
            float ch3 = sampler[3].interpolate <method> ((Cycle)cycle) * vol3;

            // Compute left channel output
            double l =
            ch0 * (1 - pan0) + ch1 * (1 - pan1) +
            ch2 * (1 - pan2) + ch3 * (1 - pan3);

            // Compute right channel output
            double r =
            ch0 * pan0 + ch1 * pan1 +
            ch2 * pan2 + ch3 * pan3;

            // Run the audio filter pipeline
            if (loEnabled) filter.loFilter.applyLP(l, r);
            if (ledEnabled) filter.ledFilter.applyLP(l, r);
            if (hiEnabled) filter.hiFilter.applyHP(l, r);

            // Apply the legacy filter if applicable
            if (legacyEnabled) {
                l = filter.butterworthL.apply(float(l));
                r = filter.butterworthR.apply(float(r));
            }

            // Modulate the master volume
            if (fading) { volL.shift(); volR.shift(); }

            // Apply master volume
            l *= volL;
            r *= volR;

            // Prevent hearing loss
            assert(abs(l) < 1.0);
            assert(abs(r) < 1.0);

            // Write sample into ringbuffer
            stream.write( SamplePair { float(l), float(r) } );

            cycle += cyclesPerSample;
        }

    } else {

        // Fast path: Repeat the most recent sample
        auto latest = stream.isEmpty() ? SamplePair() : stream.latest();
        
        for (isize i = 0; i < count; i++) {
            stream.write(latest);
        }
    }

    stats.producedSamples += count;

    stream.mutex.unlock();
}

void
AudioPort::handleBufferUnderflow()
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
AudioPort::handleBufferOverflow()
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
AudioPort::ignoreNextUnderOrOverflow()
{
    lastAlignment = util::Time::now();
}

isize
AudioPort::copyMono(float *buffer, isize n)
{
    // Copy sound samples
    auto cnt = stream.copyMono(buffer, n);
    stats.consumedSamples += cnt;

    // Check for a buffer underflow
    if (cnt < n) handleBufferUnderflow();

    return n;
}

isize
AudioPort::copyStereo(float *buffer1, float *buffer2, isize n)
{
    // Copy sound samples
    auto cnt = stream.copyStereo(buffer1, buffer2, n);
    stats.consumedSamples += cnt;

    // Check for a buffer underflow
    if (cnt < n) handleBufferUnderflow();

    return n;
}

}
