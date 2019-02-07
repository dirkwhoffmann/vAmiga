// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _BREAKPOINT_MANAGER_INC
#define _BREAKPOINT_MANAGER_INC

#include "HardwareComponent.h"
#include "Breakpoint.h"
#include <map>

/*
#include "va_std.h"
#include <string>
#include <vector>
*/

// This class manages a container that stores all breakpoints.
class BreakpointManager : public HardwareComponent {
    
private:
    
    map<uint32_t, Breakpoint> breakpoints;
    
public:
    
    // Returns the number of stored breakpoints
    long numberOfBreakpoints() { return breakpoints.size(); }
    
    // Returns an iterator pointing to the nth element.
    map<uint32_t, Breakpoint>::iterator breakpoint(long nr);
    
    bool hasBreakpointAt(uint32_t addr);
    bool hasConditionalBreakpointAt(uint32_t addr);
    void addBreakpointAt(uint32_t addr);
    void deleteBreakpointAt(uint32_t addr);
    void toggleBreakpointAt(uint32_t addr);
    
    // Gets or sets a property of a breakpoint in a specific slot
    uint32_t getAddr(long nr);
    bool setAddr(long nr, uint32_t addr);
    bool hasCondition(long nr);
    bool hasSyntaxError(long nr);
    const char *getCondition(long nr);
    bool setCondition(long nr, const char *str);
    bool removeCondition(long nr);
};

#endif
