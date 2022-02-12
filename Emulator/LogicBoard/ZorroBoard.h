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
#include "ZorroBoardTypes.h"

class ZorroBoard : public SubComponent {
    
    friend class ZorroManager;
    
protected:
    
    // Current state
    BoardState state;
    
    
    //
    // Constructing
    //
    
public:
    
    using SubComponent::SubComponent;
    
    
    //
    // Methods from AmigaObject
    //
    
protected:
        
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Methods from AmigaComponent
    //

    /*
private:
    
    void _reset(bool hard) override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {

    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << state;
        }
    }
    
    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    */
    
    //
    // Querying
    //
    
    // Informs about certain properties of this board
    virtual const BoardDescriptor &getDescriptor() const = 0;
    
    // Reads a single byte from the descriptor
    u8 getDescriptorByte(isize offset) const;
    
    
    //
    // Accessing
    //
    
    virtual u8 peek8(u32 addr) const;
    virtual u8 spypeek8(u32 addr) const { return peek8(addr); }
    virtual void poke8(u32 addr, u8 value) = 0;
    
    
    //
    // Changing state
    //
    
    // Called when autoconfig is complete
    virtual void activate();
    
    // Called when the board is supposed to shut up by software
    virtual void shutup();
    
    // Updates the current memory map
    virtual void updateMemSrcTables() { };
};
