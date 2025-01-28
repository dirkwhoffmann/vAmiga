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
#include "HdController.h"
#include "DiagBoard.h"

namespace vamiga {

class ZorroManager final : public SubComponent {
    
    Descriptions descriptions = {{

        .type           = Class::ZorroManager,
        .name           = "ZorroManager",
        .description    = "Zorro Manager",
        .shell          = "zorro"
    }};

    ConfigOptions options = {

    };

public:

    // Number of emulated Zorro slots
    static constexpr isize slotCount = 6;
    
private:
    
    ZorroBoard *slots[slotCount + 1] = {
        
        &ramExpansion,
        &hd0con,
        &hd1con,
        &hd2con,
        &hd3con,
        &diagBoard,
        nullptr
    };
    

    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;
    ZorroManager& operator= (const ZorroManager& other) { return *this; }

    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from CoreComponent
    //
    
private:
    
    template <class T>
    void serialize(T& worker) { } SERIALIZERS(serialize, override);

public:

    const Descriptions &getDescriptions() const override { return descriptions; }
    const ConfigOptions &getOptions() const override { return options; }

    //
    // Accessing
    //
    
public:
    
    // Returns the board in the specified slot
    ZorroBoard *getBoard(isize i) const;
    
    // Reads a value from Zorro board space
    u8 peek8(u32 addr);
    u16 peek16(u32 addr);
    u8 spypeek8(u32 addr) const;
    u16 spypeek16(u32 addr) const;

    // Writes a value into Zorro board space
    void poke8(u32 addr, u8 value);
    void poke16(u32 addr, u16 value);

    // Read a byte from autoconfig space
    u8 peekACF(u32 addr) const;
    u8 spypeekACF(u32 addr) const { return peekACF(addr); }
    
    // Write a byte into autoconfig space
    void pokeACF(u32 addr, u8 value);
    
    // Asks all boards to update the memory map
    void updateMemSrcTables();
    
private:
    
    // Returns the mapped in device for a given address
    ZorroBoard *mappedInDevice(u32 addr) const;
};

}
