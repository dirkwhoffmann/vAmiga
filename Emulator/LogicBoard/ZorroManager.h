// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"

/* Additional information:
 *
 *   Fast Ram emulation (Zorro II) is based on:
 *   github.com/PR77/A500_ACCEL_RAM_IDE-Rev-1/blob/master/Logic/RAM/A500_RAM.v
 */

class ZorroManager : public SubComponent {
    
    // Current configuration state (0 = unconfigured)
    u8 fastRamConf;
    
    // Fast Ram base address (provided by Kickstart)
    u32 fastRamBaseAddr;
    
    
    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "Zorro"; }
    void _dump(dump::Category category, std::ostream& os) const override { }

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { RESET_SNAPSHOT_ITEMS(hard) }

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        worker

        << fastRamConf
        << fastRamBaseAddr;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

        
    //
    // Emulating Fast Ram
    //
    
public:
    
    u8 peekFastRamDevice(u32 addr) const;
    u8 spypeekFastRamDevice(u32 addr) const;
    void pokeFastRamDevice(u32 addr, u8 value);
};
