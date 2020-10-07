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
    
    // Sample rate in Hz
    double sampleRate = 0;
    
    // Master clock cycles per audio sample
    double cyclesPerSample = 0;

    
    //
    // Sub components
    //
    
public:

    // Input sampler
    Sampler sampler[4];

    // Audio filters
    AudioFilter filterL = AudioFilter(amiga);
    AudioFilter filterR = AudioFilter(amiga);
        
    
    //
    // Initializing
    //
    
public:
    
    Muxer(Amiga& ref);
    
    void _reset(bool hard) override;
    
    
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
    // Configuring
    //
    
public:

    /*
    AudioConfig getConfig() { return config; }

    long getConfigItem(ConfigOption option);
    bool setConfigItem(ConfigOption option, long value) override;
    */

private:
    
    /*
    void _inspect() override;
    void _dump() override;
    */
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        /*
        worker
        
        & config.samplingMethod
        & config.filterType
        & config.filterAlwaysOn
        & config.vol
        & config.pan
        & config.volL
        & config.volR;
        */
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
    
    double synthesize(double clock, Cycle target);
    
private:
    
    template <SamplingMethod method> double synthesize(double clock, Cycle target);
};

#endif
