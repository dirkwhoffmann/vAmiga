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
#include "Cache/Cache.h"
#include "FPU/FPU.h"
#include "MMU/MMU.h"
#include "SubComponent.h"

namespace MOIRA_NAMESPACE {

/// Version of this release
namespace Version {

inline constexpr int major = 6;
inline constexpr int minor = 0;
inline constexpr int patch = 0;
inline constexpr int beta  = 1;

}

/** The Motorola 68k CPU emulator
 *
 * %Moira emulates the 68000, 68010, 68020, 68030 and 68040 (including the
 * 68EC and 68LC variants) together with an optional FPU and MMU. The class is
 * abstract. To use it, derive your own CPU class and implement the memory
 * access functions (see the protected functions below):
 *
 * ```cpp
 * class CPU : public moira::Moira {
 *
 *     u8 read8(u32 addr) const override;
 *     u16 read16(u32 addr) const override;
 *     u32 read32(u32 addr) const override;
 *     void write8(u32 addr, u8 val) const override;
 *     void write16(u32 addr, u16 val) const override;
 *     void write32(u32 addr, u32 val) const override;
 * };
 * ```
 *
 * The remaining virtual functions are optional delegates that inform the
 * application about events such as breakpoints, exceptions or cache register
 * changes.
 *
 * The interface functions are only virtual if the configuration option
 * MOIRA_VIRTUAL_API is enabled (the default). Otherwise, they are expected to
 * be provided by the application at compile time.
 */
class Moira : public SubComponent {
    
    friend class Debugger;
    friend class Breakpoints;
    friend class Watchpoints;
    friend class Catchpoints;
    friend class MMU030;
    friend class MMU040;
    friend class Cache;
    friend class ICache;
    friend class DCache;
    
    
    //
    // Configuration
    //
    
protected:
    
    // The emulated CPU model
    CPUModel cpuModel = CPUModel::M68000;

    // The emulated coprocessor (if any)
    CPModel cpModel = CPModel::NONE;

    // Instruction set used by the disassembler
    CPUModel dasmModel = CPUModel::M68000;
    
    // Visual style for disassembled instructions
    DasmStyle instrStyle;
    
    // Visual style for data dumps
    DasmStyle dataStyle;
    
    
    //
    // Subcomponents
    //
    
public:
    
    /// The memory management unit
    MMU mmu = MMU(*this);

    /// The floating-point unit
    FPU fpu = FPU();

    /// Debugger (breakpoints, watchpoints, catchpoints, tracing)
    Debugger debugger = Debugger(*this);

    /// Instruction cache (68020+)
    ICache iCache = ICache(*this);

    /// Data cache (68030+)
    DCache dCache = DCache(*this);


    
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

    // Speed trackers (68020 only)
    CycleBudget budget {};
    
    // Current value on the IPL (Interrupt Priority Level) pins
    u8 ipl {};

    // Set when a level 7 interrupt is taken; cleared when the pins leave level 7
    bool nmiTaken {};
    
    // Value on the lower two function code pins (FC1|FC0)
    u8 fcl {2};
    
    // Source of the function code pins
    u8 fcSource {};
    
    // Controls exact timing of instructions running in loop mode (68010 only)
    int loopModeDelay {2};
    
    // Read buffer (appears in 68010 exception frames)
    u16 readBuffer {};
    
    // Write buffer (appears in 68010 exception frames)
    u16 writeBuffer {};
    

    // Internal state flags
    int flags {};
    
    // Restart information of a faulted instruction (68020+)
    Restart restart {};

    
    //
    // Lookup tables
    //
    
private:
    
    // Function pointer types
    typedef void (Moira::*ExecPtr)(u16);
    typedef void (Moira::*DasmPtr)(StrWriter&, u32&, u16, Instr, Mode, Size) const;

    // Jump table holding the instruction handlers
    ExecPtr *exec = nullptr;
    
    // Jump table holding the loop mode instruction handlers (68010 only)
    ExecPtr *loop = nullptr;
    
    // Jump table holding the disassembler handlers of the emulated CPU model
    DasmPtr *dasm = nullptr;
    
    // Info table holding instruction descriptions of the emulated CPU model
    InstrInfo *info = nullptr;

    // Tables of the disassembler model (only allocated if it differs from the CPU model)
    DasmPtr *dasmAlt = nullptr;
    InstrInfo *infoAlt = nullptr;

    
    //
    // Public API
    //

public:

    //
    // Constructing
    //

    /** @brief Constructs and initializes a Moira instance
     *
     * The new instance emulates a Motorola 68000 and uses the default
     * disassembler style. The jump tables are created here. Call setModel()
     * to select a different CPU and reset() to bring the CPU into a defined
     * state before executing the first instruction.
     */
    Moira(Amiga &ref);
    
    virtual ~Moira();
    Moira(const Moira &) = delete;
    Moira &operator=(const Moira &) = delete;
    
    /** @brief Returns a version string for this release
     *
     * @return A string of the form `major.minor[.patch][bN]`, e.g., "5.0",
     *         "5.0.1" or "5.1b2" for the second beta of release 5.1.
     */
    static std::string version();

    /** @brief Returns a build number string for this release
     *
     * @return The version string, followed by the marker `[DEBUG BUILD]` if
     *         assertions are enabled, followed by the date and time the
     *         library was compiled.
     */
    static std::string build();


    //
    // Configuring
    //

    /** @brief Sets the emulated CPU model and the model used by the disassembler
     *
     * The CPU model determines which instructions and addressing modes are
     * available, how long they take, and which exceptions and stack frames
     * are generated. The disassembler model determines which instructions are
     * recognized when disassembling. Both models are independent of each
     * other. For example, a 68000 can be emulated while the disassembler
     * recognizes the complete 68040 instruction set.
     *
     * Changing a model rebuilds the jump tables, reconfigures the built-in
     * MMU and FPU, masks the CACR to the bits supported by the new model, and
     * resets the caches. The call has no effect if both models are unchanged.
     *
     * @note  The registers are not modified. Call reset() after changing the
     *        CPU model to bring the CPU into a defined state.
     *
     * @param cpu   The CPU model to emulate
     * @param dasm  The CPU model the disassembler is based on
     */
    void setModel(CPUModel cpu, CPUModel dasm);
    
    /** @brief Sets the emulated CPU model (the disassembler uses the same model)
     *  @param model  The CPU model
     *  @see setModel(CPUModel, CPUModel)
     */
    void setModel(CPUModel model) { setModel(model, model); }

    /// @brief Returns the emulated CPU model
    CPUModel getModel() const { return cpuModel; }

    /// @brief Returns the CPU model used by the disassembler
    CPUModel getDasmModel() const { return dasmModel; }

    /** @brief Sets the CPU model used by the disassembler
     *
     * The emulated CPU model is not changed.
     *
     * @param model  The CPU model the disassembler is based on
     */
    void setDasmModel(CPUModel model) { setModel(cpuModel, model); }

    /** @brief Returns the selected coprocessor
     *
     * The returned value is the selection made with setCPModel(). It does not
     * tell whether the coprocessor is effective. Use hasExternalFPU() for that.
     */
    CPModel getCPModel() const { return cpModel; }
    
    /** @brief Selects the coprocessor that is connected to the coprocessor interface
     *
     * The built-in units are determined by the CPU model: The 68040 has an FPU
     * and an MMU, the 68030 and the 68LC040 have an MMU, and the EC variants
     * have neither. A coprocessor (68881 or 68882) can be connected to CPUs
     * with a coprocessor interface (68EC020, 68020, 68EC030, 68030). For other
     * CPU models, the selection is remembered but has no effect. It is not
     * changed when the CPU model changes, so the order of setModel and
     * setCPModel does not matter.
     *
     * @param model  The coprocessor to connect, or CPModel::NONE
     */
    void setCPModel(CPModel model);
    
    /** @brief Returns the visual style of the disassembler output
     *
     * The style controls the syntax (Moira, MIT, GNU, ...), the letter case
     * of mnemonics and registers, the number format of immediate values and
     * displacements, and the indentation between the mnemonic and its
     * operands.
     */
    DasmStyle getDasmStyle() const { return instrStyle; }

    /** @brief Sets the visual style of the disassembler output
     *
     * The style affects the instruction text (`%i` in format strings). It does
     * not affect addresses, instruction words or registers, which are
     * formatted with the data style.
     *
     * @param style  The new style
     * @throws std::runtime_error if the number format is invalid, i.e., if
     *         the prefix is a null pointer or the radix is neither 10 nor 16.
     */
    void setDasmStyle(const DasmStyle &style);

    /** @brief Returns the style used for data
     *
     * The data style controls how addresses (`%p`), instruction words (`%b`)
     * and register values (`%Nd`, `%Na`) are printed.
     */
    DasmStyle getDataStyle() const { return dataStyle; }

    /** @brief Sets the style used for data
     *
     * @note  Only the number format and the indentation between multiple words
     *        matter.
     *
     * @param style  The new style
     * @throws std::runtime_error if the number format is invalid, i.e., if
     *         the prefix is a null pointer or the radix is neither 10 nor 16.
     */
    void setDataStyle(const DasmStyle &style);


    //
    // Querying the model
    //

    /** @brief Checks if the emulated CPU has a coprocessor interface (CPI)
     *
     * Only the 68020 and the 68030 (including their EC variants) can be
     * connected to a coprocessor.
     */
    bool hasCPI() const;
    
    /** @brief Checks if the emulated CPU has a memory management unit (MMU)
     *
     * If an MMU is present, it is accessible via the public member `mmu`.
     */
    bool hasMMU() const;
    
    /** @brief Checks if the emulated system has a floating-point unit
     *
     * @return true if the CPU has a built-in FPU or an external FPU is
     *         attached (see hasInternalFPU() and hasExternalFPU()).
     */
    bool hasFPU() const;
    
    /// @brief Checks if the emulated CPU has a built-in floating-point unit (68040)
    bool hasInternalFPU() const;

    /** @brief Checks if an external FPU (68881 or 68882) is attached
     *
     * An external FPU is attached if the CPU has a coprocessor interface and
     * a coprocessor has been selected with setCPModel().
     */
    bool hasExternalFPU() const;

    /** @brief Checks if a CPU model supports an instruction
     *
     * The function does not depend on the emulated CPU. It can be used to
     * query the instruction set of any model.
     *
     * @param model  The CPU model
     * @param I      The instruction (mnemonic)
     */
    bool isAvailable(CPUModel model, Instr I) const;

    /** @brief Checks if a CPU model supports an instruction with the given addressing mode and size
     *
     * @param model  The CPU model
     * @param I      The instruction (mnemonic)
     * @param M      The addressing mode
     * @param S      The operand size
     */
    bool isAvailable(CPUModel model, Instr I, Mode M, Size S) const;

    /** @brief Checks if a CPU model supports an instruction with the given addressing mode, size and extension word
     *
     * For some instructions, the availability depends on the extension word.
     * For example, MOVEC is available on different models depending on the
     * control register that is accessed.
     *
     * @param model  The CPU model
     * @param I      The instruction (mnemonic)
     * @param M      The addressing mode
     * @param S      The operand size
     * @param ext    The first extension word
     */
    bool isAvailable(CPUModel model, Instr I, Mode M, Size S, u16 ext) const;

    /** @brief Returns the address bus mask
     *
     * The mask defines the addressable memory range. It is 0x00FFFFFF for
     * CPUs with a 24-bit address bus (68000, 68010 and 68EC020) and
     * 0xFFFFFFFF for all other models.
     */
    u32 getAddrMask() const;
    
    /** @brief Returns the cache register mask
     *
     * The mask indicates which bits of the CACR can be set on the emulated
     * CPU. All other bits read as 0. The mask is 0x0003 for the 68020, 0x3313
     * for the 68030 and 0x80008000 for the 68040 family. For CPUs without
     * caches, the value has no meaning.
     */
    u32 getCacrMask() const;


    //
    // Running the CPU
    //

    /** @brief Performs a hard reset, simulating the native power-up sequence
     *
     * The registers are cleared, the CPU enters supervisor mode with the
     * interrupt mask set to 7, the caches are flushed, and the MMU, the FPU
     * and the debugger are reset. Then, the initial supervisor stack pointer
     * and the initial program counter are read from addresses 0 and 4 (via
     * read16OnReset()), and the prefetch queue is filled. Finally, the
     * delegate didResetCPU() is called.
     *
     * The clock is not cleared. It keeps counting from its current value. Use
     * setClock() to change it.
     */
    void reset();
    
    /** @brief Executes a single instruction
     *
     * Depending on the state of the CPU, the function processes the next
     * instruction, a pending exception (interrupt, trace, ...), or merely
     * lets time pass if the CPU is halted or stopped. Memory accesses and
     * timing are reported to the application via read8() ... write32() and
     * sync().
     *
     * @return The number of cycles that have elapsed
     */
    i64 execute();

    /** @brief Executes instructions for a given number of cycles
     *
     * Since the emulator cannot stop in the middle of an instruction, the
     * number of cycles that actually elapse may exceed the cycle budget by
     * the duration of the last instruction.
     *
     * @param cycles  The cycle budget
     * @return The number of cycles that have actually elapsed
     */
    i64 execute(i64 cycles);
    
    /** @brief Executes instructions until a specific cycle count is reached
     *
     * Like execute(i64), the function may overshoot by the duration of the
     * last instruction.
     *
     * @param cycle  The target value of the clock (see getClock())
     * @return The number of cycles that have actually elapsed
     */
    i64 executeUntil(i64 cycle);

    /** @brief Continues program execution at the specified address
     *
     * Unlike setPC, this function refills the prefetch queue (which causes
     * bus accesses and advances the clock), so that the next call to
     * execute() runs the instruction at the given address. Odd addresses are
     * rounded down.
     *
     * @param addr  The address of the next instruction
     */
    void jump(u32 addr);
    
    /** @brief Checks if the CPU is in a HALT state
     *
     * The CPU halts if an unrecoverable error occurs, e.g., a bus error while
     * processing a bus error. While halted, execute() only advances the clock.
     * Only reset() leaves this state. The delegate didHaltCPU() is called when
     * the CPU enters it.
     */
    bool isHalted() const { return flags & State::HALTED; }

    /** @brief Checks if the CPU is in the STOP state
     *
     * The CPU enters this state by executing the STOP instruction. It
     * remains there until an interrupt with a sufficiently high priority
     * occurs or the CPU is reset.
     */
    bool isStopped() const { return flags & State::STOPPED; }


    //
    // Accessing registers
    //

    /** @brief Gets the value of a data register
     *  @param n  The register number (0 - 7)
     */
    u32 getD(int n) const { return readD(n); }

    /** @brief Sets the value of a data register
     *  @param n  The register number (0 - 7)
     *  @param v  The new value
     */
    void setD(int n, u32 v) { writeD(n, v); }
    
    /** @brief Gets the value of an address register
     *
     * A7 is the active stack pointer, i.e., the USP, the ISP or the MSP
     * depending on the operation mode (see getSP()).
     *
     * @param n  The register number (0 - 7)
     */
    u32 getA(int n) const { return readA(n); }

    /** @brief Sets the value of an address register
     *  @param n  The register number (0 - 7)
     *  @param v  The new value
     */
    void setA(int n, u32 v) { writeA(n, v); }

    /** @brief Returns a read-only view of the complete register set
     *
     * The returned reference stays valid for the lifetime of the Moira
     * instance and always reflects the live state. Copy the object to take
     * a snapshot.
     */
    const Registers &getRegisters() const { return reg; }

    // Program counter

    /** @brief Gets the address of the next instruction (at an instruction boundary)
     *
     * Internally, the program counter follows the hardware: it always holds
     * the address of the word in IRC, which is two bytes ahead of the
     * instruction to be executed next. This function hides the offset.
     *
     * @note  Inside a delegate, the value depends on how far the current
     *        instruction has progressed. Use getPC0() to get the address of
     *        the instruction that is currently executed.
     */
    u32 getPC() const { return reg.pc - 2; }

    /** @brief Sets the address of the next instruction (see getPC)
     *
     * This function only changes the program counter. It neither refills the
     * prefetch queue nor updates pc0. To redirect program execution, use
     * jump().
     */
    void setPC(u32 val) { reg.pc = val + 2; }

    /** @brief Gets the address of the start of the current instruction
     *
     * While an instruction executes (e.g., inside a delegate), this is the
     * address of the instruction being executed. Between instructions, it is
     * the address of the next instruction to execute. This is what debuggers
     * need to display the current position.
     */
    u32 getPC0() const { return reg.pc0; }

    /// @brief Gets the address of the instruction that executed before the current one
    u32 getPrevPC0() const { return reg.prev_pc0; }

    /** @brief Sets the address of the start of the current instruction
     *
     * This function only changes the bookkeeping value. To redirect program
     * execution, use jump(), which keeps the program counter, pc0 and the
     * prefetch queue consistent.
     */
    void setPC0(u32 val) { reg.pc0 = val; }

    // Prefetch queue

    /** @brief Gets the IRC register
     *
     * IRC (instruction register capture) is the second stage of the
     * prefetch queue. It holds the word that follows the instruction word.
     */
    u16 getIRC() const { return queue.irc; }
    
    /** @brief Gets the IRD register
     *
     * IRD (instruction register decode) is the first stage of the prefetch
     * queue. It holds the opcode of the instruction that is executed next.
     */
    u16 getIRD() const { return queue.ird; }

    /** @brief Gets the data latch that holds the most recently fetched word
     *  @note  The latch is the 68000 pipeline stage in front of IRC.
     */
    u16 getEDB() const { return queue.edb; }
    
    // Status register and stack pointers

    /** @brief Gets the value of the Condition Code Register (CCR)
     *
     * The CCR is the lower byte of the status register. It contains the flags
     * C (bit 0), V (bit 1), Z (bit 2), N (bit 3) and X (bit 4).
     */
    u8 getCCR() const;

    /** @brief Sets the value of the Condition Code Register (CCR)
     *
     * Only the lower five bits are used.
     */
    void setCCR(u8 val);
    
    /** @brief Gets the value of the Status Register (SR)
     *
     * The register consists of the trace bits (T1 in bit 15, T0 in bit 14),
     * the supervisor bit (S, bit 13), the master bit (M, bit 12), the
     * interrupt priority mask (bits 8 - 10), and the CCR (bits 0 - 4). The
     * bits T0 and M exist on the 68020 and later models only.
     */
    u16 getSR() const;

    /** @brief Sets the value of the Status Register (SR)
     *
     * Besides setting the flags, the function updates the trace mode,
     * switches the stack pointer if the operation mode changes (see
     * setSupervisorFlags()), and makes the CPU re-evaluate pending
     * interrupts. The bits T0 and M are ignored on the 68000 and 68010.
     */
    void setSR(u16 val);
    
    /** @brief Gets the current value of the stack pointer (SP)
     *
     * This is the active stack pointer, i.e., the value of A7. Depending on
     * the S and M bits, it is the USP, the ISP or the MSP.
     */
    u32 getSP() const { return reg.sp; }

    /// @brief Sets the current value of the stack pointer (SP, A7)
    void setSP(u32 val) { reg.sp = val; }
    
    /** @brief Gets the User Stack Pointer (USP)
     *
     * The function returns the correct value regardless of whether the USP is
     * currently active (user mode) or banked out (supervisor mode).
     */
    u32 getUSP() const { return !reg.sr.s ? reg.sp : reg.usp; }

    /// @brief Sets the User Stack Pointer (USP), regardless of whether it is currently active
    void setUSP(u32 val) { if (!reg.sr.s) reg.sp = val; else reg.usp = val; }
    
    /** @brief Gets the Interrupt Stack Pointer (ISP)
     *
     * The ISP is the supervisor stack pointer. On the 68020 and later models
     * it is used if the M bit is clear.
     */
    u32 getISP() const { return (reg.sr.s && !reg.sr.m) ? reg.sp : reg.isp; }

    /// @brief Sets the Interrupt Stack Pointer (ISP), regardless of whether it is currently active
    void setISP(u32 val) { if (reg.sr.s && !reg.sr.m) reg.sp = val; else reg.isp = val; }
    
    /** @brief Gets the Master Stack Pointer (MSP)
     *
     * The MSP exists on the 68020 and later models. It is used in supervisor
     * mode if the M bit is set.
     */
    u32 getMSP() const { return (reg.sr.s && reg.sr.m) ? reg.sp : reg.msp; }

    /// @brief Sets the Master Stack Pointer (MSP), regardless of whether it is currently active
    void setMSP(u32 val) { if (reg.sr.s && reg.sr.m) reg.sp = val; else reg.msp = val; }
    
    /// @brief Gets the Vector Base Register (VBR), which exists on the 68010 and later models
    u32 getVBR() const { return reg.vbr; }

    /// @brief Sets the Vector Base Register (VBR)
    void setVBR(u32 val) { reg.vbr = val; }
    
    /** @brief Gets the Source Function Code register (SFC)
     *
     * The SFC (68010 and later) provides the function code for source
     * operands of the MOVES instruction.
     */
    u8 getSFC() const { return reg.sfc; }

    /// @brief Sets the Source Function Code register (SFC); only the lower three bits are used
    void setSFC(u8 val) { reg.sfc = val & 0b111; }
    
    /** @brief Gets the Destination Function Code register (DFC)
     *
     * The DFC (68010 and later) provides the function code for destination
     * operands of the MOVES instruction.
     */
    u8 getDFC() const { return reg.dfc; }

    /// @brief Sets the Destination Function Code register (DFC); only the lower three bits are used
    void setDFC(u8 val) { reg.dfc = val & 0b111; }
    
    /** @brief Gets the Cache Control Register (CACR)
     *
     * The CACR (68020 and later) controls the on-chip caches. Bits that are
     * not supported by the emulated model read as 0 (see getCacrMask()).
     */
    u32 getCACR() const { return reg.cacr; }

    /** @brief Sets the Cache Control Register (CACR)
     *
     * The value is masked with getCacrMask(). If one of the clear bits is
     * set, the corresponding cache (entry) is invalidated. The delegate
     * didChangeCACR() is called.
     */
    void setCACR(u32 val);
    
    /** @brief Gets the Cache Address Register (CAAR)
     *
     * The CAAR (68020 and 68030) selects the cache entry that is cleared if
     * one of the clear-entry bits in the CACR is set.
     */
    u32 getCAAR() const { return reg.caar; }

    /** @brief Sets the Cache Address Register (CAAR)
     *
     * The delegate didChangeCAAR() is called.
     */
    void setCAAR(u32 val);

    /** @brief Sets or clears supervisor mode
     *
     * The active stack pointer is swapped accordingly: The value of A7 is
     * saved in the stack pointer register that was active before and
     * replaced by the one that becomes active.
     *
     * @param value  true to enter supervisor mode, false to enter user mode
     */
    void setSupervisorMode(bool value);
    
    /** @brief Sets or clears master mode
     *
     * The M bit selects between the ISP and the MSP in supervisor mode. It
     * only exists on the 68020 and later models. The active stack pointer is
     * swapped accordingly.
     *
     * @param value  true to set the M bit, false to clear it
     */
    void setMasterMode(bool value);
    
    /** @brief Sets or clears the supervisor and master flags
     *
     * This function changes both flags at once. The active stack pointer is
     * swapped accordingly, so that A7 refers to the USP, the ISP or the MSP
     * depending on the new mode.
     *
     * @param s  The new value of the S bit
     * @param m  The new value of the M bit
     */
    void setSupervisorFlags(bool s, bool m);


    //
    // Accessing the clock
    //

    /** @brief Returns the current CPU clock cycle count
     *
     * The clock counts the cycles that have elapsed since power-up (or the
     * last call to setClock()). It advances whenever the CPU calls sync().
     */
    i64 getClock() const { return clock; }
    
    /** @brief Sets the CPU clock cycle count
     *
     * Only the counter is changed. No other state is affected.
     */
    void setClock(i64 val) { clock = val; }


    //
    // Accessing function code and interrupt pins
    //

    /** @brief Gets the current value of the function code pins
     *
     * The function code (FC2 - FC0) tells the outside world what kind of bus
     * cycle is performed: 1 (user data), 2 (user program), 5 (supervisor
     * data), 6 (supervisor program) or 7 (CPU space). For the MOVES
     * instruction, the value is taken from the SFC or DFC register.
     *
     * @note  Inside a delegate that is called before a bus cycle, the pins
     *        still show the value of the previous cycle.
     */
    u8 getFC() const;

    /** @brief Retrieves the value on the Interrupt Priority Level (IPL) pins
     *  @return The interrupt level (0 - 7). 0 means that no interrupt is pending.
     */
    u8 getIPL() const { return ipl; }
    
    /** @brief Sets the value on the Interrupt Priority Level (IPL) pins
     *
     * The CPU checks the pins at the next instruction boundary. An interrupt
     * is taken if its level is higher than the interrupt mask in the SR.
     * Level 7 is non-maskable: It is taken whenever the pins change to level
     * 7, even if the mask is also 7.
     *
     * @param val  The interrupt level (0 - 7)
     */
    void setIPL(u8 val);


    //
    // Disassembling
    //

    /** @brief Returns instruction metadata for a given opcode
     *
     * The result describes how the disassembler interprets the opcode, i.e.,
     * it depends on the disassembler model (see setDasmModel()).
     *
     * @param op  The opcode (the first word of an instruction)
     * @return The mnemonic, the addressing mode and the operand size
     * @throws std::runtime_error if the library has been compiled with
     *         MOIRA_ENABLE_DASM = false
     */
    InstrInfo getInstrInfo(u16 op) const;

    /** @brief Disassembles an instruction according to a format string
     *
     * The following directives are supported:
     *
     *   %p  Instruction address
     *   %b  Instruction words
     *   %i  Instruction
     *   %f  Status register flags
     *   %Nd Data register DN (N = 0 ... 7)
     *   %Na Address register AN (N = 0 ... 7)
     *
     * A number in front of %p, %b, %i, or %f pads the output with spaces to
     * the given width. The register directives print the current register
     * values. All other characters are copied to the output.
     *
     * Addresses, instruction words and registers are formatted with the data
     * style, the instruction is formatted with the disassembler style (see
     * setDataStyle() and setDasmStyle()). The instruction bytes are fetched
     * with read16Dasm(). By default, this delegate forwards to read16().
     * Override it if reading memory has side effects that the disassembler
     * should avoid.
     *
     * Example:
     * ```cpp
     * int len;
     * std::string s = cpu.disassemble("%p: %16b %i", cpu.getPC(), &len);
     * cpu.setPC(cpu.getPC() + len);
     * ```
     *
     * @param fmt   The format string
     * @param addr  The address of the instruction
     * @param len   If provided, receives the length of the instruction in bytes
     * @return The formatted text
     * @throws std::runtime_error if the format string is invalid or the
     *         library has been compiled with MOIRA_ENABLE_DASM = false
     */
    std::string disassemble(const std::string &fmt, u32 addr, int *len = nullptr) const;

    /** @brief Returns the length of the instruction at a given address
     *
     * The length is determined by the disassembler handlers of the emulated CPU
     * model (see setModel()). It does not depend on the disassembler model.
     *
     * @param addr  The address of the instruction
     * @return The length of the instruction in bytes (including all extension words)
     * @throws std::runtime_error if the library has been compiled with
     *         MOIRA_ENABLE_DASM = false
     */
    isize instrLength(u32 addr) const;


    //
    // Interfacing with other components
    //

protected:

#include "MoiraInterface.h"


    //
    // Internals
    //


    //
    // Constructing (internal)
    //

protected:

    // Creates or updates the jump tables for execution and disassembly
    void createJumpTable(CPUModel cpuModel, CPUModel dasmModel);
    
    // Computes an entry of the instruction length table

    // Creates the jump tables for execution and disassembly with the same model
    void createJumpTable(CPUModel model) { createJumpTable(model, model); }

private:

    // Core routines for creating jump tables    
    template <Core C> void createJumpTable(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable1(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable2(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable3(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable4(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable5(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable6(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable7(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable8(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable9(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable10(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable11(CPUModel model, bool registerExec, bool registerDasm);
    template <Core C> void createJumpTable12(CPUModel model, bool registerExec, bool registerDasm);


    //
    // Configuring (internal)
    //

private:

    // Configures the FPU and MMU to match the emulated CPU configuration
    void configureUnits();



protected:

    // Returns the address bus mask for a specific CPU core type
    template <Core C> u32 addrMask() const;


    //
    // Running the CPU (internal)
    //

private:

    // Performs a core-specific reset routine
    template <Core C> void reset();

    // Core-specific implementation of jump()
    template <Core C> void jump(u32 addr);
    
    // Puts the CPU into a HALT state, stopping execution
    void halt();

    // Reads an instruction word for an exception frame (0 if the read fails)
    u16 peekInstr(u32 addr) const;

    // Returns the length of an instruction for an exception frame (2 if it cannot be read)
    u32 peekInstrLength(u32 addr) const;


    //
    // Clock (internal)
    //

protected:

    /* Advances the clock by the cycle count computed for a 68020 instruction.
     *
     * Cache hits on instruction fetches are modelled as cycle discounts (see
     * readInstructionWord), so the resulting count may be zero or negative.
     * Since the environment can only advance in whole bus cycles, the time is
     * accumulated until at least one bus cycle is available. This preserves the
     * full cache discount.
     *
     * If no bus cycle is accumulated for maxStalled consecutive instructions,
     * one cycle is borrowed to ensure progress. Since the environment is stepped
     * only here, failing to do so would stall the emulation indefinitely.
     */
    void syncCyclePenalty(int cycles);


    //
    // Registers (internal)
    //

private:

    // Called when an instruction (or exception) has been processed: pc0 moves to the next instruction
    void advancePC0() { reg.prev_pc0 = reg.pc0; reg.pc0 = reg.pc - 2; }

    // Enables or disables trace mode (T1 flag)
    void setTraceFlag() { reg.sr.t1 = true; flags |= State::TRACING; }
    void clearTraceFlag() { reg.sr.t1 = false; flags &= ~State::TRACING; }

    /* Trace on change of flow (T1T0 = 01, 68020 and later). Instructions that change the
     * program counter or modify the status register call this function. A trace exception
     * follows the instruction. If T1 is set, every instruction is traced anyway.
     * STOP is not traced. RTE is traced if T0 was set before it started (UAE only does so
     * when the T0, S or M bit changes). */
    template <Core C> void changeOfFlow()
    {
        if constexpr (C == Core::C68020) {
            if (reg.sr.t0 && !reg.sr.t1) flags |= State::TRACE_EXC;
        }
    }
    
    // Enables or disables trace mode (T0 flag)
    void setTrace0Flag() { reg.sr.t0 = true; }
    void clearTrace0Flag() { reg.sr.t0 = false; }
    
    // Disables both trace flags (T0 and T1)
    void clearTraceFlags() { clearTraceFlag(); clearTrace0Flag(); }

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
    // Instruction availability (internal)
    //

private:

    // Retrieves the availability mask for a given instruction
    u16 availabilityMask(Instr I) const;
    u16 availabilityMask(Instr I, Mode M, Size S) const;
    u16 availabilityMask(Instr I, Mode M, Size S, u16 ext) const;


    //
    // Disassembling (internal)
    //

private:

    // Disassembles a single instruction into a character buffer and returns the instruction size in bytes
    int disassembleInstr(char *str, u32 addr, bool cpuView = false) const;

    // Implements the format-string disassembler
    std::string disassemble(const std::string &fmt, u32 addr, const Registers &regs, int *len) const;

    // Creates a textual representation of a status register
    void disassembleSR(char *str, const StatusRegister &sr) const;

    // Converts values to a textual representation (uses the data style)
    void dump16(char *str, u32 addr, int cnt) const;
    void dump24(char *str, u32 value) const;
    void dump32(char *str, u32 value) const;


    //
    // Extension word validation
    //

private:

    // Validates extension words for a given instruction
    bool isValidExt(Instr I, Mode M, u16 op, u32 ext) const;
    
    // Validates extension words for MMU-related instructions
    bool isValidExtMMU(Instr I, Mode M, u16 op, u32 ext) const;
    
    // Validates extension words for FPU-related instructions
    bool isValidExtFPU(Instr I, Mode M, u16 op, u32 ext, CPUModel model) const;


    //
    // Managing the Function Code Pins (internal)
    //

private:

    // Sets the function code pins to a specific value
    void setFC(u8 value);
    
    // Sets the function code pins based on the provided addressing mode.
    template <Mode M> void setFC();


    //
    // Handling Interrupts (internal)
    //

private:

    // Checks for a pending interrupt and handles it if necessary
    bool checkForIrq();

    // Selects the IRQ vector based on the interrupt level
    u16 getIrqVector(u8 level);


    //
    // Running the coprocessor interface
    //

private:

    // Announces the first cycle of the coprocessor protocol of a line-F instruction
    void notifyCPCycle(u16 opcode, CIR cir);


    //
    // Running the MMU
    //

private:

    // Extracts the function code from the extension word of an MMU instruction
    u8 mmuFC(u16 ext) const;

    // Checks whether MOVEC accepts a control register number
    bool isValidMovecReg(u16 rc) const;

    // Translates a logical address to a physical address
    template <Core C, bool write, Size S> u32 translate(u32 addr, u8 fc, bool *ci = nullptr);


private:
    
#include "MoiraInit.h"
#include "MoiraALU.h"
#include "MoiraDataflow.h"
#include "MoiraExceptions.h"
#include "MoiraDasm.h"
};

/* The jump table builders are the only place where the instruction handler
 * templates are instantiated. Keeping each core in a translation unit of its
 * own splits the instantiation work three ways and keeps the compiler's peak
 * memory usage within bounds. The definitions live in MoiraCore680x0.cpp.
 */
extern template void Moira::createJumpTable<Core::C68000>(CPUModel, bool, bool);
extern template void Moira::createJumpTable<Core::C68010>(CPUModel, bool, bool);
extern template void Moira::createJumpTable<Core::C68020>(CPUModel, bool, bool);

}
