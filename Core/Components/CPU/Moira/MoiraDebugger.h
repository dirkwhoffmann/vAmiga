// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"
#include "StrWriter.h"
#include <unordered_map>

namespace vamiga::moira {

//
// A single breakpoint, watchpoint, or catchpoint
//

struct Guard {

    // The observed address
    u32 addr = 0;

    // Disabled guards never trigger
    bool enabled = true;

    // Ignore counter
    isize ignore = 0;

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
    isize capacity = 1;

    // Array holding all guards
    Guard *guards = new Guard[1];

    // Number of currently stored guards
    isize count = 0;

public:

    // A copy of the latest match
    std::optional <Guard> hit;


    //
    // Constructing
    //

public:

    virtual ~Guards();


    //
    // Inspecting the guard list
    //

    isize elements() const { return count; }
    Guard *guardNr(isize nr) const;
    Guard *guardAt(u32 addr) const;

    std::optional<u32> guardAddr(isize nr) const;


    //
    // Adding or removing guards
    //

    bool isSet(isize nr) const { return guardNr(nr) != nullptr; }
    bool isSetAt(u32 addr) const { return guardAt(addr) != nullptr; }

    void setAt(u32 addr, isize ignores = 0);
    void replace(isize nr, u32 addr);

    void remove(isize nr);
    void removeAt(u32 addr);
    void removeAll() { count = 0; setNeedsCheck(false); }


    //
    // Enabling or disabling guards
    //

    bool isEnabled(isize nr) const;
    bool isEnabledAt(u32 addr) const;
    bool isDisabled(isize nr) const;
    bool isDisabledAt(u32 addr) const;

    void enable(isize nr) { setEnable(nr, true); }
    void enableAt(u32 addr) { setEnableAt(addr, true); }
    void enableAll() { setEnableAll(true); }
    void disable(isize nr) { setEnable(nr, false); }
    void disableAt(u32 addr) { setEnableAt(addr, false); }
    void disableAll() { setEnableAll(false); }
    void toggle(isize nr) { setEnable(nr, isDisabled(nr)); }
    void toggleAt(u32 addr) { setEnableAt(addr, isDisabledAt(addr)); }
    void setEnable(isize nr, bool val);
    void setEnableAt(u32 addr, bool val);
    void setEnableAll(bool val);

    void ignore(isize nr, isize count);


    //
    // Checking guards
    //

    // Indicates if guard checking is necessary
    virtual void setNeedsCheck(bool value) { };

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

    std::unordered_map<u16, SoftwareTrap>traps;

    // Creates a new software trap for a given instruction
    u16 create(u16 instr);
    u16 create(u16 key, u16 instr);

    // Replaces a software trap by its original opcode
    u16 resolve(u16 instr) const;
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
    isize logCnt = 0;


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
    int loggedInstructions() const;

    // Logs an instruction
    void logInstruction();

    /* Reads an item from the log buffer
     *
     *    xxxRel: n == 0 returns the most recently recorded entry
     *    xxxAbs: n == 0 returns the oldest entry
     */
    const Registers &logEntryRel(int n) const;
    const Registers &logEntryAbs(int n) const;

    // Clears the log buffer
    void clearLog() { logCnt = 0; }


    //
    // Changing state
    //

    // Continues program execution at the specified address
    void jump(u32 addr);
};

}
