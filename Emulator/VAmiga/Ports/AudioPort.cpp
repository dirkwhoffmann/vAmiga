// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
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
        os << tab("Buffer capacity");
        os << stream.cap() << std::endl;
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
    if (!emulator.isWarping()) unmute(10000);
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
AudioPort::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::AUD_PAN0:              return (i64)config.pan[0];
        case Opt::AUD_PAN1:              return (i64)config.pan[1];
        case Opt::AUD_PAN2:              return (i64)config.pan[2];
        case Opt::AUD_PAN3:              return (i64)config.pan[3];
        case Opt::AUD_VOL0:              return (i64)config.vol[0];
        case Opt::AUD_VOL1:              return (i64)config.vol[1];
        case Opt::AUD_VOL2:              return (i64)config.vol[2];
        case Opt::AUD_VOL3:              return (i64)config.vol[3];
        case Opt::AUD_VOLL:              return (i64)config.volL;
        case Opt::AUD_VOLR:              return (i64)config.volR;
        case Opt::AUD_BUFFER_SIZE:       return (i64)config.bufferSize;
        case Opt::AUD_SAMPLING_METHOD:   return (i64)config.samplingMethod;
        case Opt::AUD_ASR:               return (i64)config.asr;
        case Opt::AUD_FASTPATH:          return (i64)config.idleFastPath;

        default:
            fatalError;
    }
}

void
AudioPort::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::AUD_PAN0:
        case Opt::AUD_PAN1:
        case Opt::AUD_PAN2:
        case Opt::AUD_PAN3:
        case Opt::AUD_VOL0:
        case Opt::AUD_VOL1:
        case Opt::AUD_VOL2:
        case Opt::AUD_VOL3:
        case Opt::AUD_VOLL:
        case Opt::AUD_VOLR:

            return;

        case Opt::AUD_BUFFER_SIZE:

            if (value < 512 || value > 65536) {
                throw CoreError(Fault::OPT_INV_ARG, "512 ... 65536");
            }
            return;
            
        case Opt::AUD_SAMPLING_METHOD:

            if (!SamplingMethodEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, SamplingMethodEnum::keyList());
            }
            return;
            
        case Opt::AUD_ASR:
        case Opt::AUD_FASTPATH:

            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
AudioPort::setOption(Opt option, i64 value)
{
    isize channel = 0;

    switch (option) {
                        
        case Opt::AUD_VOL3: channel++;
        case Opt::AUD_VOL2: channel++;
        case Opt::AUD_VOL1: channel++;
        case Opt::AUD_VOL0:

            config.vol[channel] = std::clamp(value, 0LL, 100LL);
            vol[channel] = float(pow(config.vol[channel] / 100.0, 1.4) * 0.000025);
            if (emscripten) vol[channel] *= 0.15f;
            return;

        case Opt::AUD_VOLL:
            
            config.volL = std::clamp(value, 0LL, 100LL);
            volL = float(pow(value / 50.0, 1.4));
            return;

        case Opt::AUD_VOLR:

            config.volR = std::clamp(value, 0LL, 100LL);
            volR = float(pow(value / 50.0, 1.4));
            return;

        case Opt::AUD_PAN3: channel++;
        case Opt::AUD_PAN2: channel++;
        case Opt::AUD_PAN1: channel++;
        case Opt::AUD_PAN0:

            config.pan[channel] = value;
            pan[channel] = float(0.5 * (sin(config.pan[channel] * M_PI / 200.0) + 1));
            return;

        case Opt::AUD_BUFFER_SIZE:
                        
            config.bufferSize = value;
            stream.resize(value);
            return;
            
        case Opt::AUD_SAMPLING_METHOD:
                        
            config.samplingMethod = (SamplingMethod)value;
            return;

        case Opt::AUD_ASR:
                        
            config.asr = (bool)value;
            return;

        case Opt::AUD_FASTPATH:

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

    // Run the ASR algorithm (adaptive sample rate)
    if (config.asr) { updateSampleRateCorrection(); } else { sampleRateCorrection = 0.0; }
    
    // Determine the number of elapsed cycles per audio sample
    double cps = double(amiga.masterClockFrequency()) / (sampleRate + sampleRateCorrection);
    
    // Determine how many samples we need to produce
    double exact = (double)(target - clock) / cps + fraction;

    // Extract the integer part and remember the rest
    double count; fraction = std::modf(exact, &count);
    
    // Synthesize samples
    synthesize(clock, (long)count, cps);
}

void
AudioPort::updateSampleRateCorrection()
{
    // Compute the difference between the ideal and the current fill level
    auto error = (0.5 - stream.fillLevel());
    
    // Smooth it out
    sampleRateError = 0.75 * sampleRateError + 0.25 * error;

    // Compute a sample rate correction
    auto correction = (0.5 - stream.fillLevel()) * 4000;

    // Smooth it out
    sampleRateCorrection = (sampleRateCorrection * 0.75) + (correction * 0.25);
    
    debug(AUDBUF_DEBUG, "ASR correction: %.0f Hz (fill: %.2f)\n",
          sampleRateCorrection, stream.fillLevel());
}

void
AudioPort::synthesize(Cycle clock, long count, double cyclesPerSample)
{
    bool muted = isMuted();

    // Send the MUTE message if needed
    if (muted != wasMuted) { msgQueue.put(Msg::MUTE, wasMuted = muted); }

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

            // Copy zeroes if nothing can be heared any more
            auto latest = stream.isEmpty() ? SamplePair{} : stream.latest();
            if (std::abs(latest.l) + std::abs(latest.r) < 1e-8) {
                
                for (isize i = 0; i < count; i++) stream.write(SamplePair{});
                stats.idleSamples += count;
                stream.mutex.unlock();
                return;
            }
        }
    }

    // Take the slow path
    switch (config.samplingMethod) {

        case SamplingMethod::NONE:      synthesize<SamplingMethod::NONE>(clock, count, cyclesPerSample); break;
        case SamplingMethod::NEAREST:   synthesize<SamplingMethod::NEAREST>(clock, count, cyclesPerSample); break;
        case SamplingMethod::LINEAR:    synthesize<SamplingMethod::LINEAR>(clock, count, cyclesPerSample); break;

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
        stream.put( SamplePair { float(l), float(r) } );

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

    // Wipe out the buffer and reset the write pointer
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
AudioPort::copyStereo(float *left, float *right, isize n)
{
    // Inform the sample rate detector about the number of requested samples
    detector.feed(n);

    // Copy sound samples
    auto cnt = stream.copyStereo(left, right, n);
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
