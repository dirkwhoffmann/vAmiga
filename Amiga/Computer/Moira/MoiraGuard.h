// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef MOIRA_GUARD_H
#define MOIRA_GUARD_H

namespace moira {

struct Guard {

    // Observed memory address
    u32 addr = UINT32_MAX;

    // Indicates if this guard is enabled
    bool enabled = true;

    // Hit counter
    long hits = 0;

    // Number of skipped hits before a match is being recognized
    long skip = 0;

public:

    // Returns true if this is a coditional guard
    // bool hasCondition() { return false; }

    // Evaluates the guard condition
    // bool eval() { return true; }

    // Performs a check
    bool matches(u32 addr); 

};

template <int Capacity>
class GuardCollection {

    // Array holding all guards
    Guard guards[Capacity];

    // The number of currently stored guards
    long count = 0;

public:

    //
    // Inspecting the guard list
    //

    long elements() { return count; }
    Guard *guardWithNr(long nr);
    Guard *guardAtAddr(u32 addr);

    u32 guardAddr(long nr) { return nr < count ? guards[nr].addr : 0; }

    bool hasGuardAt(u32 addr);
    bool hasDisabledGuardAt(u32 addr);
    bool hasConditionalGuardAt(u32 addr);

    //
    // Adding or removing guards
    //

    void remove(long nr);
    void setAt(uint32_t addr, long skip = 0);
    void removeAt(uint32_t addr);
    void removeAll() { count = 0; }

    //
    // Enabling or disabling guards
    //

    // bool isDisabled(long nr);
    void setEnableAt(uint32_t addr, bool value);
    void enableGuardAt(uint32_t addr);
    void disableGuardAt(uint32_t addr);

    //
    // Checking a guard
    //

    bool matches(u32 addr); 
};

class Observer {

public:

    // Breakpoint storage
    GuardCollection<MAX_BREAKPOINTS> breakpoints;

    // Watchpoint storage
    GuardCollection<MAX_WATCHPOINTS> watchpoints;

private:
    
    /* Soft breakpoint for implementing single-stepping.
     * In contrast to a standard (hard) breakpoint, a soft breakpoint is
     * deleted when reached. The CPU halts if softStop matches the CPU's
     * program counter (used to implement "step over") or if softStop equals
     * UINT64_MAX (used to implement "step into"). To disable soft stopping,
     * simply set softStop to an unreachable memory location such as
     * UINT64_MAX - 1.
     */
    u64 softStop = UINT64_MAX - 1;


    //
    // Constructing and destructing
    //

public:

    Observer();


    //
    // Checking breakpoints
    //

    bool breakpointMatches(u32 pc);
    bool watchpointMatches(u32 addr);

    /*
    bool hasBreakpointAt(uint32_t addr);
    bool hasDisabledBreakpointAt(uint32_t addr);
    bool hasConditionalBreakpointAt(uint32_t addr);
    */

    // Returns true if the emulator has reached a breakpoint
    // bool shouldStop();

    //
    // Setting and deleting breakpoints
    //

    /*
    void setBreakpointAt(uint32_t addr);
    void _setBreakpointAt(uint32_t addr);
    void setSoftBreakpointAt(uint32_t addr) { softStop = addr; }
    void deleteBreakpoint(long nr);
    void deleteBreakpointAt(uint32_t addr);
    void deleteAllBreakpoints();
    */


    //
    // Enabling or disabling breakpoints
    //

    /*
    bool isDisabled(long nr);
    void setEnableAt(uint32_t addr, bool value);
    void enableBreakpointAt(uint32_t addr) { setEnableAt(addr, true); }
    void disableBreakpointAt(uint32_t addr) { setEnableAt(addr, false); }
    */


    //
    // Modifying a breakpoint in a certain slot
    //

    /*
    uint32_t getAddr(long nr);
    bool setAddr(long nr, uint32_t addr);

    bool hasCondition(long nr);
    const char *getCondition(long nr);
    bool setCondition(long nr, const char *str);
    bool deleteCondition(long nr);

    bool hasSyntaxError(long nr);
    */
};

}
#endif
