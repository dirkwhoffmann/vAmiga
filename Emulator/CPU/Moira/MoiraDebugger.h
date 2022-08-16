// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"
#include "StrWriter.h"
#include <map>

namespace moira {

//
// A single breakpoint, watchpoint, or catchpoint
//

struct Guard {
    
    // The observed address
    u32 addr = 0;
    
    // Disabled guards never trigger
    bool enabled = true;
    
    // Ignore counter
    long ignore = 0;
    
public:
    
    // Returns true if the guard hits
    bool eval(u32 addr, Size S = Byte);
    
};


//
// A collection of breakpoints, watchpoints, or catchpoints
//

class Guards {
    
    friend class Debugger;
    
protected:
    
    // Capacity of the guards array
    long capacity = 1;
    
    // Array holding all guards
    Guard *guards = new Guard[1];
    
    // Number of currently stored guards
    long count = 0;
    
public:
    
    // A copy of the latest match
    std::optional <Guard> hit;
    
    
    //
    // Constructing
    //
    
public:
    
    // Guards(Moira& ref) : moira(ref) { }
    virtual ~Guards();
    
    
    //
    // Inspecting the guard list
    //
    
    long elements() const { return count; }
    Guard *guardNr(long nr) const;
    Guard *guardAt(u32 addr) const;
    
    std::optional<u32> guardAddr(long nr) const;
    
    
    //
    // Adding or removing guards
    //
    
    bool isSet(long nr) const { return guardNr(nr) != nullptr; }
    bool isSetAt(u32 addr) const { return guardAt(addr) != nullptr; }
    
    void setAt(u32 addr);
    
    void remove(long nr);
    void removeAt(u32 addr);
    void removeAll() { count = 0; setNeedsCheck(false); }
    
    void replace(long nr, u32 addr);
    
    
    //
    // Enabling or disabling guards
    //
    
    bool isEnabled(long nr) const;
    bool isEnabledAt(u32 addr) const;
    bool isDisabled(long nr) const;
    bool isDisabledAt(u32 addr) const;
    
    void enable(long nr) { setEnable(nr, true); }
    void enableAt(u32 addr) { setEnableAt(addr, true); }
    void disable(long nr) { setEnable(nr, false); }
    void disableAt(u32 addr) { setEnableAt(addr, false); }
    void setEnable(long nr, bool val);
    void setEnableAt(u32 addr, bool val);
    
    void ignore(long nr, long count);
    
    
    //
    // Checking guards
    //
    
    // Indicates if guard checking is necessary
    virtual void setNeedsCheck(bool value) = 0;
    
    // Evaluates all guards
    bool eval(u32 addr, Size S = Byte);
};

class Breakpoints : public Guards {
    
    class Moira &moira;
    
public:
    
    Breakpoints(Moira& ref) : moira(ref) { }
    void setNeedsCheck(bool value) override;
};

class Watchpoints : public Guards {
    
    class Moira &moira;
    
public:
    
    Watchpoints(Moira& ref) : moira(ref) { }
    void setNeedsCheck(bool value) override;
};

class Catchpoints : public Guards {
    
    class Moira &moira;
    
public:
    
    Catchpoints(Moira& ref) : moira(ref) { }
    void setNeedsCheck(bool value) override;
};


//
// Software traps
//

struct SoftwareTrap {
    
    // The original instruction that has been replaced by this trap
    u16 instruction;
};

struct SoftwareTraps {
    
    std::map<u16, SoftwareTrap>traps;
    
    // Creates a new software trap for a given instruction
    u16 create(u16 instr);
    u16 create(u16 key, u16 instr);
    
    // Replaces a software trap by its original opcode
    u16 resolve(u16 instr);
};

class Debugger {
    
public:
    
    // Reference to the connected CPU
    class Moira &moira;
    
    // Breakpoints, watchpoints, and catchpoints
    Breakpoints breakpoints = Breakpoints(moira);
    Watchpoints watchpoints = Watchpoints(moira);
    Catchpoints catchpoints = Catchpoints(moira);
    
    // Software traps
    SoftwareTraps swTraps;
    
private:
    
    /* Soft breakpoint for implementing single-stepping. In contrast to a
     * standard (hard) breakpoint, a soft breakpoint is deleted when reached.
     * If a softStop is set, the CPU halts if it matches the program counter
     * (used to implement "step over") or if it contains a negative value (used
     * to implement "step into").
     */
    std::optional <i64> softStop;
    
    // Buffer storing logged instructions
    static const int logBufferCapacity = 256;
    Registers logBuffer[logBufferCapacity];
    
    // Logging counter
    long logCnt = 0;
    
    
    //
    // Constructing
    //
    
public:
    
    Debugger(Moira& ref) : moira(ref) { }
    
    void reset();
    
    
    //
    // Analyzing instructions
    //
    
    static bool isLineAInstr(u16 opcode) { return (opcode & 0xF000) == 0xA000; }
    static bool isLineFInstr(u16 opcode) { return (opcode & 0xF000) == 0xF000; }
    
    
    //
    // Providing textual descriptions
    //
    
    // Returns a human-readable name for an exception vector
    static std::string vectorName(u8 vector);
    
    
    //
    // Working with breakpoints, watchpoints, and catchpoints
    //
    
    // Sets a soft breakpoint that will trigger immediately
    void stepInto();
    
    // Sets a soft breakpoint to the next instruction
    void stepOver();
    
    // Checks whether a debug events should be triggered
    bool softstopMatches(u32 addr);
    bool breakpointMatches(u32 addr);
    bool watchpointMatches(u32 addr, Size S);
    bool catchpointMatches(u32 vectorNr);
    
    
    //
    // Working with the log buffer
    //
    
    // Turns instruction logging on or off
    void enableLogging();
    void disableLogging();
    
    // Returns the number of logged instructions
    int loggedInstructions();
    
    // Logs an instruction
    void logInstruction();
    
    /* Reads an item from the log buffer
     *
     *    xxxRel: n == 0 returns the most recently recorded entry
     *    xxxAbs: n == 0 returns the oldest entry
     */
    Registers &logEntryRel(int n);
    Registers &logEntryAbs(int n);
    
    // Clears the log buffer
    void clearLog() { logCnt = 0; }
    
    
    //
    // Changing state
    //
    
    // Continues program execution at the specified address
    void jump(u32 addr);
};

}
