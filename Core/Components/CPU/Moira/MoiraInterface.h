// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

/* The interface between the CPU core and the application
 *
 * This file is included inside the class body of Moira. It declares the
 * functions the application provides: The memory access functions and the
 * clock synchronization function, which are mandatory, and a set of optional
 * delegates that inform the application about events.
 *
 * Each function is declared once, using one of the following macros:
 *
 *   MOIRA_REQUIRED(decl)        A function the application must implement.
 *   MOIRA_DELEGATE(decl)        An optional delegate that does nothing by default.
 *   MOIRA_DEFAULT(decl, body)   An optional function with a default implementation.
 *
 * If MOIRA_VIRTUAL_API is enabled, the macros declare virtual functions and
 * the application overrides them in a subclass. Otherwise, they declare
 * ordinary member functions that the application has to define at compile time
 * (all of them, including the optional ones). The second variant avoids the
 * cost of virtual calls.
 */

#if MOIRA_VIRTUAL_API == true

#define MOIRA_REQUIRED(decl)       virtual decl = 0;
#define MOIRA_DELEGATE(decl)       virtual decl { }
#define MOIRA_DEFAULT(decl, ...)   virtual decl { __VA_ARGS__ }

#else

#define MOIRA_REQUIRED(decl)       decl;
#define MOIRA_DELEGATE(decl)       decl;
#define MOIRA_DEFAULT(decl, ...)   decl;

#endif


//
// Clock and memory access
//

/** @brief Advances the internal clock by the specified number of cycles
 *
 * The CPU calls this function whenever time passes, typically in steps of
 * two cycles (one bus cycle takes four). Applications use it to keep other
 * components (e.g., DMA or video hardware) in sync with the CPU. The call
 * happens before the bus access that follows, so a device can catch up before
 * its registers are read.
 *
 * The default implementation advances `clock`. An overriding implementation
 * has to do the same (e.g., by calling Moira::sync), or the clock stops
 * counting.
 *
 * @param cycles  The number of CPU clock cycles that elapsed
 */
MOIRA_DEFAULT(void sync(int cycles), clock += cycles;)

/** @brief Reads a byte from memory
 *
 * The address is masked with getAddrMask(). If an MMU is active, it is the
 * physical address.
 *
 * @param addr  The address
 * @return The value at the address
 */
MOIRA_REQUIRED(u8 read8(u32 addr) const)

/** @brief Reads a word from memory
 *
 * The address is masked with getAddrMask(). On the 68000 and 68010, it is
 * always even (odd addresses raise an address error before the bus is used).
 *
 * @param addr  The address
 * @return The value at the address
 */
MOIRA_REQUIRED(u16 read16(u32 addr) const)

/** @brief Reads a long word from memory
 *
 * The address is masked with getAddrMask(). The CPU may split a long word
 * access into two word accesses (see read16()), depending on the CPU model and
 * the port size reported by dsack().
 *
 * @param addr  The address
 * @return The value at the address
 */
MOIRA_REQUIRED(u32 read32(u32 addr) const)

/** @brief Reads a word from memory during the reset routine
 *
 * The CPU calls this function to fetch the initial stack pointer and the
 * initial program counter from addresses 0 - 7. Some systems map a different
 * memory (e.g., a ROM overlay) into this range while the reset is in progress.
 * The default implementation forwards to read16().
 *
 * @param addr  The address
 */
MOIRA_DEFAULT(u16 read16OnReset(u32 addr) const, return read16(addr);)

/** @brief Reads a word from memory for disassembly purposes
 *
 * The disassembler (see Moira::disassemble) and the debugger use this function
 * to fetch instruction bytes. The default implementation forwards to read16().
 * Override it if reading memory has side effects (e.g., I/O registers) that
 * disassembling should not trigger.
 *
 * @param addr  The address
 */
MOIRA_DEFAULT(u16 read16Dasm(u32 addr) const, return read16(addr);)

/** @brief Writes a byte into memory
 *  @param addr  The address (see read8())
 *  @param val   The value to write
 */
MOIRA_REQUIRED(void write8(u32 addr, u8 val) const)

/** @brief Writes a word into memory
 *  @param addr  The address (see read16())
 *  @param val   The value to write
 */
MOIRA_REQUIRED(void write16(u32 addr, u16 val) const)

/** @brief Writes a long word into memory
 *  @param addr  The address (see read32())
 *  @param val   The value to write
 */
MOIRA_REQUIRED(void write32(u32 addr, u32 val) const)

/** @brief Returns the DSACK bits for the addressed device (68020 and later)
 *
 * The 68020 family adapts its bus cycles to the port size of the addressed
 * device. The result tells how wide the port is: DSACK_32 (32-bit port),
 * DSACK_16 (16-bit port, the default), DSACK_8 (8-bit port) or DSACK_WAIT
 * (not acknowledged, treated like an 8-bit port). The CPU splits memory
 * accesses into as many bus cycles as the port width requires.
 *
 * @param addr  The address of the bus cycle
 */
MOIRA_DEFAULT(u8 dsack(u32 addr) const, return DSACK_16;)

/** @brief Answers an interrupt acknowledge cycle
 *
 * The CPU calls this function when it processes an interrupt, after it has
 * announced the CPU space cycle with willExecute(CpuSpaceType, u8).
 *
 * @param level   The acknowledged interrupt level (1 - 7)
 * @param vector  Receives the vector number if the answer is IrqMode::USER
 * @return How the interrupt is answered: IrqMode::AUTO (default) selects the
 *         autovector 24 + level, IrqMode::USER means that the device supplies
 *         the vector number, IrqMode::SPURIOUS selects vector 24 and
 *         IrqMode::UNINITIALIZED selects vector 15.
 */
MOIRA_DEFAULT(IrqMode readIrqVector(u8 level, u8 *vector) const, return IrqMode::AUTO;)


//
// State delegates
//

/// @brief Called after the CPU has completed the reset sequence
MOIRA_DELEGATE(void didResetCPU())

/** @brief Called when the CPU enters the HALT state
 *
 * The CPU halts if an unrecoverable error occurs (e.g., a bus error while
 * processing a bus error). Only a reset leaves this state.
 */
MOIRA_DELEGATE(void didHaltCPU())

/// @brief Called when the MMU is enabled (the E bit in the translation control register is set)
MOIRA_DELEGATE(void didEnableMMU())

/// @brief Called when the MMU is disabled (the E bit in the translation control register is cleared)
MOIRA_DELEGATE(void didDisableMMU())


//
// Instruction delegates
//

/** @brief Called before an instruction is executed
 *
 * To keep the emulator fast, the delegate is only called for the instructions
 * selected by the configuration option MOIRA_WILL_EXECUTE. By default, these
 * are STOP, TAS and BKPT.
 *
 * @param func    The name of the handler function
 * @param I       The instruction (mnemonic)
 * @param M       The addressing mode
 * @param S       The operand size
 * @param opcode  The opcode
 */
MOIRA_DELEGATE(void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode))

/** @brief Called before the bus is used to read or write a data operand
 *
 * @note  Only called if the option MOIRA_REPORT_BUS_ACCESS is enabled.
 *
 * @param addr   The (logical) address of the operand
 * @param size   The operand size
 * @param write  true for a write access, false for a read access
 */
MOIRA_DELEGATE(void willAccessBus(u32 addr, Size size, bool write))

/** @brief Called after an instruction has been executed
 *
 * To keep the emulator fast, the delegate is only called for the instructions
 * selected by the configuration option MOIRA_DID_EXECUTE. By default, this is
 * RESET.
 *
 * @param func    The name of the handler function
 * @param I       The instruction (mnemonic)
 * @param M       The addressing mode
 * @param S       The operand size
 * @param opcode  The opcode
 */
MOIRA_DELEGATE(void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode))


//
// Exception delegates
//

/** @brief Called before an exception is executed
 *
 * @param exc     The exception type
 * @param vector  The vector number of the exception
 */
MOIRA_DELEGATE(void willExecute(M68kException exc, u16 vector))

/** @brief Called after an exception has been executed
 *
 * @param exc     The exception type
 * @param vector  The vector number of the exception
 */
MOIRA_DELEGATE(void didExecute(M68kException exc, u16 vector))

/** @brief Called when an interrupt is about to be processed
 *
 * The delegate is called before the interrupt is acknowledged, i.e., before
 * readIrqVector() is called.
 *
 * @param level  The interrupt level (1 - 7)
 */
MOIRA_DELEGATE(void willInterrupt(u8 level))

/** @brief Called when the CPU jumps to an exception vector
 *
 * The call happens after the exception stack frame has been written and the
 * new program counter has been loaded from the vector table.
 *
 * @param nr    The vector number
 * @param addr  The address of the first instruction of the exception handler
 */
MOIRA_DELEGATE(void didJumpToVector(int nr, u32 addr))

/** @brief Called before a CPU space cycle (FC = 7) is executed
 *
 * @param type  The cycle type
 * @param arg   The interrupt level (IRQ_ACK), the breakpoint number
 *              (BREAKPOINT), or the coprocessor ID in bits 7-5 and the
 *              register select (a CIR value) in bits 4-0 (COPROCESSOR). This
 *              is the same value that appears on the address lines
 *              A15-A13 and A4-A0.
 * @note  The function code pins still show the value of the previous
 *        cycle (getFC()). The cycle type implies FC = 7. For interrupt
 *        acknowledge cycles, the notification precedes readIrqVector().
 */
MOIRA_DELEGATE(void willExecute(CpuSpaceType type, u8 arg))

/** @brief Called after a CPU space cycle has been executed
 *
 * @param type  The cycle type
 * @param arg   The same value that has been passed to willExecute()
 * @note  For interrupt acknowledge cycles, readIrqVector() has been
 *        called at this point.
 */
MOIRA_DELEGATE(void didExecute(CpuSpaceType type, u8 arg))


//
// Cache register delegates
//

/** @brief Called when the CACR register is modified
 *  @param value  The value that has been written (before masking with getCacrMask())
 */
MOIRA_DELEGATE(void didChangeCACR(u32 value))

/** @brief Called when the CAAR register is modified
 *  @param value  The value that has been written
 */
MOIRA_DELEGATE(void didChangeCAAR(u32 value))


//
// Debugger delegates
//

/** @brief Called when a soft stop is reached
 *
 * Soft stops are used to implement single-stepping (see Debugger::stepInto
 * and Debugger::stepOver). The CPU does not stop on its own. The application
 * decides how to react, e.g., by leaving its emulation loop.
 *
 * @param addr  The address of the instruction that is about to be executed
 */
MOIRA_DELEGATE(void didReachSoftstop(u32 addr))

/** @brief Called when a breakpoint is hit
 *
 * The delegate is called before the instruction at the breakpoint address is
 * executed. The CPU does not stop on its own.
 *
 * @param addr  The address of the breakpoint
 */
MOIRA_DELEGATE(void didReachBreakpoint(u32 addr))

/** @brief Called when a watchpoint is hit
 *
 * The delegate is called when the CPU accesses a watched address, before the
 * access is carried out. The CPU does not stop on its own.
 *
 * @param addr  The accessed (physical) address
 */
MOIRA_DELEGATE(void didReachWatchpoint(u32 addr))

/** @brief Called when a catchpoint is hit
 *
 * The delegate is called when the CPU takes an exception for which a
 * catchpoint has been set.
 *
 * @param vector  The vector number of the exception
 */
MOIRA_DELEGATE(void didReachCatchpoint(u8 vector))

/** @brief Called when a software trap is hit
 *
 * The trap opcode is replaced by the original instruction, which is executed
 * next. The trap itself stays installed.
 *
 * @param addr  The address of the instruction that has been trapped
 */
MOIRA_DELEGATE(void didReachSoftwareTrap(u32 addr))

/** @brief Called after an instruction has been appended to the log buffer
 *
 * The delegate is only called if logging has been enabled with
 * Debugger::enableLogging.
 *
 * @param reg  The register state at the beginning of the logged instruction
 */
MOIRA_DELEGATE(void didLogInstruction(const Registers &reg))

#undef MOIRA_REQUIRED
#undef MOIRA_DELEGATE
#undef MOIRA_DEFAULT
