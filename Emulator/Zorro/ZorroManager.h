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
#include "RamExpansion.h"
#include "HdrController.h"

class ZorroManager : public SubComponent {
    
public:
    
    // A FastRam expansion board
    RamExpansion ramExpansion = RamExpansion(amiga);

    // Hard drive controller (based on the AmiEmu)
    // HdrController &hdrController; 

private:
    
    ZorroBoard *slots[3] = {
        
        &ramExpansion,
        &hdrController0,
        nullptr
    };
    
            
    //
    // Initializing
    //
    
public:
    
    ZorroManager(Amiga& ref);
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "ZorroManager"; }
    void _dump(dump::Category category, std::ostream& os) const override { }

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { RESET_SNAPSHOT_ITEMS(hard) }

    template <class T>
    void applyToPersistentItems(T& worker) { }
    template <class T>
    void applyToResetItems(T& worker, bool hard = true) { }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

        
    //
    // Accessing
    //
    
public:
    
    // Read a byte from autoconfig space
    u8 peekACF(u32 addr) const;
    u8 spypeekACF(u32 addr) const { return peekACF(addr); }
    
    // Write a byte into autoconfig space
    void poke(u32 addr, u8 value);
    
    // Asks all boards to update the memory map
    void updateMemSrcTables();
};
