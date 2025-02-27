// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraConfig.h"
#include "MoiraTypes.h"
#include "MoiraDebugger.h"
#include "SubComponent.h"

namespace vamiga::moira {

/** @brief Main CPU emulator class */
class Moira : public SubComponent {
    
    friend class FPU;
    friend class Debugger;
    friend class Breakpoints;
    friend class Watchpoints;
    friend class Catchpoints;
    
    
    //
    // Configuration
    //
    
protected:
    
    /** @brief Emulated CPU model. */
    Model cpuModel = Model::M68000;
    
    /** @brief Instruction set used by the disassembler. */
    Model dasmModel = Model::M68000;
    
    /** @brief Visual style for disassembled instructions. */
    DasmStyle instrStyle;
    
    /** @brief Visual style for data dumps. */
    DasmStyle dataStyle;
    
    
    //
    // Subcomponents
    //
    
public:
    
    /** @brief Debugger handling breakpoints, watchpoints, catchpoints, and instruction tracing. */
    Debugger debugger = Debugger(*this);
    
    
    //
    // Internals
    //
    
protected:
    
    /** @brief Number of elapsed cycles since power-up. */
    i64 clock {};
    
    /** @brief Register set of the CPU. */
    Registers reg {};
    
    /** @brief Prefetch queue for instruction fetching. */
    PrefetchQueue queue {};
    
    /** @brief Interrupt mode of this CPU. */
    IrqMode irqMode {IrqMode::AUTO};
    
    /** @brief Current value on the IPL (Interrupt Priority Level) pins. */
    u8 ipl {};
    
    /** @brief Value on the lower two function code pins (FC1|FC0). */
    u8 fcl {2};
    
    /** @brief Determines the source of the function code pins. */
    u8 fcSource {};
    
    /** @brief Remembers the vector number of the most recent exception. */
    int exception {};
    
    /** @brief Cycle penalty (for 68020+ extended addressing modes). */
    int cp {};
    
    /** @brief Controls exact timing of instructions running in loop mode (68010 only). */
    int loopModeDelay {2};
    
    /** @brief Read buffer (appears in 68010 exception frames). */
    u16 readBuffer {};
    
    /** @brief Write buffer (appears in 68010 exception frames). */
    u16 writeBuffer {};
    
    /** @brief State flags used internally. */
    int flags {};
    
    
    //
    // Lookup tables
    //
    
private:
    
    /** @brief Jump table holding the instruction handlers. */
    typedef void (Moira::*ExecPtr)(u16);
    ExecPtr *exec = nullptr;
    
    /** @brief Jump table holding the loop mode instruction handlers (68010 only). */
    ExecPtr *loop = nullptr;
    
    /** @brief Jump table holding the disassembler handlers. */
    typedef void (Moira::*DasmPtr)(StrWriter&, u32&, u16) const;
    DasmPtr *dasm = nullptr;
    
    /** @brief Table holding instruction information. */
    InstrInfo *info = nullptr;
    
    
    //
    // Constructing
    //
    
public:
    
    /** @brief Constructs and initializes a Moira instance. */
    Moira(Amiga &ref);
    
    /** @brief  Destroys the Moira instance. */
    virtual ~Moira();
    
protected:
    
    /**
     * @brief Creates or updates the jump tables for execution and disassembly.
     * @param cpuModel The CPU model for execution.
     * @param dasmModel The CPU model for disassembly.
     */
    void createJumpTable(Model cpuModel, Model dasmModel);
    void createJumpTable(Model model) { createJumpTable(model, model); }
    
private:
    
    /**
     * @brief Core routine for creating jump tables.
     * @tparam C The core type.
     * @param model The CPU model.
     * @param registerDasm Whether to register disassembly handlers.
     */
    template <Core C> void createJumpTable(Model model, bool registerDasm);
    
    
    //
    // Configuring
    //
    
public:
    
    /**
     * @brief Sets the emulated CPU model.
     * @param cpuModel The CPU model for execution.
     * @param dasmModel The CPU model for disassembly.
     */
    void setModel(Model cpuModel, Model dasmModel);
    void setModel(Model model) { setModel(model, model); }
    
    /**
     * @brief Configures the syntax style for disassembly output.
     * @param value The syntax style.
     */
    void setDasmSyntax(Syntax value);
    
    /**
     * @brief Sets the number format for disassembly output.
     * @param value The number format.
     */
    void setDasmNumberFormat(DasmNumberFormat value) { setNumberFormat(instrStyle, value); }
    
    /**
     * @brief Sets the letter case for disassembly output.
     * @param value The letter case format.
     */
    void setDasmLetterCase(LetterCase value);
    
    /**
     * @brief Sets the indentation for disassembly output.
     * @param value The indentation level.
     */
    void setDasmIndentation(int value) { instrStyle.tab = value; }
    
    /**
     * @brief Sets the number format for data dumps.
     * @param value The number format.
     */
    void setDumpNumberFormat(DasmNumberFormat value) { setNumberFormat(instrStyle, value); }
    
    /**
     * @brief Sets the indentation for data dumps.
     * @param value The indentation level.
     */
    void setDumpIndentation(int value) { dataStyle.tab = value; }
    
private:
    
    /**
     * @brief Helper function to configure the number format for disassembly and data dumps.
     * @param style The new syntax style.
     * @param value The new number format.
     */
    void setNumberFormat(DasmStyle &style, const DasmNumberFormat &value);
    
    
    //
    // Querying CPU properties
    //
    
public:
    
    /**
     * @brief Checks if the emulated CPU model has a coprocessor interface.
     * @return True if the CPU model includes a coprocessor interface, false otherwise.
     */
    bool hasCPI() const;
    
    /**
     * @brief Checks if the emulated CPU model has a memory management unit (MMU).
     * @return True if the CPU model includes an MMU, false otherwise.
     */
    bool hasMMU() const;
    
    /**
     * @brief Checks if the emulated CPU model has a floating-point unit (FPU).
     * @return True if the CPU model includes an FPU, false otherwise.
     */
    bool hasFPU() const;
    
    /**
     * @brief Returns the cache register mask, indicating the accessible CACR bits.
     * @return A 32-bit mask specifying which CACR bits are accessible.
     */
    u32 cacrMask() const;
    
    /**
     * @brief Returns the address bus mask, which defines the CPU's addressable memory range.
     * @return A 32-bit mask representing the bus width.
     */
    u32 addrMask() const;
    
protected:
    
    /**
     * @brief Returns the address bus mask for a specific CPU core type.
     * @tparam C The CPU core type.
     * @return A 32-bit mask representing the bus width.
     */
    template <Core C> u32 addrMask() const;
    
    
    //
    // Running the CPU
    //
    
public:
    
    /**
     * @brief Performs a hard reset, simulating a power-up sequence.
     */
    void reset();
    
    /**
     * @brief Executes the next instruction in the CPU pipeline.
     */
    void execute();
    
    /**
     * @brief Checks if the CPU is in a HALT state.
     * @return True if the CPU is halted, false otherwise.
     */
    bool isHalted() const { return flags & State::HALTED; }
    
private:
    
    /**
     * @brief Processes an exception that was caught during execution.
     * @param exception The exception that was caught.
     */
    void processException(const std::exception &exception);
    
    /**
     * @brief Processes an exception for a specific CPU core type.
     * @tparam C The CPU core type.
     * @param exception The exception that was caught.
     */
    template <Core C> void processException(const std::exception &exception);
    
    /**
     * @brief Performs a core-specific reset routine.
     * @tparam C The CPU core type.
     */
    template <Core C> void reset();
    
    /**
     * @brief Checks for a pending interrupt and handles it if necessary.
     * @return True if an interrupt was detected, false otherwise.
     */
    bool checkForIrq();
    
    /**
     * @brief Puts the CPU into a HALT state, stopping execution.
     */
    void halt();
    
    
    //
    // Running the disassembler
    //
    
public:
    
    /**
     * @brief Disassembles a single instruction and returns its size.
     * @param str Output buffer to store the disassembled instruction.
     * @param addr Address of the instruction to disassemble.
     * @return The size of the disassembled instruction in bytes.
     */
    int disassemble(char *str, u32 addr) const;
    
    /**
     * @brief Creates a textual representation of the status register.
     * @param str Output buffer to store the formatted status register.
     */
    void disassembleSR(char *str) const { disassembleSR(str, reg.sr); }
    
    /**
     * @brief Creates a textual representation of a given status register.
     * @param str Output buffer to store the formatted status register.
     * @param sr The status register to format.
     */
    void disassembleSR(char *str, const StatusRegister &sr) const;
    
    /**
     * @brief Converts an 8-bit value to a textual representation.
     * @param str Output buffer to store the formatted value.
     * @param value The 8-bit value to format.
     */
    void dump8(char *str, u8 value) const;
    
    /**
     * @brief Converts a 16-bit value to a textual representation.
     * @param str Output buffer to store the formatted value.
     * @param value The 16-bit value to format.
     */
    void dump16(char *str, u16 value) const;
    
    /**
     * @brief Converts a 24-bit value to a textual representation.
     * @param str Output buffer to store the formatted value.
     * @param value The 24-bit value to format.
     */
    void dump24(char *str, u32 value) const;
    
    /**
     * @brief Converts a 32-bit value to a textual representation.
     * @param str Output buffer to store the formatted value.
     * @param value The 32-bit value to format.
     */
    void dump32(char *str, u32 value) const;
    
    /**
     * @brief Converts multiple 16-bit values to a textual representation.
     * @param str Output buffer to store the formatted values.
     * @param values Array of 16-bit values to format.
     * @param cnt Number of values in the array.
     */
    void dump16(char *str, u16 values[], int cnt) const;
    
    /**
     * @brief Reads and converts multiple 16-bit values from memory.
     * @param str Output buffer to store the formatted values.
     * @param addr Memory address to read values from.
     * @param cnt Number of values to read and format.
     */
    void dump16(char *str, u32 addr, int cnt) const;
    
    /**
     * @brief Returns instruction metadata for a given opcode.
     * @param op The opcode to look up.
     * @return An InstrInfo struct containing details about the instruction.
     */
    InstrInfo getInstrInfo(u16 op) const;
    
    //
    // Interfacing with other components
    //
    
protected:
    
#if MOIRA_VIRTUAL_API == true
    
    /**
     * @brief Advances the internal clock by the specified number of cycles.
     * @param cycles Number of cycles to add to the clock.
     */
    virtual void sync(int cycles) { clock += cycles; }
    
    /**
     * @brief Reads an 8-bit value from memory.
     * @param addr The memory address to read from.
     * @return The 8-bit value at the specified address.
     */
    virtual u8 read8(u32 addr) const = 0;
    
    /**
     * @brief Reads a 16-bit value from memory.
     * @param addr The memory address to read from.
     * @return The 16-bit value at the specified address.
     */
    virtual u16 read16(u32 addr) const = 0;
    
    /**
     * @brief Reads a 16-bit value from memory during the reset routine.
     * @param addr The memory address to read from.
     * @return The 16-bit value at the specified address.
     */
    virtual u16 read16OnReset(u32 addr) const { return read16(addr); }
    
    /**
     * @brief Reads a 16-bit value from memory for disassembly purposes.
     * @param addr The memory address to read from.
     * @return The 16-bit value at the specified address.
     */
    virtual u16 read16Dasm(u32 addr) const { return read16(addr); }
    
    /**
     * @brief Writes an 8-bit value into memory.
     * @param addr The memory address to write to.
     * @param val The 8-bit value to write.
     */
    virtual void write8(u32 addr, u8 val) const = 0;
    
    /**
     * @brief Writes a 16-bit value into memory.
     * @param addr The memory address to write to.
     * @param val The 16-bit value to write.
     */
    virtual void write16(u32 addr, u16 val) const = 0;
    
    /**
     * @brief Provides the interrupt vector for a given interrupt level in USER mode.
     * @param level The interrupt level.
     * @return The corresponding interrupt vector (default: 0).
     */
    virtual u16 readIrqUserVector(u8 level) const { return 0; }
    
    //
    // State delegates
    //
    
    /**
     * @brief Called when the CPU is reset.
     */
    virtual void cpuDidReset() { }
    
    /**
     * @brief Called when the CPU enters the HALT state.
     */
    virtual void cpuDidHalt() { }
    
    //
    // Instruction delegates
    //
    
    /**
     * @brief Called before an instruction is executed.
     * @param func The name of the function handling the instruction.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @param opcode The raw opcode.
     */
    virtual void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode) { }
    
    /**
     * @brief Called after an instruction has been executed.
     * @param func The name of the function handling the instruction.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @param opcode The raw opcode.
     */
    virtual void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode) { }
    
    //
    // Exception delegates
    //
    
    /**
     * @brief Called before an exception is executed.
     * @param exc The type of exception.
     * @param vector The exception vector.
     */
    virtual void willExecute(ExceptionType exc, u16 vector) { }
    
    /**
     * @brief Called after an exception has been executed.
     * @param exc The type of exception.
     * @param vector The exception vector.
     */
    virtual void didExecute(ExceptionType exc, u16 vector) { }
    
    /**
     * @brief Called when an interrupt is about to be processed.
     * @param level The interrupt level.
     */
    virtual void willInterrupt(u8 level) { }
    
    /**
     * @brief Called when the CPU jumps to an exception vector.
     * @param nr The vector number.
     * @param addr The address of the exception handler.
     */
    virtual void didJumpToVector(int nr, u32 addr) { }
    
    //
    // Cache register delegates
    //
    
    /**
     * @brief Called when the CACR register is modified.
     * @param value The new CACR value.
     */
    virtual void didChangeCACR(u32 value) { }
    
    /**
     * @brief Called when the CAAR register is modified.
     * @param value The new CAAR value.
     */
    virtual void didChangeCAAR(u32 value) { }
    
    //
    // Debugger delegates
    //
    
    /**
     * @brief Called when a soft stop is reached.
     * @param addr The address of the stop.
     */
    virtual void didReachSoftstop(u32 addr) { }
    
    /**
     * @brief Called when a breakpoint is hit.
     * @param addr The address of the breakpoint.
     */
    virtual void didReachBreakpoint(u32 addr) { }
    
    /**
     * @brief Called when a watchpoint is hit.
     * @param addr The address of the watchpoint.
     */
    virtual void didReachWatchpoint(u32 addr) { }
    
    /**
     * @brief Called when a catchpoint is hit.
     * @param vector The exception vector associated with the catchpoint.
     */
    virtual void didReachCatchpoint(u8 vector) { }
    
    /**
     * @brief Called when a software trap is hit.
     * @param addr The address of the trap.
     */
    virtual void didReachSoftwareTrap(u32 addr) { }
    
#else
    
    /**
     * @brief Advances the internal clock by the specified number of cycles.
     * @param cycles Number of cycles to add to the clock.
     */
    void sync(int cycles);
    
    /**
     * @brief Reads an 8-bit value from memory.
     * @param addr The memory address to read from.
     * @return The 8-bit value at the specified address.
     */
    u8 read8(u32 addr) const;
    
    /**
     * @brief Reads a 16-bit value from memory.
     * @param addr The memory address to read from.
     * @return The 16-bit value at the specified address.
     */
    u16 read16(u32 addr) const;
    
    /**
     * @brief Reads a 16-bit value from memory during the reset routine.
     * @param addr The memory address to read from.
     * @return The 16-bit value at the specified address.
     */
    u16 read16OnReset(u32 addr) const;
    
    /**
     * @brief Reads a 16-bit value from memory for disassembly purposes.
     * @param addr The memory address to read from.
     * @return The 16-bit value at the specified address.
     */
    u16 read16Dasm(u32 addr) const;
    
    /**
     * @brief Writes an 8-bit value into memory.
     * @param addr The memory address to write to.
     * @param val The 8-bit value to write.
     */
    void write8(u32 addr, u8 val) const;
    
    /**
     * @brief Writes a 16-bit value into memory.
     * @param addr The memory address to write to.
     * @param val The 16-bit value to write.
     */
    void write16(u32 addr, u16 val) const;
    
    /**
     * @brief Provides the interrupt vector for a given interrupt level in USER mode.
     * @param level The interrupt level.
     * @return The corresponding interrupt vector (default: 0).
     */
    u16 readIrqUserVector(u8 level) const;
    
    //
    // State delegates
    //
    
    /**
     * @brief Called when the CPU is reset.
     */
    void cpuDidReset();
    
    /**
     * @brief Called when the CPU enters the HALT state.
     */
    void cpuDidHalt();
    
    //
    // Instruction delegates
    //
    
    /**
     * @brief Called before an instruction is executed.
     * @param func The name of the function handling the instruction.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @param opcode The raw opcode.
     */
    void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode);
    
    /**
     * @brief Called after an instruction has been executed.
     * @param func The name of the function handling the instruction.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @param opcode The raw opcode.
     */
    void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode);
    
    //
    // Exception delegates
    //
    
    /**
     * @brief Called before an exception is executed.
     * @param exc The type of exception.
     * @param vector The exception vector.
     */
    void willExecute(M68kException exc, u16 vector);
    
    /**
     * @brief Called after an exception has been executed.
     * @param exc The type of exception.
     * @param vector The exception vector.
     */
    void didExecute(M68kException exc, u16 vector);
    
    /**
     * @brief Called when an interrupt is about to be processed.
     * @param level The interrupt level.
     */
    void willInterrupt(u8 level);
    
    /**
     * @brief Called when the CPU jumps to an exception vector.
     * @param nr The vector number.
     * @param addr The address of the exception handler.
     */
    void didJumpToVector(int nr, u32 addr);
    
    //
    // Cache register delegates
    //
    
    /**
     * @brief Called when the CACR register is modified.
     * @param value The new CACR value.
     */
    void didChangeCACR(u32 value);
    
    /**
     * @brief Called when the CAAR register is modified.
     * @param value The new CAAR value.
     */
    void didChangeCAAR(u32 value);
    
    //
    // Debugger delegates
    //
    
    /**
     * @brief Called when a soft stop is reached.
     * @param addr The address of the stop.
     */
    void didReachSoftstop(u32 addr);
    
    /**
     * @brief Called when a breakpoint is hit.
     * @param addr The address of the breakpoint.
     */
    void didReachBreakpoint(u32 addr);
    
    /**
     * @brief Called when a watchpoint is hit.
     * @param addr The address of the watchpoint.
     */
    void didReachWatchpoint(u32 addr);
    
    /**
     * @brief Called when a catchpoint is hit.
     * @param vector The exception vector associated with the catchpoint.
     */
    void didReachCatchpoint(u8 vector);
    
    /**
     * @brief Called when a software trap is hit.
     * @param addr The address of the trap.
     */
    void didReachSoftwareTrap(u32 addr);
    
#endif
    
    //
    // Accessing the clock
    //
    
public:
    
    /**
     * @brief Retrieves the current CPU clock cycle count.
     * @return The number of elapsed cycles since power-up.
     */
    i64 getClock() const { return clock; }
    
    /**
     * @brief Sets the CPU clock cycle count.
     * @param val The new cycle count.
     */
    void setClock(i64 val) { clock = val; }
    
    
    //
    // Accessing registers
    //
    
public:
    
    /**
     * @brief Retrieves the value of a data register.
     * @param n The register index (0-7).
     * @return The value of the specified data register.
     */
    u32 getD(int n) const { return readD(n); }
    
    /**
     * @brief Sets the value of a data register.
     * @param n The register index (0-7).
     * @param v The new value for the data register.
     */
    void setD(int n, u32 v) { writeD(n, v); }
    
    /**
     * @brief Retrieves the value of an address register.
     * @param n The register index (0-7).
     * @return The value of the specified address register.
     */
    u32 getA(int n) const { return readA(n); }
    
    /**
     * @brief Sets the value of an address register.
     * @param n The register index (0-7).
     * @param v The new value for the address register.
     */
    void setA(int n, u32 v) { writeA(n, v); }
    
    /**
     * @brief Retrieves the current value of the program counter (PC).
     * @return The value of the PC register.
     */
    u32 getPC() const { return reg.pc; }
    
    /**
     * @brief Sets the program counter (PC) to a new value.
     * @param val The new PC value.
     */
    void setPC(u32 val) { reg.pc = val; }
    
    /**
     * @brief Retrieves the value of PC0 (previous program counter).
     * @return The value of the PC0 register.
     */
    u32 getPC0() const { return reg.pc0; }
    
    /**
     * @brief Sets the value of PC0.
     * @param val The new PC0 value.
     */
    void setPC0(u32 val) { reg.pc0 = val; }
    
    /**
     * @brief Retrieves the instruction register cache (IRC).
     * @return The value of the IRC register.
     */
    u16 getIRC() const { return queue.irc; }
    
    /**
     * @brief Sets the instruction register cache (IRC).
     * @param val The new value for the IRC register.
     */
    void setIRC(u16 val) { queue.irc = val; }
    
    /**
     * @brief Retrieves the instruction register decode (IRD).
     * @return The value of the IRD register.
     */
    u16 getIRD() const { return queue.ird; }
    
    /**
     * @brief Sets the instruction register decode (IRD).
     * @param val The new value for the IRD register.
     */
    void setIRD(u16 val) { queue.ird = val; }
    
    /**
     * @brief Retrieves the value of the Condition Code Register (CCR).
     * @return The CCR value.
     */
    u8 getCCR() const;
    
    /**
     * @brief Sets the value of the Condition Code Register (CCR).
     * @param val The new CCR value.
     */
    void setCCR(u8 val);
    
    /**
     * @brief Retrieves the value of the Status Register (SR).
     * @return The SR value.
     */
    u16 getSR() const;
    
    /**
     * @brief Sets the value of the Status Register (SR).
     * @param val The new SR value.
     */
    void setSR(u16 val);
    
    /**
     * @brief Retrieves the current value of the stack pointer (SP).
     * @return The value of the SP register.
     */
    u32 getSP() const { return reg.sp; }
    
    /**
     * @brief Sets the stack pointer (SP) to a new value.
     * @param val The new SP value.
     */
    void setSP(u32 val) { reg.sp = val; }
    
    /**
     * @brief Retrieves the User Stack Pointer (USP).
     * @return The value of the USP register.
     */
    u32 getUSP() const { return !reg.sr.s ? reg.sp : reg.usp; }
    
    /**
     * @brief Sets the User Stack Pointer (USP).
     * @param val The new USP value.
     */
    void setUSP(u32 val) { if (!reg.sr.s) reg.sp = val; else reg.usp = val; }
    
    /**
     * @brief Retrieves the Interrupt Stack Pointer (ISP).
     * @return The value of the ISP register.
     */
    u32 getISP() const { return (reg.sr.s && !reg.sr.m) ? reg.sp : reg.isp; }
    
    /**
     * @brief Sets the Interrupt Stack Pointer (ISP).
     * @param val The new ISP value.
     */
    void setISP(u32 val) { if (reg.sr.s && !reg.sr.m) reg.sp = val; else reg.isp = val; }
    
    /**
     * @brief Retrieves the Master Stack Pointer (MSP).
     * @return The value of the MSP register.
     */
    u32 getMSP() const { return (reg.sr.s && reg.sr.m) ? reg.sp : reg.msp; }
    
    /**
     * @brief Sets the Master Stack Pointer (MSP).
     * @param val The new MSP value.
     */
    void setMSP(u32 val) { if (reg.sr.s && reg.sr.m) reg.sp = val; else reg.msp = val; }
    
    /**
     * @brief Retrieves the Vector Base Register (VBR).
     * @return The value of the VBR register.
     */
    u32 getVBR() const { return reg.vbr; }
    
    /**
     * @brief Sets the Vector Base Register (VBR).
     * @param val The new VBR value.
     */
    void setVBR(u32 val) { reg.vbr = val; }
    
    /**
     * @brief Retrieves the Source Function Code (SFC).
     * @return The value of the SFC register.
     */
    u32 getSFC() const { return reg.sfc; }
    
    /**
     * @brief Sets the Source Function Code (SFC).
     * @param val The new SFC value.
     */
    void setSFC(u32 val) { reg.sfc = val & 0b111; }
    
    /**
     * @brief Retrieves the Destination Function Code (DFC).
     * @return The value of the DFC register.
     */
    u32 getDFC() const { return reg.dfc; }
    
    /**
     * @brief Sets the Destination Function Code (DFC).
     * @param val The new DFC value.
     */
    void setDFC(u32 val) { reg.dfc = val & 0b111; }
    
    /**
     * @brief Retrieves the Cache Control Register (CACR).
     * @return The value of the CACR register.
     */
    u32 getCACR() const { return reg.cacr; }
    
    /**
     * @brief Sets the Cache Control Register (CACR).
     * @param val The new CACR value.
     */
    void setCACR(u32 val);
    
    /**
     * @brief Retrieves the Cache Address Register (CAAR).
     * @return The value of the CAAR register.
     */
    u32 getCAAR() const { return reg.caar; }
    
    /**
     * @brief Sets the Cache Address Register (CAAR).
     * @param val The new CAAR value.
     */
    void setCAAR(u32 val);
    
    //
    // Supervisor mode
    //
    
public:
    
    /**
     * @brief Sets or clears supervisor mode.
     * @param value True to enable supervisor mode, false to disable it.
     */
    void setSupervisorMode(bool value);
    
    /**
     * @brief Sets or clears master mode.
     * @param value True to enable master mode, false to disable it.
     */
    void setMasterMode(bool value);
    
    /**
     * @brief Sets the supervisor and master flags.
     * @param s The supervisor flag.
     * @param m The master flag.
     */
    void setSupervisorFlags(bool s, bool m);
    
    
    //
    // Trace Flags
    //
    
private:
    
    /**
     * @brief Enables trace mode (T1 flag).
     */
    void setTraceFlag() { reg.sr.t1 = true; flags |= State::TRACING; }
    
    /**
     * @brief Disables trace mode (T1 flag).
     */
    void clearTraceFlag() { reg.sr.t1 = false; flags &= ~State::TRACING; }
    
    /**
     * @brief Enables trace mode (T0 flag).
     */
    void setTrace0Flag() { reg.sr.t0 = true; }
    
    /**
     * @brief Disables trace mode (T0 flag).
     */
    void clearTrace0Flag() { reg.sr.t0 = false; }
    
    /**
     * @brief Disables both trace flags (T0 and T1).
     */
    void clearTraceFlags() { clearTraceFlag(); clearTrace0Flag(); }
    
    
    //
    // Register Access
    //
    
protected:
    
    /**
     * @brief Reads a value from a data register.
     * @param n The register index (0-7).
     * @return The value stored in the data register.
     */
    template<Size S = Long> u32 readD(int n) const;
    
    /**
     * @brief Reads a value from an address register.
     * @param n The register index (0-7).
     * @return The value stored in the address register.
     */
    template <Size S = Long> u32 readA(int n) const;
    
    /**
     * @brief Reads a value from a general-purpose register.
     * @param n The register index.
     * @return The value stored in the register.
     */
    template <Size S = Long> u32 readR(int n) const;
    
    /**
     * @brief Writes a value to a data register.
     * @param n The register index (0-7).
     * @param v The value to store.
     */
    template <Size S = Long> void writeD(int n, u32 v);
    
    /**
     * @brief Writes a value to an address register.
     * @param n The register index (0-7).
     * @param v The value to store.
     */
    template <Size S = Long> void writeA(int n, u32 v);
    
    /**
     * @brief Writes a value to a general-purpose register.
     * @param n The register index.
     * @param v The value to store.
     */
    template <Size S = Long> void writeR(int n, u32 v);
    
    //
    // Instruction Analysis
    //
    
public:
    
    /**
     * @brief Returns the availability mask for a given instruction.
     * @param I The instruction type.
     * @return The availability mask.
     */
    u16 availabilityMask(Instr I) const;
    
    /**
     * @brief Returns the availability mask for an instruction with mode and size.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @return The availability mask.
     */
    u16 availabilityMask(Instr I, Mode M, Size S) const;
    
    /**
     * @brief Returns the availability mask for an instruction with extended parameters.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @param ext Additional extension word.
     * @return The availability mask.
     */
    u16 availabilityMask(Instr I, Mode M, Size S, u16 ext) const;
    
    /**
     * @brief Checks if a given CPU model supports a specific instruction.
     * @param model The CPU model.
     * @param I The instruction type.
     * @return True if the instruction is available, false otherwise.
     */
    bool isAvailable(Model model, Instr I) const;
    
    /**
     * @brief Checks if a given CPU model supports a specific instruction with mode and size.
     * @param model The CPU model.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @return True if the instruction is available, false otherwise.
     */
    bool isAvailable(Model model, Instr I, Mode M, Size S) const;
    
    /**
     * @brief Checks if a given CPU model supports a specific instruction with extended parameters.
     * @param model The CPU model.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @param ext Additional extension word.
     * @return True if the instruction is available, false otherwise.
     */
    bool isAvailable(Model model, Instr I, Mode M, Size S, u16 ext) const;
    
private:
    
    //
    // Extension Word Validation
    //
    
    /**
     * @brief Validates extension words for a given instruction.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param op The opcode.
     * @param ext The extension word.
     * @return True if the extension word is valid, false otherwise.
     */
    bool isValidExt(Instr I, Mode M, u16 op, u32 ext) const;
    
    /**
     * @brief Validates extension words for MMU-related instructions.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param op The opcode.
     * @param ext The extension word.
     * @return True if the extension word is valid, false otherwise.
     */
    bool isValidExtMMU(Instr I, Mode M, u16 op, u32 ext) const;
    
    /**
     * @brief Validates extension words for FPU-related instructions.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param op The opcode.
     * @param ext The extension word.
     * @return True if the extension word is valid, false otherwise.
     */
    bool isValidExtFPU(Instr I, Mode M, u16 op, u32 ext) const;
    
    
    //
    // Disassembler Support
    //
    
private:
    
    /**
     * @brief Returns an availability string for a given instruction.
     * @param I The instruction type.
     * @param M The addressing mode.
     * @param S The operand size.
     * @param ext The extension word.
     * @return A string representing instruction availability.
     */
    const char *availabilityString(Instr I, Mode M, Size S, u16 ext);
    
    //
    // Loop Mode Detection
    //
    
    /**
     * @brief Checks if an instruction is a loop mode instruction.
     * @tparam I The instruction type.
     * @return True if the instruction operates in loop mode, false otherwise.
     */
    template <Instr I>
    constexpr bool looping() { return I >= Instr::ABCD_LOOP && I <= Instr::TST_LOOP; }
    
    
    //
    // Managing the Function Code Pins
    //
    
public:
    
    /**
     * @brief Reads the current value of the function code pins.
     * @return The current function code (FC) value.
     */
    u8 readFC() const;
    
private:
    
    /**
     * @brief Sets the function code pins to a specific value.
     * @param value The function code to set.
     */
    void setFC(u8 value);
    
    /**
     * @brief Sets the function code pins based on the provided addressing mode.
     * @tparam M The addressing mode.
     */
    template <Mode M> void setFC();
    
    //
    // Handling Interrupts
    //
    
public:
    
    /**
     * @brief Gets the current Interrupt Priority Level (IPL) pins value.
     * @return The current IPL value.
     */
    u8 getIPL() const { return ipl; }
    
    /**
     * @brief Sets the Interrupt Priority Level (IPL) pins value.
     * @param val The IPL value to set.
     */
    void setIPL(u8 val);
    
private:
    
    /**
     * @brief Selects the IRQ vector to branch to based on the interrupt level.
     * @param level The interrupt level.
     * @return The IRQ vector number.
     */
    u16 getIrqVector(u8 level) const;
    
private:
    
#include "MoiraInit.h"
#include "MoiraALU.h"
#include "MoiraDataflow.h"
#include "MoiraExceptions.h"
#include "MoiraDasm.h"
};

}
