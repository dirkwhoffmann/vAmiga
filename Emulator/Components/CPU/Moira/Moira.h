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
    Model cpuModel = M68000;

    // Instruction set used by the disassembler
    Model dasmModel = M68000;

    // Visual style for disassembled instructions
    DasmStyle instrStyle;

    // Visual style for data dumps
    DasmStyle dataStyle;


    //
    // Subcomponents
    //

public:

    // Floating point unit (not supported yet)
    // FPU fpu = FPU(*this);

    // Breakpoints, watchpoints, catchpoints, instruction tracing
    Debugger debugger = Debugger(*this);


    //
    // Internals
    //

protected:

    // Number of elapsed cycles since powerup
    i64 clock;

    // The register set
    Registers reg;

    // The prefetch queue
    PrefetchQueue queue;

    // The interrupt mode of this CPU
    IrqMode irqMode = IRQ_AUTO;

    // Current value on the IPL pins (Interrupt Priority Level)
    u8 ipl;

    // Value on the lower two function code pins (FC1|FC0)
    u8 fcl;

    // Determines the source of the function code pins
    u8 fcSource;

    // Remembers the vector number of the most recent exception
    int exception;

    // Cycle penalty (needed for 68020+ extended addressing modes)
    int cp;

    // Controls exact timing of instructions running in loop mode
    int loopModeDelay = 2;

    // Read and write buffers (appear in 68010 exception frames)
    u16 readBuffer;
    u16 writeBuffer;

    // State flags
    int flags;


    //
    // Lookup tables
    //

private:

    // Jump table holding the instruction handlers
    typedef void (Moira::*ExecPtr)(u16);
    // ExecPtr exec[65536];
    ExecPtr *exec = nullptr;

    // Jump table holding the loop mode instruction handlers (68010 only)
    // ExecPtr loop[65536];
    ExecPtr *loop = nullptr;

    // Jump table holding the disassebler handlers
    typedef void (Moira::*DasmPtr)(StrWriter&, u32&, u16) const;
    DasmPtr *dasm = nullptr;

    // Table holding instruction infos
    InstrInfo *info = nullptr;


    //
    // Constructing
    //

public:

    Moira(Amiga &ref);
    virtual ~Moira();

protected:

    // Creates or updates the jump tables
    void createJumpTable(Model cpuModel, Model dasmModel);
    void createJumpTable(Model model) { createJumpTable(model, model); }

private:

    // The createJumpTable core routine
    template <Core C> void createJumpTable(Model model, bool registerDasm);


    //
    // Configuring
    //

public:

    // Selects the emulated CPU model
    void setModel(Model cpuModel, Model dasmModel);
    void setModel(Model model) { setModel(model, model); }

    // Configures the visual appearance of disassembled instructions
    void setDasmSyntax(DasmSyntax value);
    void setDasmNumberFormat(DasmNumberFormat value) { setNumberFormat(instrStyle, value); }
    void setDasmLetterCase(DasmLetterCase value);
    void setDasmIndentation(int value) { instrStyle.tab = value; }

    // Configures the visual appearance of data dumps
    void setDumpNumberFormat(DasmNumberFormat value) { setNumberFormat(instrStyle, value); }
    void setDumpIndentation(int value) { dataStyle.tab = value; }

private:

    void setNumberFormat(DasmStyle &style, const DasmNumberFormat &value);


    //
    // Querying CPU properties
    //

public:

    // Checks if the emulated CPU model has a coprocessor interface
    bool hasCPI() const;

    // Checks if the emulated CPU model has a memory managenemt unit
    bool hasMMU() const;

    // Checks if the emulated CPU model has a floating point unit
    bool hasFPU() const;

    // Returns the cache register mask (accessible CACR bits)
    u32 cacrMask() const;

    // Returns the address bus mask (bus width)
    u32 addrMask() const;

protected:

    // The addrMask core routine
    template <Core C> u32 addrMask() const;


    //
    // Running the CPU
    //

public:

    // Performs a hard reset (power up)
    void reset();

    // Executes the next instruction
    void execute();

    // Returns true if the CPU is in HALT state
    bool isHalted() const { return flags & CPU_IS_HALTED; }

private:

    // Processes an exception that was catched in execute()
    void processException(const std::exception &exception);
    template <Core C> void processException(const std::exception &exception);

    // The reset core routine
    template <Core C> void reset();

    // Invoked inside execute() to check for a pending interrupt
    bool checkForIrq();

    // Puts the CPU into HALT state
    void halt();


    //
    // Running the disassembler
    //

public:

    // Disassembles a single instruction and returns the instruction size
    int disassemble(char *str, u32 addr) const;

    // Creates a textual representation for the status register
    void disassembleSR(char *str) const { disassembleSR(str, reg.sr); }
    void disassembleSR(char *str, const StatusRegister &sr) const;

    // Creates a textual representation for a single data value
    void dump8(char *str, u8 value) const;
    void dump16(char *str, u16 value) const;
    void dump24(char *str, u32 value) const;
    void dump32(char *str, u32 value) const;

    // Creates a textual representation for multiple data values
    void dump16(char *str, u16 values[], int cnt) const;
    void dump16(char *str, u32 addr, int cnt) const;

    // Return an info struct for a certain opcode
    InstrInfo getInstrInfo(u16 op) const;


    //
    // Interfacing with other components
    //

protected:

#if VIRTUAL_API == true

    // Advances the clock
    virtual void sync(int cycles) { clock += cycles; }

    // Reads a byte or a word from memory
    virtual u8 read8(u32 addr) const = 0;
    virtual u16 read16(u32 addr) const = 0;

    // Special variants used by the reset routine and the disassembler
    virtual u16 read16OnReset(u32 addr) const { return read16(addr); }
    virtual u16 read16Dasm(u32 addr) const { return read16(addr); }

    // Writes a byte or word into memory
    virtual void write8(u32 addr, u8 val) const = 0;
    virtual void write16(u32 addr, u16 val) const = 0;

    // Provides the interrupt level in IRQ_USER mode
    virtual u16 readIrqUserVector(u8 level) const { return 0; }

    // State delegates
    virtual void didReset() { }
    virtual void didHalt() { }

    // Instruction delegates
    virtual void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode) { }
    virtual void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode) { }

    // Exception delegates
    virtual void willExecute(ExceptionType exc, u16 vector) { }
    virtual void didExecute(ExceptionType exc, u16 vector) { }

    // Exception delegates
    virtual void willInterrupt(u8 level) { }
    virtual void didJumpToVector(int nr, u32 addr) { }

    // Cache register delegated
    virtual void didChangeCACR(u32 value) { }
    virtual void didChangeCAAR(u32 value) { }

    // Debugger delegates
    virtual void softstopReached(u32 addr) { }
    virtual void breakpointReached(u32 addr) { }
    virtual void watchpointReached(u32 addr) { }
    virtual void catchpointReached(u8 vector) { }
    virtual void softwareTrapReached(u32 addr) { }

#else

    // Advances the clock
    void sync(int cycles);

    // Reads a byte or a word from memory
    u8 read8(u32 addr) const;
    u16 read16(u32 addr) const;

    // Special variants used by the reset routine and the disassembler
    u16 read16OnReset(u32 addr) const;
    u16 read16Dasm(u32 addr) const;

    // Writes a byte or word into memory
    void write8(u32 addr, u8 val) const;
    void write16(u32 addr, u16 val) const;

    // Provides the interrupt level in IRQ_USER mode
    u16 readIrqUserVector(u8 level) const;

    // State delegates
    void cpuDidReset();
    void cpuDidHalt();

    // Instruction delegates
    void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode);
    void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode);

    // Exception delegates
    void willExecute(ExceptionType exc, u16 vector);
    void didExecute(ExceptionType exc, u16 vector);
    void willInterrupt(u8 level);
    void didJumpToVector(int nr, u32 addr);

    // Cache register delegated
    void didChangeCACR(u32 value);
    void didChangeCAAR(u32 value);

    // Debugger delegates
    void softstopReached(u32 addr);
    void breakpointReached(u32 addr);
    void watchpointReached(u32 addr);
    void catchpointReached(u8 vector);
    void softwareTrapReached(u32 addr);

#endif

    //
    // Accessing the clock
    //

public:

    i64 getClock() const { return clock; }
    void setClock(i64 val) { clock = val; }


    //
    // Accessing registers
    //

public:

    u32 getD(int n) const { return readD(n); }
    void setD(int n, u32 v) { writeD(n,v); }

    u32 getA(int n) const { return readA(n); }
    void setA(int n, u32 v) { writeA(n,v); }

    u32 getPC() const { return reg.pc; }
    void setPC(u32 val) { reg.pc = val; }

    u32 getPC0() const { return reg.pc0; }
    void setPC0(u32 val) { reg.pc0 = val; }

    u16 getIRC() const { return queue.irc; }
    void setIRC(u16 val) { queue.irc = val; }

    u16 getIRD() const { return queue.ird; }
    void setIRD(u16 val) { queue.ird = val; }

    u8 getCCR() const;
    void setCCR(u8 val);

    u16 getSR() const;
    void setSR(u16 val);

    u32 getSP() const { return reg.sp; }
    void setSP(u32 val) { reg.sp = val; }

    u32 getUSP() const { return !reg.sr.s ? reg.sp : reg.usp; }
    void setUSP(u32 val) { if (!reg.sr.s) reg.sp = val; else reg.usp = val; }

    u32 getISP() const { return (reg.sr.s && !reg.sr.m) ? reg.sp : reg.isp; }
    void setISP(u32 val) { if (reg.sr.s && !reg.sr.m) reg.sp = val; else reg.isp = val; }

    u32 getMSP() const { return (reg.sr.s && reg.sr.m) ? reg.sp : reg.msp; }
    void setMSP(u32 val) { if (reg.sr.s && reg.sr.m) reg.sp = val; else reg.msp = val; }

    u32 getVBR() const { return reg.vbr; }
    void setVBR(u32 val) { reg.vbr = val; }

    u32 getSFC() const { return reg.sfc; }
    void setSFC(u32 val) { reg.sfc = val & 0b111; }

    u32 getDFC() const { return reg.dfc; }
    void setDFC(u32 val) { reg.dfc = val & 0b111; }

    u32 getCACR() const { return reg.cacr; }
    void setCACR(u32 val);

    u32 getCAAR() const { return reg.caar; }
    void setCAAR(u32 val);

public:

    void setSupervisorMode(bool value);
    void setMasterMode(bool value);
    void setSupervisorFlags(bool s, bool m);

private:

    void setTraceFlag() { reg.sr.t1 = true; flags |= CPU_TRACE_FLAG; }
    void clearTraceFlag() { reg.sr.t1 = false; flags &= ~CPU_TRACE_FLAG; }

    void setTrace0Flag() { reg.sr.t0 = true; }
    void clearTrace0Flag() { reg.sr.t0 = false; }

    void clearTraceFlags() { clearTraceFlag(); clearTrace0Flag(); }

protected:

    template <Size S = Long> u32 readD(int n) const;
    template <Size S = Long> u32 readA(int n) const;
    template <Size S = Long> u32 readR(int n) const;
    template <Size S = Long> void writeD(int n, u32 v);
    template <Size S = Long> void writeA(int n, u32 v);
    template <Size S = Long> void writeR(int n, u32 v);


    //
    // Analyzing instructions
    //

public:

    // Returns the availability mask for a given instruction
    u16 availabilityMask(Instr I) const;
    u16 availabilityMask(Instr I, Mode M, Size S) const;
    u16 availabilityMask(Instr I, Mode M, Size S, u16 ext) const;

    // Checks if a certain CPU model supports a given instruction
    bool isAvailable(Model model, Instr I) const;
    bool isAvailable(Model model, Instr I, Mode M, Size S) const;
    bool isAvailable(Model model, Instr I, Mode M, Size S, u16 ext) const;

private:

    // Checks the validity of the extension words
    bool isValidExt(Instr I, Mode M, u16 op, u32 ext) const;
    bool isValidExtMMU(Instr I, Mode M, u16 op, u32 ext) const;
    bool isValidExtFPU(Instr I, Mode M, u16 op, u32 ext) const;

    // Returns an availability string (used by the disassembler)
    const char *availabilityString(Instr I, Mode M, Size S, u16 ext);

    // Checks if an instruction is a loop mode instruction
    template <Instr I>
    constexpr bool looping() { return I >= ABCD_LOOP && I <= TST_LOOP; }


    //
    // Managing the function code pins
    //

public:

    // Returns the current value on the function code pins
    u8 readFC() const;

private:

    // Sets the function code pins to a specific value
    void setFC(u8 value);

    // Sets the function code pins according the the provided addressing mode
    template <Mode M> void setFC();


    //
    // Handling interrupts
    //

public:

    // Gets or sets the value on the IPL pins
    u8 getIPL() const { return ipl; }
    void setIPL(u8 val);

private:

    // Selects the IRQ vector to branch to
    u16 getIrqVector(u8 level) const;


private:

#include "MoiraInit.h"
#include "MoiraALU.h"
#include "MoiraDataflow.h"
#include "MoiraExceptions.h"
#include "MoiraDasm.h"
};

}
