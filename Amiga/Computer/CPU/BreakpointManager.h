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
    long breakpointCount() { return breakpoints.size(); }
    
    // Returns an iterator pointing to the nth element.
    map<uint32_t, Breakpoint>::iterator breakpoint(long nr);
    
    bool hasBreakpointAt(uint32_t addr);
    bool hasConditionalBreakpointAt(uint32_t addr);
    void addBreakpointAt(uint32_t addr);
    void deleteBreakpointAt(uint32_t addr);
    void toggleBreakpointAt(uint32_t addr);
    
    // The following functions call the corresponding function on the
    // breakpoint at the specified location.
    bool hasCondition(long nr) {
        auto i = breakpoint(nr);
        return i == breakpoints.end() ? false : (*i).second.hasCondition();
    }
    
    bool hasSyntaxError(long nr) {
        auto i = breakpoint(nr);
        return i == breakpoints.end() ? false : (*i).second.hasSyntaxError();
    }
    
    const char *getCondition(long nr) {
        auto i = breakpoint(nr);
        return i == breakpoints.end() ? "" : (*i).second.getCondition();
    }
    
    bool setCondition(long nr, const char *str) {
        auto i = breakpoint(nr);
        return i == breakpoints.end() ? false : (*i).second.setCondition(str);
    }
    
    bool removeCondition(long nr) {
        auto i = breakpoint(nr);
        return i == breakpoints.end() ? false : (*i).second.removeCondition();
    }
    
};

#endif
