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
    
    // Container storing all breakpoints
    map<uint32_t, Breakpoint> breakpoints;

    /* Soft breakpoint for implementing single-stepping.
     * In contrast to standard (hard) breakpoints, a soft breakpoint is
     * deleted when reached.
     */
    uint32_t softStop = UINT32_MAX;
    
public:
    
    // Returns the number of stored breakpoints
    long numberOfBreakpoints() { return breakpoints.size(); }
    
    // Returns an iterator pointing to the nth element.
    map<uint32_t, Breakpoint>::iterator breakpoint(long nr);
    
    bool hasBreakpointAt(uint32_t addr);
    bool hasDisabledBreakpointAt(uint32_t addr);
    bool hasConditionalBreakpointAt(uint32_t addr);
    void setBreakpointAt(uint32_t addr);
    void deleteBreakpointAt(uint32_t addr);
    void enableBreakpointAt(uint32_t addr);
    void disableBreakpointAt(uint32_t addr);
    
    // void enableOrDisableBreakpointAt(uint32_t addr);
    // void deleteBreakpointAt(uint32_t addr);
    // void toggleBreakpointAt(uint32_t addr);
    
    // Gets or sets a property of a breakpoint in a specific slot
    void deleteBreakpoint(long nr);
    uint32_t getAddr(long nr);
    bool setAddr(long nr, uint32_t addr);
    bool isDisabled(long nr);
    bool hasCondition(long nr);
    bool hasSyntaxError(long nr);
    const char *getCondition(long nr);
    bool setCondition(long nr, const char *str);
    bool removeCondition(long nr);
    
    // Returns true if the emulator has reached a breakpoint
    bool shouldStop();
};

#endif
