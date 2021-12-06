// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "OSDebuggerTypes.h"
#include "SubComponent.h"
#include "Constants.h"

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
    // Extracting structures from Amiga memory
    //
    
public:
            
    // Reads an integer value in big endian format from Amiga memory
    void read(u32 addr, u8 *result) const;
    void read(u32 addr, u16 *result) const;
    void read(u32 addr, u32 *result) const;
    void read(u32 addr, i8 *result) const { read(addr, (u8 *)result); }
    void read(u32 addr, i16 *result) const { read(addr, (u16 *)result); }
    void read(u32 addr, i32 *result) const { read(addr, (u32 *)result); }
    
    // Reads an AmigaOS structure from Amiga memory
    void read(u32 addr, os::Node *result) const;
    void read(u32 addr, os::Library *result) const;
    void read(u32 addr, os::IntVector *result) const;
    void read(u32 addr, os::List *result) const;
    void read(u32 addr, os::MinList *result) const;
    void read(u32 addr, os::SoftIntList *result) const;
    void read(u32 addr, os::ExecBase *result) const;


    //
    // Dumping system information
    //
    
    void dumpExecBase(std::ostream& os) const;
};
