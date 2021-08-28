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
#include "Event.h"
#include "EventHandlerTypes.h"

class EventManager : public SubComponent {

public:
    
    // Event slots
    Event slot[SLOT_COUNT];
    
    // Next trigger cycle
    Cycle nextTrigger = NEVER;
    
    
    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;
        
    
    //
    // Methods From AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "EventManager"; }
    void _dump(dump::Category category, std::ostream& os) const override { };
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

};
