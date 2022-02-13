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
    
    // Base address of this device (assigned by Kickstart after configuring)
    u32 baseAddr = 0;
    
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
    // Querying
    //
    
public:
    
    // Returns basic board properties
    virtual u8 type() const { return 0; }
    virtual u8 product() const { return 0; }
    virtual u8 flags() const { return 0; }
    virtual u16 manufacturer() const { return 0; }
    virtual u32 serialNumber() const { return 0; }
    virtual u16 initDiagVec() const { return 0; }
    
private:
    
    // Reads a single byte from configuration space
    u8 getDescriptorByte(isize offset) const;
    
    
    //
    // Accessing Autoconfig space
    //
    
    u8 peekAutoconf8(u32 addr) const;
    u8 spypeekAutoconf8(u32 addr) const { return peekAutoconf8(addr); }
    void pokeAutoconf8(u32 addr, u8 value);
    
    
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
