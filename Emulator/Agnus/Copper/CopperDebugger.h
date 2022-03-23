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
#include "MoiraDebugger.h"
#include "CPU.h" // REMOVE AFTER REMOVING Moira from constructor
#include <map>

struct CopperList {
    
    u32 start;
    u32 end;
};

class CopperBreakpoints : public moira::Guards {
    
public:
    
    class Copper &copper;
    CopperBreakpoints(moira::Moira& cpu, Copper& ref) : moira::Guards(cpu), copper(ref) { }
    void setNeedsCheck(bool value) override;
};

class CopperDebugger: public SubComponent {
               
    friend class Amiga;
    friend class Copper;
    
    // Cached Copper lists
    std::map<u32, CopperList> cache;
        
    // The most recently used Copper list 1
    CopperList *current1 = nullptr;

    // The most recently used Copper list 2
    CopperList *current2 = nullptr;
    
    // The breakpoint list
    CopperBreakpoints breakpoints = CopperBreakpoints(cpu, copper);
    
    
    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;


    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "CopperDebugger"; }
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //

private:
    
    void _reset(bool hard) override;
    
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override { return 0; }
    isize _save(u8 *buffer) override { return 0; }
    

    //
    // Tracking the Copper
    //
    
public:
    
    // Returns the start or end address of the currently processed Copper list
    u32 startOfCopperList(isize nr) const;
    u32 endOfCopperList(isize nr) const;

    // Notifies the debugger that the Copper has advanced the program counter
    void advanced();
    
    // Notifies the debugger that the Copper has jumped to a new Copper list
    void jumped();
    
    
    //
    // Disassembling instructions
    //
    
    // Disassembles a single Copper command
    string disassemble(isize list, isize offset, bool symbolic) const;
    string disassemble(u32 addr, bool symbolic) const;
    

    //
    // Manages the breakpoint list
    //

    void setBreakpoint(u32 addr) throws;
    void deleteBreakpoint(isize nr) throws;
    void enableBreakpoint(isize nr) throws;
    void disableBreakpoint(isize nr) throws;
    void ignoreBreakpoint(isize nr, isize count) throws;
};
