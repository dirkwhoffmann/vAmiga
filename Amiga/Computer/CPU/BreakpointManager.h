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

#include "SubComponent.h"
#include "Breakpoint.h"

// This class manages a container that stores all breakpoints.
class BreakpointManager : public SubComponent {
    
private:
    
    // A list containing all set breakpoints
    static const int maxBreakpoints = 32;
    Breakpoint *breakpoints[maxBreakpoints];
    
    // The number of breakpoints stored in the breakpoint list
    long numBreakpoints;
    
    /* Soft breakpoint for implementing single-stepping.
     * In contrast to a standard (hard) breakpoint, a soft breakpoint is
     * deleted when reached. The CPU halts if softStop matches the CPU's
     * program counter (used to implement "step over") or if softStop equals
     * UINT32_MAX (used to implement "step into"). To disable soft stopping,
     * simply set softStop to an unreachable memory location such as
     * UINT32_MAX - 1.
     */
    uint32_t softStop = UINT32_MAX - 1;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    BreakpointManager(Amiga &ref);


    //
    // Methods from HardwareComponent
    //

private:

    void _reset() override { }
    size_t _size() override { return 0; }
    size_t _load(uint8_t *buffer) override { return 0; }
    size_t _save(uint8_t *buffer) override { return 0; }

    
    //
    // Accessing properties
    //
    
public:
    
    // Returns the number of currently set breakpoints
    long numberOfBreakpoints() { return numBreakpoints; }
    
    // Returns the breakpoint with the specified number or NULL.
    Breakpoint *breakpointWithNr(long nr);
    
    // Returns the breakpoint at the specified address or NULL.
    Breakpoint *breakpointAtAddr(uint32_t addr);

    // Returns an iterator pointing to the nth element. (DEPRECATED)
    // map<uint32_t, Breakpoint>::iterator breakpoint(long nr);
    
    
    //
    // Checking breakpoints
    //

    bool hasBreakpointAt(uint32_t addr);
    bool hasDisabledBreakpointAt(uint32_t addr);
    bool hasConditionalBreakpointAt(uint32_t addr);

    // Returns true if the emulator has reached a breakpoint
    bool shouldStop();
    
    //
    // Setting and deleting breakpoints
    //

    void setBreakpointAt(uint32_t addr);
    void _setBreakpointAt(uint32_t addr);
    void setSoftBreakpointAt(uint32_t addr) { softStop = addr; }
    void deleteBreakpoint(long nr);
    void deleteBreakpointAt(uint32_t addr);
    void deleteAllBreakpoints();
    

    
    //
    // Enabling or disabling breakpoints
    //
    
    bool isDisabled(long nr);
    void setEnableAt(uint32_t addr, bool value);
    void enableBreakpointAt(uint32_t addr) { setEnableAt(addr, true); }
    void disableBreakpointAt(uint32_t addr) { setEnableAt(addr, false); }

    
    //
    // Modifying a breakpoint in a certain slot
    //
    
    uint32_t getAddr(long nr);
    bool setAddr(long nr, uint32_t addr);
    
    bool hasCondition(long nr);
    const char *getCondition(long nr);
    bool setCondition(long nr, const char *str);
    bool deleteCondition(long nr);

    bool hasSyntaxError(long nr);
};

#endif
