// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "MoiraConfig.h"
#include "Moira.h"
#include "MoiraMacros.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <bit>
#include <vector>
#include <stdexcept>

namespace MOIRA_NAMESPACE {

using namespace Flag;

// Mark this file as the main translation unit
#define MOIRA_MAIN_TU

#include "MoiraInit_cpp.h"
#include "MoiraALU_cpp.h"
#include "MoiraDataflow_cpp.h"
#include "MoiraExceptions_cpp.h"
#include "MoiraExec_cpp.h"
#include "StrWriter_cpp.h"
#include "MoiraDasm_cpp.h"
#include "MoiraInfo_cpp.h"
#include "MoiraRegisters_cpp.h"

// Instantiations used by the caches (see Cache.cpp)
template u32 Moira::translate<Core::C68020, false, Word>(u32, u8, bool *);
template u32 Moira::translate<Core::C68020, false, Long>(u32, u8, bool *);
template u32 Moira::readLongSplit<Core::C68020, 0>(u32);

Moira::Moira(Amiga &ref) : SubComponent(ref)
{
    exec = new ExecPtr[65536];
    loop = new ExecPtr[65536];
    info = MOIRA_ENABLE_DASM ? new InstrInfo[65536] : nullptr;
    dasm = MOIRA_ENABLE_DASM ? new DasmPtr[65536] : nullptr;

    createJumpTable(cpuModel, dasmModel);

    instrStyle = DasmStyle {

        .syntax         = Syntax::MOIRA,
        .letterCase     = LetterCase::MIXED_CASE,
        .numberFormat   = { .prefix = "$", .radix = 16, .upperCase = false, .plainZero = false },
        .tab            = 8
    };

    dataStyle = DasmStyle {

        .syntax         = Syntax::MOIRA,
        .letterCase     = LetterCase::MIXED_CASE,
        .numberFormat   = { .prefix = "", .radix = 16, .upperCase = false, .plainZero = false },
        .tab            = 1
    };
}

Moira::~Moira()
{
    if (exec) delete [] exec;
    if (loop) delete [] loop;
    if (info) delete [] info;
    if (dasm) delete [] dasm;
    if (infoAlt) delete [] infoAlt;
    if (dasmAlt) delete [] dasmAlt;
}

std::string
Moira::version()
{
    std::string result;
    
    result = std::to_string(Version::major) + "." + std::to_string(Version::minor);
    if constexpr (Version::patch > 0) result += "." + std::to_string(Version::patch);
    if constexpr (Version::beta > 0) result += 'b' + std::to_string(Version::beta);
    
    return result;
}

std::string
Moira::build()
{
#ifdef NDEBUG
    std::string db = "";
#else
    std::string db = " [DEBUG BUILD]";
#endif
    
    return version() + db + " (" + __DATE__ + " " + __TIME__ + ")";
}

void
Moira::setModel(CPUModel cpu, CPUModel dasm)
{
    if (cpuModel != cpu || dasmModel != dasm) {
        
        cpuModel = cpu;
        dasmModel = dasm;

        configureUnits();
        createJumpTable(cpuModel, dasmModel);

        reg.cacr &= getCacrMask();
        iCache.reset(cpuModel);
        dCache.reset(cpuModel);
        
        // The 68040 caches are not emulated. The CPU behaves like it had perfectly coherent caches
        iCache.setEmulated(MOIRA_EMULATE_ICACHE && cpuModel >= CPUModel::M68EC020 && cpuModel < CPUModel::M68EC040);
        dCache.setEmulated(MOIRA_EMULATE_DCACHE && (cpuModel == CPUModel::M68030 || cpuModel == CPUModel::M68EC030));
        
        flags &= ~State::LOOPING;
    }
}

void
Moira::setCPModel(CPModel model)
{
    if (cpModel == model) return;

    cpModel = model;
    configureUnits();

    // The FPU handlers and the disassembler entries depend on the coprocessor
    if (moira::hasCPI(cpuModel) || moira::hasCPI(dasmModel)) createJumpTable(cpuModel, dasmModel);
}

void
Moira::setDasmStyle(const DasmStyle &style)
{
    style.validate();
    instrStyle = style;
}

void
Moira::setDataStyle(const DasmStyle &style)
{
    style.validate();
    dataStyle = style;
}

void
Moira::reset()
{
    mmu.reset();
    fpu.reset();

    switch (cpuModel) {

        case CPUModel::M68000: reset<Core::C68000>(); break;
        case CPUModel::M68010: reset<Core::C68010>(); break;
        
        default:
            reset<Core::C68020>();
    }
}

i64
Moira::execute()
{
    using namespace State;

    const auto start = clock;

    // Check the integrity of the IRQ flag
    if (reg.ipl > reg.sr.ipl || (reg.ipl == 7 && !nmiTaken)) assert(flags & CHECK_IRQ);

    // Check the integrity of the trace flag
    assert(!!(flags & TRACING) == reg.sr.t1);

    // Check the integrity of the program counter
    assert(reg.pc == reg.pc0 + 2);

    // Take the fast path or the slow path
    if (!flags) {

        //
        // Fast path: Call the instruction handler and return
        //

        try {
            (this->*exec[queue.ird])(queue.ird);
        } catch (const std::exception &exc) {
            processException(exc);
        }
        
        advancePC0();

    } else {

        //
        // Slow path: Process flags one by one
        //

        if (flags & (HALTED | TRACE_EXC | TRACING)) {

            // Only continue if the CPU is not halted
            if (flags & HALTED) {
                sync(2);
                return clock - start;
            }

            // Process pending trace exception (if any)
            if (flags & TRACE_EXC) {
                execException(M68kException::TRACE);
                goto done;
            }

            // Check if the T flag is set inside the status register
            if ((flags & TRACING) && !(flags & STOPPED)) {
                flags |= TRACE_EXC;
            }
        }

        // Process pending interrupt (if any)
        if (flags & CHECK_IRQ) {

            try {
                if (checkForIrq()) goto done;
            } catch (const std::exception &exc) {
                processException(exc);
                advancePC0();
            }
        }

        // If the CPU is stopped, poll the IPL lines and return
        if (flags & STOPPED) {

            POLL_IPL;
            sync((compat == Compatibility::MUSASHI) ? 1 : 2);
            return clock - start;
        }

        // If logging is enabled, record the executed instruction
        if (flags & LOGGING) {
            debugger.logInstruction();
        }

        // Execute the instruction
        if (flags & (State::SAVE_REGS | State::MMU)) {

            const Registers snapshot = reg;

            try {
                (this->*((flags & LOOPING) ? loop[queue.ird] : exec[queue.ird]))(queue.ird);
            } catch (const std::exception &exc) {
                processException(exc, &snapshot);
            }

        } else {

            try {
                (this->*((flags & LOOPING) ? loop[queue.ird] : exec[queue.ird]))(queue.ird);
            } catch (const std::exception &exc) {
                processException(exc);
            }
        }

    done:

        // The next instruction starts where the prefetch queue points to
        advancePC0();

        // Check if a breakpoint has been reached
        if (flags & CHECK_BP) {

            // Don't break if the instruction won't be executed due to tracing
            if (flags & TRACE_EXC) return clock - start;

            // Check if a softstop has been reached
            if (debugger.softstopMatches(reg.pc0)) didReachSoftstop(reg.pc0);

            // Check if a breakpoint has been reached
            if (debugger.breakpointMatches(reg.pc0)) didReachBreakpoint(reg.pc0);
        }
    }

    // Check the integrity of the program counter again
    assert(reg.pc == reg.pc0 + 2);

    return clock - start;
}

i64
Moira::execute(i64 cycles)
{
    return executeUntil(clock + cycles);
}

i64
Moira::executeUntil(i64 cycle)
{
    const i64 start = clock;
    while (clock < cycle) { execute(); }
    return clock - start;
}

void
Moira::jump(u32 addr)
{
    switch (cpuModel) {

        case CPUModel::M68000: jump<Core::C68000>(addr); break;
        case CPUModel::M68010: jump<Core::C68010>(addr); break;

        default:
            jump<Core::C68020>(addr);
    }
}

template <Core C> void
Moira::jump(u32 addr)
{
    reg.pc = addr & ~1;
    fullPrefetch<C, Flag::POLL>();
    reg.pc0 = reg.pc - 2;
}

u8
Moira::getFC() const
{
    switch (fcSource) {

        case 0: return u8((reg.sr.s ? 4 : 0) | fcl);
        case 1: return reg.sfc;
        case 2: return reg.dfc;

        default:
            fatalError;
    }
}

void
Moira::setIPL(u8 val)
{
    if (ipl != val) {
        
        ipl = val;
        flags |= State::CHECK_IRQ;
    }
}

InstrInfo
Moira::getInstrInfo(u16 op) const
{
    if constexpr (!MOIRA_ENABLE_DASM)
        throw std::runtime_error("This feature requires MOIRA_ENABLE_DASM = true\n");
        
    return infoAlt ? infoAlt[op] : info[op];
}

isize
Moira::instrLength(u32 addr) const
{
    // Use the CPU model instead of the DASM model for disassembly
    constexpr bool cpuView = true;
    
    char tmp[128];
    return disassembleInstr(tmp, addr, cpuView);
}

/* The exception handlers read instruction words to fill in the stack frame
 * (e.g., the pipe stages of a bus fault frame). These reads are not part of
 * the emulated bus traffic. If one of them hits the address that has just
 * faulted, the host throws again. This must not be treated as a double bus
 * fault, because the real CPU performs no bus cycle at this point.
 */
u16
Moira::peekInstr(u32 addr) const
{
    try { return read16Dasm(addr); } catch (const BusError &) { return 0; }
}

u32
Moira::peekInstrLength(u32 addr) const
{
    try { return u32(instrLength(addr)); } catch (const BusError &) { return 2; }
}

void
Moira::configureUnits()
{
    switch (cpuModel) {

        case CPUModel::M68EC020:
        case CPUModel::M68020:
        case CPUModel::M68EC030:

            mmu.setModel(MMUModel::NONE);
            fpu.setModel(cpModel);
            break;

        case CPUModel::M68030:

            mmu.setModel(MMUModel::M68030);
            fpu.setModel(cpModel);
            break;

        case CPUModel::M68LC040:

            mmu.setModel(MMUModel::M68040);
            break;

        case CPUModel::M68040:

            mmu.setModel(MMUModel::M68040);
            fpu.setModel(FPUModel::M68040);
            break;

        default:
            
            mmu.setModel(MMUModel::NONE);
            break;
    }
}

template <Core C> void
Moira::reset()
{
    flags = State::CHECK_IRQ;

    if constexpr (MOIRA_EMULATE_BUS_ERROR && C == Core::C68020) {
        
        // Background: Bus errors on the 68020 family are restartable. After an RTE,
        // the CPU resumes execution from its internal state. Moira approximates this
        // behavior by restarting the current instruction, which requires the original
        // register state to be available.
        //
        // Note that this comes with a performance penalty: the 68020 must remain on
        // the slow path at all times. Other CPU models are unaffected and stay on the
        // fast path, as they do not require register snapshots.
        
        flags |= State::SAVE_REGS;
    }
    
    reg = { };
    reg.sr.s = 1;
    reg.sr.ipl = 7;

    ipl = 0;
    nmiTaken = false;
    fcl = 2;
    fcSource = 0;
    budget = { };

    iCache.flush();
    dCache.flush();
    iCache.refresh();
    dCache.refresh();

    SYNC(16);

    // Read the initial (supervisor) stack pointer from memory
    SYNC(2);
    reg.sp = read16OnReset(0);
    SYNC(4);
    reg.isp = reg.sp = (read16OnReset(2) & ~0x1) | reg.sp << 16;
    SYNC(4);
    reg.pc = read16OnReset(4);
    SYNC(4);
    reg.pc = (read16OnReset(6) & ~0x1) | reg.pc << 16;

    // Fill the prefetch queue
    SYNC(4);
    queue.irc = read16OnReset(reg.pc & addrMask<C>());
    SYNC(2);
    prefetch<C>();
    reg.pc0 = reg.pc - 2;

    // Reset subcomponents
    debugger.reset();

    // Inform the delegate
    didResetCPU();
}

void
Moira::halt()
{
    // Halt the CPU
    flags |= State::HALTED;
    reg.pc = reg.pc0 + 2;

    // Inform the delegate
    didHaltCPU();
}

void
Moira::syncCyclePenalty(int cycles)
{
    // Max number of instructions that may execute without advancing the clock
    static constexpr int maxStalled = 4;

    // Lower bound for the model time debt
    static constexpr int minAccum = -454;

    // Accumulate the cycle penalty (may become negative)
    budget.accumulated += cycles;

    if (budget.accumulated >= 2) {

        // The CPU is ahead of its environment. Spend everything that amounts
        // to whole bus cycles

        int step = budget.accumulated & ~1;
        budget.accumulated -= step;
        budget.stalled = 0;
        sync(step);

    } else if (++budget.stalled >= maxStalled) {

        // Nothing was spend for a while now. Since the environment is stepped
        // from here alone, it has to be moved forward regardless - otherwise
        // the emulation makes no progress and its main loop never terminates.
        // The borrowed bus cycle is booked as a debt.

        budget.stalled = 0;
        budget.accumulated -= 2;
        if (budget.accumulated < minAccum) budget.accumulated = minAccum;
        sync(2);
    }
}

void
Moira::setFC(u8 value)
{
    if constexpr (MOIRA_EMULATE_FC) {
        
        fcl = (u8)value;
    }
}

bool
Moira::checkForIrq()
{
    // Level 7 is edge triggered. The interrupt can be taken again once the polled level has dropped.
    if (reg.ipl < 7) nmiTaken = false;

    if (reg.ipl > reg.sr.ipl || (reg.ipl == 7 && !nmiTaken)) {

        // Exit loop mode
        if (flags & State::LOOPING) flags &= ~State::LOOPING;

        // Trigger interrupt
        execInterrupt(reg.ipl);

        return true;

    } else {

        // If the polled IPL is up to date, we disable interrupt checking for
        // the time being, because no interrupt can occur as long as the
        // external IPL or the IPL mask inside the status register keep the
        // same. If one of these variables changes, we reenable interrupt
        // checking.
        if (reg.ipl == ipl) flags &= ~State::CHECK_IRQ;

        return false;
    }
}

u16
Moira::getIrqVector(u8 level) {

    assert(level < 8);

    // The interrupt acknowledge cycle is a CPU space cycle
    willExecute(CpuSpaceType::IRQ_ACK, level);

    // Ask the interrupting device for the vector
    u8 userVector = 0;
    u16 vector = 0;

    switch (readIrqVector(level, &userVector)) {

        case IrqMode::AUTO:          vector = 24 + level; break;
        case IrqMode::USER:          vector = userVector; break;
        case IrqMode::SPURIOUS:      vector = 24; break;
        case IrqMode::UNINITIALIZED: vector = 15; break;
    }

    didExecute(CpuSpaceType::IRQ_ACK, level);
    return vector;
}

void
Moira::notifyCPCycle(u16 opcode, CIR cir)
{
    // Coprocessor ID (opcode bits 11-9) on A15-A13, register select on A4-A0
    u8 arg = u8(((opcode >> 9) & 7) << 5 | u8(cir));
    willExecute(CpuSpaceType::COPROCESSOR, arg);
    didExecute(CpuSpaceType::COPROCESSOR, arg);
}

template u32 Moira::readD <Long> (int n) const;
template u32 Moira::readA <Long> (int n) const;
template void Moira::writeD <Long> (int n, u32 v);
template void Moira::writeA <Long> (int n, u32 v);

}
