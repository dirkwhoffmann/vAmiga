// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "CPU.h"
#include "MoiraDebugger.h"
#include <map>

namespace vamiga {

struct CopperList {
    
    u32 start;
    u32 end;
};

class CopperBreakpoints : public GuardList {

    class Copper &copper;

public:
    
    CopperBreakpoints(Copper& ref);
    void setNeedsCheck(bool value) override;
};

class CopperWatchpoints : public GuardList {

    class Copper &copper;

public:
    
    CopperWatchpoints(Copper& ref);
    void setNeedsCheck(bool value) override;
};

class CopperDebugger final : public SubComponent {

    Descriptions descriptions = {{

        .type           = CopperDebuggerClass,
        .name           = "cdebugger",
        .description    = "Copper Debugger",
        .shell          = ""
    }};

    ConfigOptions options = {

    };

    friend class Amiga;
    friend class Copper;
    
    // Cached Copper lists
    std::map<u32, CopperList> cache;

    // The most recently used Copper list 1
    CopperList *current1 = nullptr;

    // The most recently used Copper list 2
    CopperList *current2 = nullptr;
    
public:
    
    // Breakpoint and watchpoints
    CopperBreakpoints breakpoints = CopperBreakpoints(copper);
    CopperWatchpoints watchpoints = CopperWatchpoints(copper);

    
    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;

    
    //
    // Methods from Serializable
    //

private:
        
    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream& os) const override;
    void _didReset(bool hard) override;


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


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
};

}
