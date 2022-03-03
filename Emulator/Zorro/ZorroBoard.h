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
    
    // Checks whether the board is plugged in
    virtual bool pluggedIn() const = 0;
    
    // Returns product information
    virtual isize pages() const = 0;
    virtual u8 type() const = 0;
    virtual u8 product() const = 0;
    virtual u8 flags() const = 0;
    virtual u16 manufacturer() const = 0;
    virtual u32 serialNumber() const = 0;
    virtual u16 initDiagVec() const = 0;
    virtual string vendorName() const = 0;
    virtual string productName() const = 0;
    virtual string revisionName() const = 0;
    
    
    //
    // Configuring (AutoConfig)
    //
    
private:

    u8 peekACF8(u32 addr) const;
    u8 spypeekACF8(u32 addr) const { return peekACF8(addr); }
    void pokeACF8(u32 addr, u8 value);
        
    // Reads a single byte from configuration space
    u8 getDescriptorByte(isize offset) const;

    
    //
    // Accessing
    //
    
private:
    
    virtual u8 peek8(u32 addr) { fatalError; }
    virtual u16 peek16(u32 addr) { fatalError; }
    virtual u8 spypeek8(u32 addr) const { fatalError; }
    virtual u16 spypeek16(u32 addr) const { fatalError; }
    virtual void poke8(u32 addr, u8 value) { fatalError; }
    virtual void poke16(u32 addr, u16 value) { fatalError; }


    //
    // Querying the memory map
    //
    
public:
    
    // Returns the first or the last page this device is mapped to
    isize firstPage() const { return baseAddr / 0x10000; }
    isize lastPage() const { return firstPage() + pages() - 1; }
    
    // Checks if the specified address belongs to this device
    bool mappedIn(u32 addr);
    
    
    //
    // Changing state
    //
    
private:
    
    // Called when autoconfig is complete
    virtual void activate();
    
    // Called when the board is supposed to shut up by software
    virtual void shutup();
    
    // Updates the current memory map
    virtual void updateMemSrcTables() { };
};
