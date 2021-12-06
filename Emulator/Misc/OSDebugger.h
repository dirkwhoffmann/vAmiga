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
#include "Constants.h"
#include "AmigaTypes.h"

class OSDebugger : public SubComponent {
    
private:
    
    
    //
    // Constructing
    //
    
public:
    
    using SubComponent::SubComponent;
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "OSDebugger"; }
    void _dump(dump::Category category, std::ostream& os) const override { }

    
    //
    // Methods from AmigaComponent
    //
        
private:
    
    void _reset(bool hard) override { };
    
    
    //
    // Serializing
    //
    
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override { return 0; }
    isize _save(u8 *buffer) override { return 0; }
    
    
    //
    //
    //

public:

};
