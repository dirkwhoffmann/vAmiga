// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MuxerTypes.h"

#include "SubComponent.h"
#include "AudioStream.h"
#include "AudioFilter.h"
#include "Chrono.h"
#include "Sampler.h"

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

class Muxer : public SubComponent {

    Descriptions descriptions = {{

        .name           = "Muxer",
        .description    = "Audio Muxer"
    }};

    ConfigOptions options = {

        OPT_SAMPLING_METHOD,
        OPT_AUDPAN0,
        OPT_AUDPAN1,
        OPT_AUDPAN2,
        OPT_AUDPAN3,
        OPT_AUDVOL0,
        OPT_AUDVOL1,
        OPT_AUDVOL2,
        OPT_AUDVOL3,
        OPT_AUDVOLL,
        OPT_AUDVOLR,
        OPT_AUD_FASTPATH,
        OPT_FILTER_TYPE
    };

    friend class Paula;
    
    // Current configuration
    MuxerConfig config = {};
    
    // Underflow and overflow counters
    MuxerStats stats = {};
    
    // Master clock cycles per audio sample
    // double cyclesPerSample = 0.0;

    // Fraction of a sample that hadn't been generated in synthesize
    double fraction = 0.0;

    // Time stamp of the last write pointer alignment
    util::Time lastAlignment;

    // Volume control
    Volume volume;

    // Volume scaling factors
    float vol[4];
    float volL;
    float volR;

    // Panning factors
    float pan[4];
    
    
    //
    // Sub components
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
    AudioStream<SAMPLE_T> stream;
    
    // The audio filter pipeline
    AudioFilter filter = AudioFilter(amiga);

    
    //
    // Initializing
    //
    
public:
    
    Muxer(Amiga& ref);

    // Resets the output buffer and the two audio filters
    void clear();


    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _initialize() override;
    void _reset(bool hard) override;
    
    template <class T>
    void serialize(T& worker)
    {
     
        if (util::isResetter(worker)) return;

        worker

        << config.samplingMethod
        << config.pan
        << config.vol
        << config.volL
        << config.volR
        << pan
        << vol
        << volL
        << volR;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:
    
    const MuxerConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    void resetConfig() override;
    i64 getOption(Option option) const override;
    void setOption(Option option, i64 value) override;

    void setSampleRate(double hz);


    //
    // Analyzing
    //
    
public:
    
    // Returns information about the gathered statistical information
    const MuxerStats &getStats() const { return stats; }

    // Returns true if the output volume is zero
    bool isMuted() const { return config.volL == 0 && config.volR == 0; }


    //
    // Controlling volume
    //
    
public:

    /* Starts to ramp up the volume. This function configures variables volume
     * and targetVolume to simulate a smooth audio fade in.
     */
    void rampUp();
    void rampUpFromZero();
    
    /* Starts to ramp down the volume. This function configures variables
     * volume and targetVolume to simulate a quick audio fade out.
     */
    void rampDown();
    
    
    //
    // Generating audio streams
    //
    
public:

    // Entry point for the screen recorder
    void synthesize(Cycle clock, Cycle target, long count);

    // Entry point for the core emulator
    void synthesize(Cycle clock, Cycle target);

private:

    template <SamplingMethod method>
    void synthesize(Cycle clock, long count, double cyclesPerSample);
    
    // Handles a buffer underflow or overflow condition
    void handleBufferUnderflow();
    void handleBufferOverflow();
    
public:
    
    // Signals to ignore the next underflow or overflow condition
    void ignoreNextUnderOrOverflow();


    //
    // Reading audio samples
    //
    
public:
    
    // Copies a certain amout of audio samples into a buffer
    void copy(void *buffer, isize n);
    void copy(void *buffer1, void *buffer2, isize n);
    
    /* Returns a pointer to a buffer holding a certain amount of audio samples
     * without copying data. This function has been implemented for speedup.
     * Instead of copying ring buffer data into the target buffer, it returns
     * a pointer into the ringbuffer itself. The caller has to make sure that
     * the ring buffer's read pointer is not closer than n elements to the
     * buffer end.
     */
    SAMPLE_T *nocopy(isize n);
};

}
