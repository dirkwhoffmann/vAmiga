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

    
    //
    // Sub components
    //
    
public:

    // Inputs
    Sampler sampler[4];

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
    
    void _reset(bool hard) override;
    
    // Resets the output buffer and the two audio filters
    void clear();

    
    //
    // Configuring
    //
    
public:
    
    MuxerConfig getConfig() { return config; }

    long getConfigItem(ConfigOption option);
    bool setConfigItem(ConfigOption option, long value) override;
            
    bool isMuted() { return config.volL == 0 && config.volR == 0; }

    double getSampleRate() { return sampleRate; }
    void setSampleRate(double hz);
    

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
        & config.vol
        & config.pan
        & config.volL
        & config.volR;
    }
    
    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
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
    void ignoreNextUnderOrOverflow() { lastAlignment = mach_absolute_time(); }


    //
    // Copying data
    //
    
public:
    
    void copy(float *left, float *right, size_t n,
              i32 &volume, i32 targetVolume, i32 volumeDelta);
    
    void copyMono(float *buffer, size_t n,
                  i32 &volume, i32 targetVolume, i32 volumeDelta);
    
};

#endif
