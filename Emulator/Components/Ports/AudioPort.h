// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AudioPortTypes.h"
#include "SubComponent.h"
#include "Animated.h"
#include "AudioStream.h"
#include "AudioFilter.h"
#include "Chrono.h"
#include "Sampler.h"
#include "SampleRateDetector.h"

namespace vamiga {

/* Architecture of the audio pipeline
 *
 *           Mux class
 *           -----------------------------------------------------
 *  State   |   ---------                                         |
 * machine -|->| Sampler |-> vol ->|                              |
 *    0     |   ---------          |                              |
 *          |                      |                              |
 *  State   |   ---------          |                              |
 * machine -|->| Sampler |-> vol ->|                              |
 *    1     |   ---------          |     pan     --------------   |
 *          |                      |--> l vol ->| Audio Stream |--|-> GUI
 *  State   |   ---------          |    r vol    --------------   |
 * machine -|->| Sampler |-> vol ->|                              |
 *    2     |   ---------          |                              |
 *          |                      |                              |
 *  State   |   ---------          |                              |
 * machine -|->| Sampler |-> vol ->|                              |
 *    3     |   ---------                                         |
 *           -----------------------------------------------------
 */

class AudioPort final : public SubComponent {

    friend class AudioFilter;

    Descriptions descriptions = {
        {
            .type           = AudipPortClass,
            .name           = "AudioPort",
            .description    = "Audio Port",
            .shell          = "audio"
        },
        {
            .type           = AudipPortClass,
            .name           = "RecAudioPort",
            .description    = "Audio Port (Recorder)",
            .shell          = ""
        },
    };

    ConfigOptions options = {

        OPT_AUD_SAMPLING_METHOD,
        OPT_AUD_PAN0,
        OPT_AUD_PAN1,
        OPT_AUD_PAN2,
        OPT_AUD_PAN3,
        OPT_AUD_VOL0,
        OPT_AUD_VOL1,
        OPT_AUD_VOL2,
        OPT_AUD_VOL3,
        OPT_AUD_VOLL,
        OPT_AUD_VOLR,
        OPT_AUD_FASTPATH
    };

    friend class Paula;

    // Current configuration
    AudioPortConfig config = {};
    
    // Underflow and overflow counters
    AudioPortStats stats = {};

    // Current sample rate
    double sampleRate = 44100;

    // Fraction of a sample that hadn't been generated in synthesize
    double fraction = 0.0;

    // Time stamp of the last write pointer alignment
    util::Time lastAlignment = util::Time::now();
    
    // Channel volumes
    float vol[4] = { };

    // Panning factors
    float pan[4] ={ };

    // Master volumes (fadable)
    util::Animated<float> volL;
    util::Animated<float> volR;

    // Used to determine if a MSG_MUTE should be send
    bool wasMuted = false;


    //
    // Subcomponents
    //
    
public:

    // Inputs (one Sampler for each of the four channels)
    Sampler sampler[4] = {
        
        Sampler(),
        Sampler(),
        Sampler(),
        Sampler()
    };

    // Output
    AudioStream stream;

    // The audio filter pipeline
    AudioFilter filter = AudioFilter(amiga, *this);

    // Detector for measuring the sample rate
    SampleRateDetector detector = SampleRateDetector(amiga);


    //
    // Methods
    //
    
public:
    
    AudioPort(Amiga& ref, isize objid = 0);

    AudioPort& operator= (const AudioPort& other) {

        CLONE(filter)
        
        CLONE(config)
        CLONE_ARRAY(pan)
        CLONE_ARRAY(vol)
        CLONE(volL)
        CLONE(volR)

        return *this;
    }


    // Resets the output buffer and the two audio filters
    void clear();

    
    //
    // Methods from Serializable
    //
    
private:

    template <class T>
    void serialize(T& worker)
    {

        if (isResetter(worker)) return;

        worker

        << config.samplingMethod
        << config.pan
        << config.vol
        << config.volL
        << config.volR
        << pan
        << vol
        << volL.maximum
        << volR.maximum;

    } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream& os) const override;
    void _didLoad() override;
    void _didReset(bool hard) override;
    void _initialize() override;
    void _powerOn() override;
    void _run() override;
    void _pause() override;
    void _warpOn() override;
    void _warpOff() override;
    void _focus() override;
    void _unfocus() override;


    //
    // Methods from Configurable
    //

public:
    
    const AudioPortConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Option option) const override;
    void checkOption(Option opt, i64 value) override;
    void setOption(Option option, i64 value) override;

    void setSampleRate(double hz);


    //
    // Analyzing
    //
    
public:
    
    // Returns information about the gathered statistical information
    const AudioPortStats &getStats() const { return stats; }

    // Returns true if the output volume is zero
    bool isMuted() const;


    //
    // Generating audio streams
    //
    
public:

    // Entry point for the screen recorder
    void synthesize(Cycle clock, Cycle target, long count);

    // Entry point for the core emulator
    void synthesize(Cycle clock, Cycle target);

    // Returns the sample rate adjustment
    // double getSampleRateCorrection() { return sampleRateCorrection; }

private:

    void synthesize(Cycle clock, long count, double cyclesPerSample);
    template <SamplingMethod method>
    void synthesize(Cycle clock, long count, double cyclesPerSample);

    // Handles a buffer underflow or overflow condition
    void handleBufferUnderflow();
    void handleBufferOverflow();
    
public:
    
    // Signals to ignore the next underflow or overflow condition
    void ignoreNextUnderOrOverflow();


    //
    // Controlling volume
    //

public:

    // Rescale the existing samples to gradually fade out (to avoid cracks)
    void eliminateCracks();

    // Gradually decrease the master volume to zero
    void mute(isize steps = 0) { volL.fadeOut(steps); volR.fadeOut(steps); }

    // Gradually inrease the master volume to max
    void unmute(isize steps = 0) { volL.fadeIn(steps); volR.fadeIn(steps); }


    //
    // Reading audio samples
    //
    
public:
    
    /* Copies n audio samples into a memory buffer. These functions mark the
     * final step in the audio pipeline. They are used to copy the generated
     * sound samples into the buffers of the native sound device. The function
     * returns the number of copied samples.
     */
    isize copyMono(float *buffer, isize n);
    isize copyStereo(float *left, float *right, isize n);
    isize copyInterleaved(float *buffer, isize n);
};

}
