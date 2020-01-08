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
    u32 addr;

    // Indicates if this guard is enabled
    bool enabled;

    // Hit counter
    long hits;

    // Number of skipped hits before a match is being recognized
    long skip;

public:

    // Returns true if the guard hits
    bool eval(u32 addr);

};

class GuardCollection {

    // Capacity of the guards array
    long capacity = 2;

    // Array holding all guards
    Guard *guards = new Guard[2];

    // Number of currently stored guards
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
    void add(uint32_t addr, long skip = 0);
    void removeAt(uint32_t addr);
    void removeAll() { count = 0; }

    //
    // Enabling or disabling guards
    //

    bool isEnabled(long nr);
    bool isDisabled(long nr) { return !isEnabled(nr); }
    void setEnable(long nr, bool val);
    void enable(long nr, bool val) { setEnable(nr, true); }
    void disable(long nr, bool val) { setEnable(nr, false); }
    void setEnableAt(uint32_t addr, bool val);
    void enableGuardAt(uint32_t addr);
    void disableGuardAt(uint32_t addr);

    //
    // Checking a guard
    //

    bool eval(u32 addr);
};

class Observer {

public:

    // Breakpoint storage
    GuardCollection breakpoints;

    // Watchpoint storage
    GuardCollection watchpoints;

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
};

}
#endif
