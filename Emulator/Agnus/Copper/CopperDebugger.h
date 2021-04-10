// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"
#include <map>

struct CopperList {
    
    u32 start;
    u32 end;
};

class CopperDebugger: public AmigaComponent {
                
    // Cached Copper lists
    std::map<u32, CopperList> cache;
        
    // The currently executed Copper list
    CopperList *current = nullptr;
    
    
    //
    // Initializing
    //
    
public:
    
    using AmigaComponent::AmigaComponent;
    // CopperDebugger(Amiga& ref);
    const char *getDescription() const override { return "CopperDebugger"; }
    void _reset(bool hard) override;
    
    
    //
    // Analyzing
    //

private:

    void _dump(dump::Category category, std::ostream& os) const override;
    
    
    //
    // Serialization
    //
    
private:
    /*
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }
    */
    
    isize _size() override { return 0; }
    isize _load(const u8 *buffer) override { return 0; }
    isize _save(u8 *buffer) override { return 0; }
    

    //
    // Tracking the Copper
    //
    
public:
    
    // Returns the start or end address of the currently processed Copper list
    u32 startOfCopperList();
    u32 endOfCopperList();

    // Notifies the debugger that the Copper has advanced the program counter
    void advanced();
    
    // Notifies the debugger that the Copper has jumped to a new Copper list
    void jumped();
};
