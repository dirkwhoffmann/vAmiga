// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _MUXER_H
#define _MUXER_H

#include "AmigaComponent.h"
#include "AudioStream.h"
#include "AudioFilter.h"

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

struct Volume {

    // Maximum volume
    const static i32 maxVolume = 100000;

    // Current volume (will eventually reach the target volume)
    i32 current = maxVolume;

    // Target volume
    i32 target = maxVolume;

    // Delta steps (added to volume until the target volume is reached)
    i32 delta = 0;

    // Shifts the current volume towards the target volume
    void shift() {
        if (current < target) {
            current += MIN(delta, target - current);
        } else {
            current -= MIN(delta, current - target);
        }
    }
};

class Muxer : public AmigaComponent {

    // Current configuration
    MuxerConfig config;
    
    // Underflow and overflow counters
    MuxerStats stats;

    // Sample rate in Hz
    double sampleRate = 0;
    
    // Master clock cycles per audio sample
    double cyclesPerSample = 0;

    // Fraction of a sample that hadn't been generated in synthesize
    double fraction;

    // Time stamp of the last write pointer alignment
    Cycle lastAlignment = 0;

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
    Sampler *sampler[4];

    // Output
    AudioStream stream;
    
    // Audio filters
    AudioFilter filterL = AudioFilter(amiga);
    AudioFilter filterR = AudioFilter(amiga);
        
    
    //
    // Initializing
    //
    
public:
    
    Muxer(Amiga& ref);
    ~Muxer();

    const char *getDescription() override { return "Muxer"; }
    void _reset(bool hard) override;
    
    // Resets the output buffer and the two audio filters
    void clear();
    
    
    //
    // Configuring
    //
    
public:
    
    MuxerConfig getConfig() { return config; }

    long getConfigItem(ConfigOption option);
    long getConfigItem(ConfigOption option, long id);
    bool setConfigItem(ConfigOption option, long value) override;
    bool setConfigItem(ConfigOption option, long id, long value) override;

    bool isMuted() { return config.volL == 0 && config.volR == 0; }

    double getSampleRate() { return sampleRate; }
    void setSampleRate(double hz);

private:
    
    void _dumpConfig() override;


    //
    // Analyzing
    //
    
public:
    
    // Returns information about the gathered statistical information
    MuxerStats getStats() { return stats; }
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker
        
        & config.samplingMethod
        & config.filterType
        & config.filterAlwaysOn
        & config.pan
        & config.vol
        & config.volL
        & config.volR
        & vol
        & volL
        & volR
        & pan;
    }
    
    template <class T>
    void applyToHardResetItems(T& worker)
    {
        /*
        worker
        
        & sampler
        & stream;
        */
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(u8 *buffer) override;
    
    
    //
    // Controlling the volume
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
    
    void synthesize(Cycle clock, Cycle target, long count);
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
    // Copying data
    //
    
public:
    
    void copyMono(float *buffer, size_t n);
    void copyStereo(float *left, float *right, size_t n);
    void copyInterleaved(float *buffer, size_t n);
};

#endif
