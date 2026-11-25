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
#include <ostream>
#include <utility>

namespace MOIRA_NAMESPACE {

//
// A single breakpoint, watchpoint, or catchpoint
//

/** @brief A single breakpoint, watchpoint, or catchpoint
 *
 * A guard observes a single address. For breakpoints and watchpoints, this is
 * a memory address. For catchpoints, it is an exception vector number.
 */
struct Guard {

    /// @brief The observed address (or the vector number for catchpoints)
    u32 addr = 0;

    /// @brief Disabled guards never trigger
    bool enabled = true;

    /// @brief The number of hits that are skipped before the guard triggers
    isize ignore = 0;

public:

    /** @brief Checks if the guard hits
     *
     * A guard hits if it is enabled and the accessed range covers the
     * observed address. If an ignore counter is set, the hit is skipped and
     * the counter is decremented instead.
     *
     * @param addr  The first accessed address
     * @param S     The size of the access (Byte, Word or Long)
     * @return true if the guard triggers
     */
    bool eval(u32 addr, Size S = Byte);
};


//
// A collection of breakpoints, watchpoints, or catchpoints
//

/** @brief A collection of breakpoints, watchpoints, or catchpoints
 *
 * Guards are identified either by their index in the list (functions taking
 * an `nr` argument) or by the address they observe (functions ending in `At`).
 * A list never contains two guards that observe the same address. Removing a
 * guard shifts the indices of all guards that follow it.
 *
 * The list is the base class of Breakpoints, Watchpoints and Catchpoints.
 * Guards do not stop the CPU by themselves. When one hits, the CPU calls the
 * matching delegate (didReachBreakpoint, didReachWatchpoint or
 * didReachCatchpoint) and leaves it to the application to react.
 */
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

    /// @brief A copy of the guard that hit most recently
    std::optional <Guard> hit;


    //
    // Constructing
    //

public:

    /// @brief Destroys the guard list
    virtual ~Guards();


    //
    // Inspecting the guard list
    //

    /// @brief Returns the number of guards in the list
    isize elements() const { return count; }

    /** @brief Returns the guard with the given index
     *  @param nr  The index (0 ... elements() - 1)
     *  @return A pointer to the guard, or nullptr if the index is out of range.
     *          The pointer is invalidated when guards are added or removed.
     */
    Guard *guardNr(isize nr) const;

    /** @brief Returns the guard observing the given address
     *  @return A pointer to the guard, or nullptr if there is none.
     *          The pointer is invalidated when guards are added or removed.
     */
    Guard *guardAt(u32 addr) const;

    /** @brief Returns the address observed by the guard with the given index
     *  @return The address, or an empty value if the index is out of range
     */
    std::optional<u32> guardAddr(isize nr) const;


    //
    // Adding or removing guards
    //

    /// @brief Checks if a guard with the given index exists
    bool isSet(isize nr) const { return guardNr(nr) != nullptr; }

    /// @brief Checks if a guard observes the given address
    bool isSetAt(u32 addr) const { return guardAt(addr) != nullptr; }

    /** @brief Adds a guard that observes the given address
     *
     * The new guard is appended to the list and enabled. The call has no
     * effect if a guard for this address exists already.
     *
     * @param addr     The address to observe
     * @param ignores  The number of hits that are skipped before the guard triggers
     */
    void setAt(u32 addr, isize ignores = 0);

    /** @brief Changes the address observed by the guard with the given index
     *
     * The call has no effect if the index is out of range or if another
     * guard observes the new address already.
     */
    void replace(isize nr, u32 addr);

    /// @brief Removes the guard with the given index (no effect if it does not exist)
    void remove(isize nr);

    /// @brief Removes the guard observing the given address (no effect if it does not exist)
    void removeAt(u32 addr);

    /// @brief Removes all guards
    void removeAll() { count = 0; setNeedsCheck(false); }


    //
    // Enabling or disabling guards
    //

    /// @brief Checks if the guard with the given index exists and is enabled
    bool isEnabled(isize nr) const;
    /// @brief Checks if a guard observes the given address and is enabled
    bool isEnabledAt(u32 addr) const;
    /// @brief Checks if the guard with the given index exists and is disabled
    bool isDisabled(isize nr) const;
    /// @brief Checks if a guard observes the given address and is disabled
    bool isDisabledAt(u32 addr) const;

    /// Enables the guard with the given index
    void enable(isize nr) { setEnable(nr, true); }
    /// Enables the guard observing the given address
    void enableAt(u32 addr) { setEnableAt(addr, true); }
    /// Enables all guards
    void enableAll() { setEnableAll(true); }
    /// Disables the guard with the given index
    void disable(isize nr) { setEnable(nr, false); }
    /// Disables the guard observing the given address
    void disableAt(u32 addr) { setEnableAt(addr, false); }
    /// Disables all guards
    void disableAll() { setEnableAll(false); }
    /// Toggles the guard with the given index
    void toggle(isize nr) { setEnable(nr, isDisabled(nr)); }
    /// Toggles the guard observing the given address
    void toggleAt(u32 addr) { setEnableAt(addr, isDisabledAt(addr)); }
    /// Enables or disables the guard with the given index
    void setEnable(isize nr, bool val);
    /// Enables or disables the guard observing the given address
    void setEnableAt(u32 addr, bool val);
    /// Enables or disables all guards
    void setEnableAll(bool val);

    /** @brief Makes the guard with the given index skip the next hits
     *  @param nr     The index of the guard
     *  @param count  The number of hits to skip
     */
    void ignore(isize nr, isize count);


    //
    // Checking guards
    //

    /** @brief Informs the CPU whether the list has to be checked
     *
     * The list calls this function whenever it becomes empty or non-empty.
     * Subclasses use it to set or clear the corresponding check flag of the
     * CPU, so that an empty list costs no time during emulation.
     */
    virtual void setNeedsCheck(bool value) { };

    /** @brief Evaluates all guards
     *
     * If a guard hits, a copy of it is stored in `hit`.
     *
     * @param addr  The first accessed address
     * @param S     The size of the access
     * @return true if at least one guard hits
     */
    bool eval(u32 addr, Size S = Byte);
};

/** @brief The list of breakpoints
 *
 * A breakpoint hits when the CPU is about to execute the instruction at the
 * observed address. The CPU then calls didReachBreakpoint().
 */
class Breakpoints : public Guards {

    class Moira &moira;

public:

    Breakpoints(Moira& ref) : moira(ref) { }
    void setNeedsCheck(bool value) override;
};

/** @brief The list of watchpoints
 *
 * A watchpoint hits when the CPU reads or writes a byte, word or long word
 * that covers the observed address. The CPU then calls didReachWatchpoint().
 */
class Watchpoints : public Guards {

    class Moira &moira;

public:

    Watchpoints(Moira& ref) : moira(ref) { }
    void setNeedsCheck(bool value) override;
};

/** @brief The list of catchpoints
 *
 * A catchpoint observes an exception vector number instead of a memory
 * address. It hits when the CPU takes the exception with that vector number.
 * The CPU then calls didReachCatchpoint().
 */
class Catchpoints : public Guards {

    class Moira &moira;

public:

    Catchpoints(Moira& ref) : moira(ref) { }
    void setNeedsCheck(bool value) override;
};


//
// Software traps
//

/// @brief A software trap
struct SoftwareTrap {

    /// @brief The original instruction that has been replaced by this trap
    u16 instruction;
};

/** @brief A table of software traps
 *
 * A software trap replaces an instruction by a special line A opcode (0xA000 -
 * 0xAFFF). The original instruction is remembered and can be recovered with
 * resolve(). When the CPU executes a trap opcode, it restores the original
 * instruction in the prefetch queue and calls didReachSoftwareTrap(). This
 * way, an application can intercept the execution of code (e.g., to hook a
 * ROM routine) without a breakpoint. The trap stays in place and fires again
 * the next time the opcode is executed.
 *
 * To install a trap, write the opcode returned by create() into memory at the
 * location of the instruction.
 */
struct SoftwareTraps {

    /// @brief The installed traps, indexed by their opcode
    std::unordered_map<u16, SoftwareTrap>traps;

    /** @brief Creates a new software trap for a given instruction
     *
     * The function picks the first unused line A opcode.
     *
     * @param instr  The instruction (opcode) that the trap replaces
     * @return The trap opcode
     */
    u16 create(u16 instr);

    /** @brief Creates a software trap with a given opcode for a given instruction
     *
     * @param key    The trap opcode (must be a line A opcode that is not in use)
     * @param instr  The instruction (opcode) that the trap replaces
     * @return The trap opcode (identical to `key`)
     */
    u16 create(u16 key, u16 instr);

    /** @brief Replaces a software trap by its original opcode
     *
     * @param instr  An opcode
     * @return The original instruction if `instr` is a trap opcode,
     *         otherwise `instr` itself.
     */
    u16 resolve(u16 instr) const;
};

/** @brief The debugger of the CPU
 *
 * Each CPU owns a debugger (see Moira::debugger). It manages breakpoints,
 * watchpoints, catchpoints and software traps, implements single-stepping and
 * records the most recently executed instructions.
 *
 * The debugger does not stop the CPU. It reports events via delegates
 * (didReachBreakpoint, didReachWatchpoint, didReachCatchpoint,
 * didReachSoftstop, didReachSoftwareTrap). A typical application sets a flag in
 * the delegate and leaves its emulation loop afterwards.
 *
 * ```cpp
 * cpu.debugger.breakpoints.setAt(0xFC00D2);   // Break at this address
 * cpu.debugger.enableLogging();               // Record executed instructions
 * ...
 * cpu.debugger.dumpLogBuffer(std::cout, 10);  // Print the last 10 instructions
 * ```
 */
class Debugger {

public:

    /// @brief Reference to the connected CPU
    class Moira &moira;

    /// @brief The breakpoint list
    Breakpoints breakpoints = Breakpoints(moira);

    /// @brief The watchpoint list
    Watchpoints watchpoints = Watchpoints(moira);

    /// @brief The catchpoint list
    Catchpoints catchpoints = Catchpoints(moira);

    /// @brief The software traps
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

    /// @brief Creates a debugger for a CPU
    Debugger(Moira& ref) : moira(ref) { }

    /** @brief Resets the debugger
     *
     * The CPU calls this function when it is reset. The guard lists are kept,
     * only the check flags of the CPU are brought in sync with them.
     */
    void reset();


    //
    // Analyzing instructions
    //

    /// @brief Checks if an opcode belongs to the line A instruction group (0xA000 - 0xAFFF)
    static bool isLineAInstr(u16 opcode) { return (opcode & 0xF000) == 0xA000; }
    /// @brief Checks if an opcode belongs to the line F instruction group (0xF000 - 0xFFFF)
    static bool isLineFInstr(u16 opcode) { return (opcode & 0xF000) == 0xF000; }


    //
    // Providing textual descriptions
    //

    /** @brief Returns a human-readable name for an exception vector
     *
     * @param vector  The vector number (0 - 255)
     * @return A description such as "Bus error", "Trap #3" or "Level 2 interrupt"
     */
    static std::string vectorName(u8 vector);


    //
    // Working with breakpoints, watchpoints, and catchpoints
    //

    /** @brief Stops before the next instruction is executed
     *
     * Sets a soft stop that triggers immediately. After the current
     * instruction has finished, the CPU calls didReachSoftstop() before it
     * executes the next one. A soft stop is removed when it is reached.
     */
    void stepInto();

    /** @brief Stops at the instruction that follows the current one
     *
     * Sets a soft stop at the address behind the instruction at the program
     * counter. In contrast to stepInto(), subroutine calls are executed as a
     * whole, because the soft stop only triggers when the program counter
     * reaches the address behind the call. A soft stop is removed when it is
     * reached.
     */
    void stepOver();

    /// @brief Checks whether the soft stop matches the given address (removes the soft stop if so)
    bool softstopMatches(u32 addr);

    /// @brief Checks whether a breakpoint is set at the given address
    bool breakpointMatches(u32 addr);

    /// @brief Checks whether a watchpoint observes an access of the given size
    bool watchpointMatches(u32 addr, Size S);

    /// @brief Checks whether a catchpoint is set for the given exception vector
    bool catchpointMatches(u32 vectorNr);


    //
    // Working with the log buffer
    //

    /** @brief Turns instruction logging on
     *
     * While logging is on, the register state is recorded before each
     * instruction is executed. The log buffer is a ring buffer that holds the
     * last 256 entries.
     */
    void enableLogging();

    /// @brief Turns instruction logging off
    void disableLogging();

    /// @brief Returns the number of logged instructions (at most 256)
    isize loggedInstructions() const;

    /** @brief Logs an instruction
     *
     * The CPU calls this function automatically if logging is enabled. It
     * records the current registers and calls didLogInstruction().
     */
    void logInstruction();

    /** @brief Reads an item from the log buffer, counting from the newest entry
     *  @param n  The entry number. 0 is the most recently recorded entry.
     *            Must be smaller than loggedInstructions().
     */
    const Registers &logEntryRel(isize n) const;

    /** @brief Reads an item from the log buffer, counting from the oldest entry
     *  @param n  The entry number. 0 is the oldest entry. Must be smaller than
     *            loggedInstructions().
     */
    const Registers &logEntryAbs(isize n) const;

    /// @brief Clears the log buffer
    void clearLog() { logCnt = 0; }

    /** Disassembles an instruction from the log buffer
     *
     * Works like Moira::disassemble, but prints the register values recorded
     * when the instruction was logged.
     *
     * @param fmt  The format string (see Moira::disassemble)
     * @param nr   The index of the log entry (0 is the oldest entry)
     * @param len  If provided, receives the length of the instruction in bytes
     */
    std::string disassembleRecorded(const std::string &fmt, isize nr, int *len = nullptr) const;

    /** @brief Prints the most recent entries of the log buffer
     *
     * The entries are printed oldest first, one per line.
     *
     * @param os     The output stream
     * @param count  The number of entries to print
     * @param fmt    The format string (see Moira::disassemble)
     */
    void dumpLogBuffer(std::ostream &os, isize count, const std::string &fmt = "%p  %f  %i") const;

    /** @brief Prints the whole log buffer
     *  @param os   The output stream
     *  @param fmt  The format string (see Moira::disassemble)
     */
    void dumpLogBuffer(std::ostream &os, const std::string &fmt = "%p  %f  %i") const;


    //
    // Disassembling memory
    //

    /** @brief Disassembles a number of instructions starting at the given address
     *
     * Each instruction is printed on a line of its own. The instruction at
     * the program counter is marked with "->", and addresses with a
     * breakpoint are marked with "B" (enabled) or "b" (disabled).
     *
     * @param os     The output stream
     * @param addr   The address of the first instruction
     * @param count  The number of instructions to disassemble
     * @param fmt    The format string (see Moira::disassemble)
     */
    void disassembleRange(std::ostream &os, u32 addr, isize count, const std::string &fmt = "%p  %15b   %i") const;

    /** @brief Disassembles an address range
     *
     * The markers are the same as in the overload above.
     *
     * @param os     The output stream
     * @param range  The first and the last address of the range
     * @param max    Stops after the given number of instructions
     * @param fmt    The format string (see Moira::disassemble)
     */
    void disassembleRange(std::ostream &os, std::pair<u32, u32> range, isize max = 255, const std::string &fmt = "%p  %15b   %i") const;


    //
    // Changing state
    //

    /** @brief Continues program execution at the specified address
     *
     * This is a shortcut for Moira::jump().
     *
     * @param addr  The address of the next instruction
     */
    void jump(u32 addr);
};

}
