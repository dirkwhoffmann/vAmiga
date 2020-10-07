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

template <int capacity>
class Muxer : public AmigaComponent {

public:
    
    Sampler<capacity> sampler[4];
    
    
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
};

#endif
