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
    
    // Emulated CPU model
    Model cpuModel = Model::M68000;
    
    // Instruction set used by the disassembler
    Model dasmModel = Model::M68000;
    
    // Visual style for disassembled instructions
    DasmStyle instrStyle;
    
    // Visual style for data dumps
    DasmStyle dataStyle;
    
    
    //
    // Subcomponents
    //
    
public:
    
    // Debugger handling breakpoints, watchpoints, catchpoints, and instruction tracing
    Debugger debugger = Debugger(*this);
    
    
    //
    // Internals
    //
    
protected:
    
    // Number of elapsed cycles since power-up
    i64 clock {};
    
    // The CPU's register set
    Registers reg {};
    
    // Prefetch queue for fetching instructions
    PrefetchQueue queue {};
    
    // Interrupt mode
    IrqMode irqMode {IrqMode::AUTO};
    
    // Current value on the IPL (Interrupt Priority Level) pins
    u8 ipl {};
    
    // Value on the lower two function code pins (FC1|FC0)
    u8 fcl {2};
    
    // Source of the function code pins
    u8 fcSource {};
    
    // Remembers the vector number of the most recent exception
    int exception {};
    
    // Cycle penalty (for 68020+ extended addressing modes)
    int cp {};
    
    // Controls exact timing of instructions running in loop mode (68010 only)
    int loopModeDelay {2};
    
    // Read buffer (appears in 68010 exception frames)
    u16 readBuffer {};
    
    // Write buffer (appears in 68010 exception frames)
    u16 writeBuffer {};
    
    // State flags used internally
    int flags {};
    
    
    //
    // Lookup tables
    //
    
private:
    
    // Jump table holding the instruction handlers
    typedef void (Moira::*ExecPtr)(u16);
    ExecPtr *exec = nullptr;
    
    // Jump table holding the loop mode instruction handlers (68010 only)
    ExecPtr *loop = nullptr;
    
    // Jump table holding the disassembler handlers
    typedef void (Moira::*DasmPtr)(StrWriter&, u32&, u16) const;
    DasmPtr *dasm = nullptr;
    
    // Table holding instruction information
    InstrInfo *info = nullptr;
    
    
    //
    // Constructing
    //
    
public:
    
    // Constructs and initializes a Moira instance
    Moira(Amiga &ref);
    
    //  Destroys the Moira instance
    virtual ~Moira();
    
protected:
    
    // Creates or updates the jump tables for execution and disassembly
    void createJumpTable(Model cpuModel, Model dasmModel);
    void createJumpTable(Model model) { createJumpTable(model, model); }
    
private:
    
    // Core routine for creating jump tables
    template <Core C> void createJumpTable(Model model, bool registerDasm);
    
    
    //
    // Configuring
    //
    
public:
    
    // Sets the emulated CPU models
    void setModel(Model cpuModel, Model dasmModel);
    void setModel(Model model) { setModel(model, model); }
    
    // Configures the syntax style for disassembly output
    void setDasmSyntax(Syntax value);
    
    // Sets the number format for disassembly output
    void setDasmNumberFormat(DasmNumberFormat value) { setNumberFormat(instrStyle, value); }
    
    // Sets the letter case for disassembly output
    void setDasmLetterCase(LetterCase value);
    
    // Sets the indentation for disassembly output
    void setDasmIndentation(int value) { instrStyle.tab = value; }
    
    // Sets the number format for data dumps
    void setDumpNumberFormat(DasmNumberFormat value) { setNumberFormat(instrStyle, value); }
    
    // Sets the indentation for data dumps
    void setDumpIndentation(int value) { dataStyle.tab = value; }
    
private:
    
    // Helper function
    void setNumberFormat(DasmStyle &style, const DasmNumberFormat &value);
    
    
    //
    // Querying CPU properties
    //
    
public:
    
    // Checks if the emulated CPU model has a coprocessor interface
    bool hasCPI() const;
    
    // Checks if the emulated CPU model has a memory management unit (MMU)
    bool hasMMU() const;
    
    // Checks if the emulated CPU model has a floating-point unit (FPU)
    bool hasFPU() const;
    
    // Returns the cache register mask, indicating the accessible CACR bits
    u32 cacrMask() const;
    
    // Returns the address bus mask, which defines the CPU's addressable memory range
    u32 addrMask() const;
    
protected:
    
    // Returns the address bus mask for a specific CPU core type
    template <Core C> u32 addrMask() const;
    
    
    //
    // Running the CPU
    //
    
public:
    
    // Performs a hard reset, simulating the native power-up sequence
    void reset();
    
    // Executes a single instruction
    void execute();

    // Executes instructions for the given number of cycles.
    //
    // Note: Since the emulartor cannot stop in the middle of an instruction,
    // the number of actually elapsed cycles may exceed the specified cycle
    // budget.
    void execute(i64 cycles);
    
    // Executes instructions until a specific cycle count is reached.
    void executeUntil(i64 cycle);
    
    // Checks if the CPU is in a HALT state
    bool isHalted() const { return flags & State::HALTED; }
    
private:
    
    // Processes an exception that was caught during execution
    void processException(const std::exception &exception);
    
    // Processes an exception for a specific CPU core type
    template <Core C> void processException(const std::exception &exception);
    
    // Performs a core-specific reset routine
    template <Core C> void reset();
    
    // Checks for a pending interrupt and handles it if necessary
    bool checkForIrq();
    
    // Puts the CPU into a HALT state, stopping execution
    void halt();
    
    
    //
    // Running the disassembler
    //
    
public:
    
    // Disassembles an instruction and returns the instruction size in bytes
    int disassemble(char *str, u32 addr) const;
    
    // Creates a textual representation of the status register
    void disassembleSR(char *str) const { disassembleSR(str, reg.sr); }
    
    // Creates a textual representation of a given status register
    void disassembleSR(char *str, const StatusRegister &sr) const;
    
    // Converts an integer value to a textual representation
    void dump8(char *str, u8 value) const;
    void dump16(char *str, u16 value) const;
    void dump24(char *str, u32 value) const;
    void dump32(char *str, u32 value) const;
    
    // Converts multiple 16-bit values to a textual representation
    void dump16(char *str, u16 values[], int cnt) const;
    void dump16(char *str, u32 addr, int cnt) const;
    
    // Returns instruction metadata for a given opcode
    InstrInfo getInstrInfo(u16 op) const;

    
    //
    // Interfacing with other components
    //
    
protected:
    
#if MOIRA_VIRTUAL_API == true
    
    // Advances the internal clock by the specified number of cycles
    virtual void sync(int cycles) { clock += cycles; }
    
    // Reads a value from memory
    virtual u8 read8(u32 addr) const = 0;
    virtual u16 read16(u32 addr) const = 0;
    
    // Reads a 16-bit value from memory during the reset routine
    virtual u16 read16OnReset(u32 addr) const { return read16(addr); }
    
    // Reads a 16-bit value from memory for disassembly purposes
    virtual u16 read16Dasm(u32 addr) const { return read16(addr); }
    
    // Writes a value into memory
    virtual void write8(u32 addr, u8 val) const = 0;
    virtual void write16(u32 addr, u16 val) const = 0;
    
    // Provides the interrupt vector for a given interrupt level in USER mode
    virtual u16 readIrqUserVector(u8 level) const { return 0; }

    
    //
    // State delegates
    //
    
    // Called when the CPU is reset
    virtual void cpuDidReset() { }
    
    // Called when the CPU enters the HALT state
    virtual void cpuDidHalt() { }
    
    
    //
    // Instruction delegates
    //
    
    // Called before an instruction is executed
    virtual void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode) { }
    
    // Called after an instruction has been executed
    virtual void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode) { }
    
    
    //
    // Exception delegates
    //
    
    // Called before an exception is executed
    virtual void willExecute(M68kException exc, u16 vector) { }
    
    // Called after an exception has been executed
    virtual void didExecute(M68kException exc, u16 vector) { }
    
    // Called when an interrupt is about to be processed
    virtual void willInterrupt(u8 level) { }
    
    // Called when the CPU jumps to an exception vector
    virtual void didJumpToVector(int nr, u32 addr) { }

    
    //
    // Cache register delegates
    //
    
    // Called when the CACR register is modified
    virtual void didChangeCACR(u32 value) { }
    
    // Called when the CAAR register is modified
    virtual void didChangeCAAR(u32 value) { }

    
    //
    // Debugger delegates
    //
    
    // Called when a soft stop is reached
    virtual void didReachSoftstop(u32 addr) { }
    
    // Called when a breakpoint is hit
    virtual void didReachBreakpoint(u32 addr) { }
    
    // Called when a watchpoint is hit
    virtual void didReachWatchpoint(u32 addr) { }
    
    // Called when a catchpoint is hit
    virtual void didReachCatchpoint(u8 vector) { }
    
    // Called when a software trap is hit
    virtual void didReachSoftwareTrap(u32 addr) { }
    
#else
    
    // Advances the internal clock by the specified number of cycles
    void sync(int cycles);
    
    // Reads a value from memory
    u8 read8(u32 addr) const;
    u16 read16(u32 addr) const;
    
    // Reads a 16-bit value from memory during the reset routine
    u16 read16OnReset(u32 addr) const;
    
    // Reads a 16-bit value from memory for disassembly purposes
    u16 read16Dasm(u32 addr) const;
    
    // Writes a value into memory
    void write8(u32 addr, u8 val) const;
    void write16(u32 addr, u16 val) const;
    
    // Provides the interrupt vector for a given interrupt level in USER mode
    u16 readIrqUserVector(u8 level) const;

    
    //
    // State delegates
    //
    
    // Called when the CPU is reset
    void cpuDidReset();
    
    // Called when the CPU enters the HALT state
    void cpuDidHalt();
    
    
    //
    // Instruction delegates
    //
    
    // Called before an instruction is executed
    void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode);
    
    // Called after an instruction has been executed
    void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode);
    
    
    //
    // Exception delegates
    //
    
    // Called before an exception is executed
    void willExecute(M68kException exc, u16 vector);
    
    // Called after an exception has been executed
    void didExecute(M68kException exc, u16 vector);
    
    // Called when an interrupt is about to be processed
    void willInterrupt(u8 level);
    
    // Called when the CPU jumps to an exception vector
    void didJumpToVector(int nr, u32 addr);
    
    
    //
    // Cache register delegates
    //
    
    // Called when the CACR register is modified
    void didChangeCACR(u32 value);
    
    // Called when the CAAR register is modified
    void didChangeCAAR(u32 value);
    
    
    //
    // Debugger delegates
    //
    
    // Called when a soft stop is reached
    void didReachSoftstop(u32 addr);
    
    // Called when a breakpoint is hit
    void didReachBreakpoint(u32 addr);
    
    // Called when a watchpoint is hit
    void didReachWatchpoint(u32 addr);
    
    // Called when a catchpoint is hit
    void didReachCatchpoint(u8 vector);
    
    // Called when a software trap is hit
    void didReachSoftwareTrap(u32 addr);
    
#endif
    
    //
    // Accessing the clock
    //
    
public:
    
    // Returns the current CPU clock cycle count (elapsed cycles since power-up)
    i64 getClock() const { return clock; }
    
    // Sets the CPU clock cycle count
    void setClock(i64 val) { clock = val; }
    
    
    //
    // Accessing registers
    //
    
public:
    
    // Gets or sets the value of a data register (0 - 7)
    u32 getD(int n) const { return readD(n); }
    void setD(int n, u32 v) { writeD(n, v); }
    
    // Gets or sets the value of an address register (0 - 7)
    u32 getA(int n) const { return readA(n); }
    void setA(int n, u32 v) { writeA(n, v); }
    
    // Gets or sets the value of the program counter (PC)
    u32 getPC() const { return reg.pc; }
    void setPC(u32 val) { reg.pc = val; }
    
    // Gets or sets the address of the currently executed instruction
    u32 getPC0() const { return reg.pc0; }
    void setPC0(u32 val) { reg.pc0 = val; }
    
    // Gets or sets the IRC register, which is part of the prefetch queue
    u16 getIRC() const { return queue.irc; }
    void setIRC(u16 val) { queue.irc = val; }
    
    // Gets or sets the IRD register, which is part of the prefetch queue
    u16 getIRD() const { return queue.ird; }
    void setIRD(u16 val) { queue.ird = val; }
    
    // Gets or sets the value of the Condition Code Register (CCR)
    u8 getCCR() const;
    void setCCR(u8 val);
    
    // Gets or sets the value of the Status Register (SR)
    u16 getSR() const;
    void setSR(u16 val);
    
    // Gets or sets the current value of the stack pointer (SP)
    u32 getSP() const { return reg.sp; }
    void setSP(u32 val) { reg.sp = val; }
    
    // Gets or sets the User Stack Pointer (USP)
    u32 getUSP() const { return !reg.sr.s ? reg.sp : reg.usp; }
    void setUSP(u32 val) { if (!reg.sr.s) reg.sp = val; else reg.usp = val; }
    
    // Gets or sets the Interrupt Stack Pointer (ISP)
    u32 getISP() const { return (reg.sr.s && !reg.sr.m) ? reg.sp : reg.isp; }
    void setISP(u32 val) { if (reg.sr.s && !reg.sr.m) reg.sp = val; else reg.isp = val; }
    
    // Gets or sets the Master Stack Pointer (MSP)
    u32 getMSP() const { return (reg.sr.s && reg.sr.m) ? reg.sp : reg.msp; }
    void setMSP(u32 val) { if (reg.sr.s && reg.sr.m) reg.sp = val; else reg.msp = val; }
    
    // Gets or sets the Vector Base Register (VBR)
    u32 getVBR() const { return reg.vbr; }
    void setVBR(u32 val) { reg.vbr = val; }
    
    // Gets or sets the Source Function Code (SFC)
    u32 getSFC() const { return reg.sfc; }
    void setSFC(u32 val) { reg.sfc = val & 0b111; }
    
    // Gets or sets the Destination Function Code (DFC)
    u32 getDFC() const { return reg.dfc; }
    void setDFC(u32 val) { reg.dfc = val & 0b111; }
    
    // Gets or sets the Cache Control Register (CACR)
    u32 getCACR() const { return reg.cacr; }
    void setCACR(u32 val);
    
    // Gets or sets the Cache Address Register (CAAR)
    u32 getCAAR() const { return reg.caar; }
    void setCAAR(u32 val);

    
    //
    // Supervisor mode
    //
    
public:
    
    // Sets or clears supervisor mode
    void setSupervisorMode(bool value);
    
    // Sets or clears master mode
    void setMasterMode(bool value);
    
    // Sets or clear the supervisor and master flags
    void setSupervisorFlags(bool s, bool m);
    
    
    //
    // Trace Flags
    //
    
private:
    
    // Enables or disables trace mode (T1 flag)
    void setTraceFlag() { reg.sr.t1 = true; flags |= State::TRACING; }
    void clearTraceFlag() { reg.sr.t1 = false; flags &= ~State::TRACING; }
    
    // Enables or disables trace mode (T0 flag)
    void setTrace0Flag() { reg.sr.t0 = true; }
    void clearTrace0Flag() { reg.sr.t0 = false; }
    
    // Disables both trace flags (T0 and T1)
    void clearTraceFlags() { clearTraceFlag(); clearTrace0Flag(); }
    
    
    //
    // Register Access
    //
    
protected:
    
    // Reads a value from a data register (D0, D1 ... D7)
    template<Size S = Long> u32 readD(int n) const;
    
    // Reads a value from an address register (A0, A1 ... A7)
    template <Size S = Long> u32 readA(int n) const;
    
    // Reads a value from a register (D0, D1 ... D7, A0, A1 ... A7)
    template <Size S = Long> u32 readR(int n) const;
    
    // Writes a value to a data register (D0, D1 ... D7)
    template <Size S = Long> void writeD(int n, u32 v);
    
    // Writes a value to an address register (A0, A1 ... A7)
    template <Size S = Long> void writeA(int n, u32 v);
    
    // Writes a value to a register (D0, D1 ... D7, A0, A1 ... A7)
    template <Size S = Long> void writeR(int n, u32 v);

    
    //
    // Instruction Analysis
    //
    
public:
    
    // Retrieves the availability mask for a given instruction
    u16 availabilityMask(Instr I) const;
    u16 availabilityMask(Instr I, Mode M, Size S) const;
    u16 availabilityMask(Instr I, Mode M, Size S, u16 ext) const;
    
    // Checks if a given CPU model supports a specific instruction
    bool isAvailable(Model model, Instr I) const;
    bool isAvailable(Model model, Instr I, Mode M, Size S) const;
    bool isAvailable(Model model, Instr I, Mode M, Size S, u16 ext) const;
    
    
    //
    // Extension Word Validation
    //

private:

    // Validates extension words for a given instruction
    bool isValidExt(Instr I, Mode M, u16 op, u32 ext) const;
    
    // Validates extension words for MMU-related instructions
    bool isValidExtMMU(Instr I, Mode M, u16 op, u32 ext) const;
    
    // Validates extension words for FPU-related instructions
    bool isValidExtFPU(Instr I, Mode M, u16 op, u32 ext) const;
    
    
    //
    // Disassembler Support
    //
    
private:
    
    // Returns an availability string for a given instruction
    const char *availabilityString(Instr I, Mode M, Size S, u16 ext);

    
    //
    // Loop Mode Detection
    //
    
    // Checks if an instruction is a loop mode instruction
    template <Instr I> constexpr bool looping() {
        return I >= Instr::ABCD_LOOP && I <= Instr::TST_LOOP;
    }
    
    
    //
    // Managing the Function Code Pins
    //
    
public:
    
    // Reads the current value of the function code pins
    u8 readFC() const;
    
private:
    
    // Sets the function code pins to a specific value
    void setFC(u8 value);
    
    // Sets the function code pins based on the provided addressing mode.
    template <Mode M> void setFC();

    
    //
    // Handling Interrupts
    //
    
public:
    
    // Retrieves the value on the Interrupt Priority Level (IPL) pins
    u8 getIPL() const { return ipl; }
    
    // Sets the value on the Interrupt Priority Level (IPL) pins
    void setIPL(u8 val);
    
private:
    
    // Selects the IRQ vector based on the interrupt level
    u16 getIrqVector(u8 level) const;
    
private:
    
#include "MoiraInit.h"
#include "MoiraALU.h"
#include "MoiraDataflow.h"
#include "MoiraExceptions.h"
#include "MoiraDasm.h"
};

}
