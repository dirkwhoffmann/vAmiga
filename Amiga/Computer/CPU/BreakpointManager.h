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

// This class manages a container that stores all breakpoints.
class BreakpointManager : public HardwareComponent {
    
private:
    
    // Container storing all breakpoints
    map<uint32_t, Breakpoint> breakpoints;

    /* Soft breakpoint for implementing single-stepping.
     * In contrast to a standard (hard) breakpoint, a soft breakpoint is
     * deleted when reached. The CPU halts if softStop matches the CPU's
     * program counter (used to implement "step over") or if softStop equals
     * UINT32_MAX (used to implement "step into"). To disable soft stopping,
     * simply set softStop to an unreachable memory location such as
     * UINT32_MAX - 1.
     */
    uint32_t softStop = UINT32_MAX - 1;
    
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
    
    void setSoftBreakpointAt(uint32_t addr) { softStop = addr; }

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
