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

        &filter,
        &detector
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
        os << tab("Sample rate");
        os << flt(sampleRate) << " Hz" << std::endl;
    }
}

void
AudioPort::_initialize()
{

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

}

void
AudioPort::_run()
{
    unmute(10000);
}

void
AudioPort::_pause()
{
    eliminateCracks();
    mute();
}

void
AudioPort::_warpOn()
{
    eliminateCracks();
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
    debug(AUDBUF_DEBUG, "Clearing the audio sample buffer\n");

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
        case OPT_AUD_PAN0:              return config.pan[0];
        case OPT_AUD_PAN1:              return config.pan[1];
        case OPT_AUD_PAN2:              return config.pan[2];
        case OPT_AUD_PAN3:              return config.pan[3];
        case OPT_AUD_VOL0:              return config.vol[0];
        case OPT_AUD_VOL1:              return config.vol[1];
        case OPT_AUD_VOL2:              return config.vol[2];
        case OPT_AUD_VOL3:              return config.vol[3];
        case OPT_AUD_VOLL:              return config.volL;
        case OPT_AUD_VOLR:              return config.volR;
        case OPT_AUD_FASTPATH:          return config.idleFastPath;

        default:
            fatalError;
    }
}

void
AudioPort::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_AUD_SAMPLING_METHOD:

            if (!SamplingMethodEnum::isValid(value)) {
                throw Error(VAERROR_OPT_INV_ARG, SamplingMethodEnum::keyList());
            }
            return;

        case OPT_AUD_PAN0:
        case OPT_AUD_PAN1:
        case OPT_AUD_PAN2:
        case OPT_AUD_PAN3:
        case OPT_AUD_VOL0:
        case OPT_AUD_VOL1:
        case OPT_AUD_VOL2:
        case OPT_AUD_VOL3:
        case OPT_AUD_VOLL:
        case OPT_AUD_VOLR:
        case OPT_AUD_FASTPATH:

            return;

        default:
            throw(VAERROR_OPT_UNSUPPORTED);
    }
}

void
AudioPort::setOption(Option option, i64 value)
{
    isize channel = 0;

    switch (option) {
            
        case OPT_AUD_SAMPLING_METHOD:
                        
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
            return;

        case OPT_AUD_VOLR:

            config.volR = std::clamp(value, 0LL, 100LL);
            volR = float(pow(value / 50.0, 1.4));
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

        default:
            fatalError;
    }
}

void
AudioPort::setSampleRate(double hz)
{
    // Set the sample rate or get it from the detector if none is provided
    if (hz != 0.0) {

        sampleRate = hz;
        trace(AUD_DEBUG, "setSampleRate(%.2f)\n", sampleRate);

    } else {

        sampleRate = detector.sampleRate();
        trace(AUD_DEBUG, "setSampleRate(%.2f) (predicted)\n", sampleRate);
    }

    // Inform the audio filter about the new sample rate
    filter.setup(sampleRate);
}

void
AudioPort::_didLoad()
{
    for (isize i = 0; i < 4; i++) sampler[i].reset();
}

void
AudioPort::eliminateCracks()
{
    stream.eliminateCracks();
    volL.current = 0;
    volR.current = 0;
}

bool 
AudioPort::isMuted() const
{
    if (volL.isFading() || volR.isFading()) return false;
    return volL + volR == 0.0 || vol[0] + vol[1] + vol[2] + vol[3] == 0.0;
}

void
AudioPort::synthesize(Cycle clock, Cycle target, long count)
{
    assert(target > clock);
    assert(count > 0);

    // Determine the number of elapsed cycles per audio sample
    double cps = (double)(target - clock) / (double)count;

    // Synthesize samples
    synthesize(clock, count, cps);
}

void
AudioPort::synthesize(Cycle clock, Cycle target)
{
    assert(target > clock);

    // Do not synthesize anything if this is the run-ahead instance
    if (amiga.objid != 0) return;

    // Determine the number of elapsed cycles per audio sample
    double cps = double(amiga.masterClockFrequency()) / sampleRate;

    // Determine how many samples we need to produce
    double exact = (double)(target - clock) / cps + fraction;

    // Extract the integer part and remember the rest
    double count; fraction = std::modf(exact, &count);

    // Synthesize samples
    synthesize(clock, (long)count, cps);
}

void 
AudioPort::synthesize(Cycle clock, long count, double cyclesPerSample)
{
    bool muted = isMuted();

    // Send the MUTE message if needed
    if (muted != wasMuted) { msgQueue.put(MSG_MUTE, wasMuted = muted); }

    stream.mutex.lock();

    // Check for a buffer overflow
    if (stream.count() + count >= stream.cap()) handleBufferOverflow();

    // Check if we can take a fast path
    if (config.idleFastPath) {

        if (muted) {

            // Fill with zeroes
            for (isize i = 0; i < count; i++) stream.write( SamplePair { 0, 0 } );
            stats.idleSamples += count;
            stream.mutex.unlock();
            return;
        }
        if (!sampler[0].isActive() && !sampler[1].isActive() &&
            !sampler[2].isActive() && !sampler[3].isActive()) {

            // Repeat the most recent sample
            auto latest = stream.isEmpty() ? SamplePair() : stream.latest();
            for (isize i = 0; i < count; i++) stream.write(latest);
            stats.idleSamples += count;
            stream.mutex.unlock();
            return;
        }
    }

    // Take the slow path
    switch (config.samplingMethod) {

        case SMP_NONE:      synthesize<SMP_NONE>(clock, count, cyclesPerSample); break;
        case SMP_NEAREST:   synthesize<SMP_NEAREST>(clock, count, cyclesPerSample); break;
        case SMP_LINEAR:    synthesize<SMP_LINEAR>(clock, count, cyclesPerSample); break;

        default:
            fatalError;
    }

    stream.mutex.unlock();
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

    double cycle = (double)clock;
    bool loEnabled = filter.loFilterEnabled();
    bool ledEnabled = filter.ledFilterEnabled();
    bool hiEnabled = filter.hiFilterEnabled();

    for (isize i = 0; i < count; i++) {

        float ch0 = sampler[0].interpolate <method> ((Cycle)cycle) * vol0;
        float ch1 = sampler[1].interpolate <method> ((Cycle)cycle) * vol1;
        float ch2 = sampler[2].interpolate <method> ((Cycle)cycle) * vol2;
        float ch3 = sampler[3].interpolate <method> ((Cycle)cycle) * vol3;

        // Compute left and right channel output
        double l = ch0 * (1 - pan0) + ch1 * (1 - pan1) + ch2 * (1 - pan2) + ch3 * (1 - pan3);
        double r = ch0 * pan0 + ch1 * pan1 + ch2 * pan2 + ch3 * pan3;

        // Run the audio filter pipeline
        if (loEnabled) filter.loFilter.applyLP(l, r);
        if (ledEnabled) filter.ledFilter.applyLP(l, r);
        if (hiEnabled) filter.hiFilter.applyHP(l, r);

        // Modulate the master volume
        if (fading) { volL.shift(); volR.shift(); }

        // Apply master volume
        l *= volL;
        r *= volR;

        // Prevent hearing loss
        assert(std::abs(l) < 1.0);
        assert(std::abs(r) < 1.0);

        // Write sample into ringbuffer
        stream.write( SamplePair { float(l), float(r) } );

        cycle += cyclesPerSample;
    }

    stats.producedSamples += count;
}

void
AudioPort::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer
    // (2) The producer is halted or not startet yet
    
    // debug(AUDBUF_DEBUG, "UNDERFLOW (r: %ld w: %ld)\n", stream.r, stream.w);

    // Reset the write pointer
    stream.clear(SamplePair{0,0});
    stream.alignWritePtr();

    // Determine the elapsed seconds since the last pointer adjustment
    auto elapsedTime = util::Time::now() - lastAlignment;
    lastAlignment = util::Time::now();
    
    // Adjust the sample rate, if condition (1) holds
    if (emulator.isRunning() && !emulator.isWarping()) {

        stats.bufferUnderflows++;
        debug(AUDBUF_DEBUG, "Audio buffer underflow after %f seconds\n", elapsedTime.asSeconds());

        // Adjust the sample rate
        setSampleRate(host.getConfig().sampleRate);
        debug(AUDBUF_DEBUG, "New sample rate = %.2f\n", sampleRate);
    }
}

void
AudioPort::handleBufferOverflow()
{
    // There are two common scenarios in which buffer overflows occur:
    //
    // (1) The consumer runs slightly slower than the producer
    // (2) The consumer is halted or not startet yet

    // debug(AUDBUF_DEBUG, "OVERFLOW (r: %ld w: %ld)\n", stream.r, stream.w);

    // Reset the write pointer
    stream.alignWritePtr();

    // Determine the number of elapsed seconds since the last adjustment
    auto elapsedTime = util::Time::now() - lastAlignment;
    lastAlignment = util::Time::now();

    // Adjust the sample rate, if condition (1) holds
    if (emulator.isRunning() && !emulator.isWarping()) {

        stats.bufferOverflows++;
        debug(AUDBUF_DEBUG, "Audio buffer overflow after %f seconds\n", elapsedTime.asSeconds());

        // Adjust the sample rate
        setSampleRate(host.getConfig().sampleRate);
        debug(AUDBUF_DEBUG, "New sample rate = %.2f\n", sampleRate);
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

    return cnt;
}

isize
AudioPort::copyStereo(float *buffer1, float *buffer2, isize n)
{
    // Inform the sample rate detector about the number of requested samples
    detector.feed(n);

    // Copy sound samples
    auto cnt = stream.copyStereo(buffer1, buffer2, n);
    stats.consumedSamples += cnt;

    // Check for a buffer underflow
    if (cnt < n) handleBufferUnderflow();

    return cnt;
}

isize
AudioPort::copyInterleaved(float *buffer, isize n)
{
    // Copy sound samples
    auto cnt = stream.copyInterleaved(buffer, n);
    stats.consumedSamples += cnt;

    // Check for a buffer underflow
    if (cnt < n) handleBufferUnderflow();

    return cnt;
}

}
