// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "MoiraConfig.h"
#include "Moira.h"
#include "MoiraMacros.h"

#include <cstdio>
#include <algorithm>
#include <cmath>
#include <bit>
#include <vector>
#include <stdexcept>

namespace vamiga::moira {

#include "MoiraInit_cpp.h"
#include "MoiraALU_cpp.h"
#include "MoiraDataflow_cpp.h"
#include "MoiraExceptions_cpp.h"
#include "MoiraExec_cpp.h"
#include "StrWriter_cpp.h"
#include "MoiraDasm_cpp.h"

Moira::Moira(Amiga &ref) : SubComponent(ref)
{
    exec = new ExecPtr[65536];
    loop = new ExecPtr[65536];
    if (BUILD_INSTR_INFO_TABLE) info = new InstrInfo[65536];
    if (ENABLE_DASM) dasm = new DasmPtr[65536];

    createJumpTable(cpuModel, dasmModel);

    instrStyle = DasmStyle {

        .syntax         = DASM_MOIRA,
        .letterCase     = DASM_MIXED_CASE,
        .numberFormat   = { .prefix = "$", .radix = 16, .upperCase = false, .plainZero = false },
        .tab            = 8
    };

    dataStyle = DasmStyle {

        .syntax         = DASM_MOIRA,
        .letterCase     = DASM_MIXED_CASE,
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
}

void
Moira::setModel(Model cpuModel, Model dasmModel)
{
    if (this->cpuModel != cpuModel || this->dasmModel != dasmModel) {
        
        this->cpuModel = cpuModel;
        this->dasmModel = dasmModel;

        createJumpTable(cpuModel, dasmModel);
        
        reg.cacr &= cacrMask();
        flags &= ~CPU_IS_LOOPING;
    }
}

void
Moira::setDasmSyntax(DasmSyntax value)
{
    instrStyle.syntax = value;
}

void
Moira::setDasmLetterCase(DasmLetterCase value)
{
    instrStyle.letterCase = value;
}

void
Moira::setNumberFormat(DasmStyle &style, const DasmNumberFormat &value)
{
    auto validPrefix = [&](DasmNumberFormat fmt) { return fmt.prefix != nullptr; };
    auto validRadix = [&](DasmNumberFormat fmt) { return fmt.radix == 10 || fmt.radix == 16; };

    if (!validPrefix(value)) {
        throw std::runtime_error("prefix must not be NULL");
    }
    if (!validRadix(value)) {
        throw std::runtime_error("radix must be 10 or 16");
    }

    style.numberFormat = value;
}

bool
Moira::hasCPI() const
{
    switch (cpuModel) {

        case M68EC020: case M68020: case M68EC030: case M68030:
            return true;

        default:
            return false;
    }
}

bool
Moira::hasMMU() const
{
    switch (cpuModel) {

        case M68030: case M68LC040: case M68040:
            return true;

        default:
            return false;
    }
}

bool
Moira::hasFPU() const
{
    switch (cpuModel) {

        case M68040:
            return true;

        default:
            return false;
    }
}

u32
Moira::cacrMask() const
{
    switch (cpuModel) {

        case M68020: case M68EC020: return 0x0003;
        case M68030: case M68EC030: return 0x3F13;
        default:                    return 0xFFFF;
    }
}

u32
Moira::addrMask() const
{
    switch (cpuModel) {

        case M68000:    return addrMask<C68000>();
        case M68010:    return addrMask<C68010>();
        default:        return addrMask<C68020>();
    }
}

template <Core C> u32
Moira::addrMask() const
{
    if constexpr (C == C68020) {

        return cpuModel == M68EC020 ? 0x00FFFFFF : 0xFFFFFFFF;

    } else {

        return 0x00FFFFFF;
    }
}

void
Moira::reset()
{
    switch (cpuModel) {

        case M68000:    reset<C68000>(); break;
        case M68010:    reset<C68010>(); break;
        default:        reset<C68020>(); break;
    }
}

template <Core C> void
Moira::reset()
{
    flags = CPU_CHECK_IRQ;

    reg = { };
    reg.sr.s = 1;
    reg.sr.ipl = 7;

    ipl = 0;
    fcl = 0;
    fcSource = 0;

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

    // Reset subcomponents
    debugger.reset();

    // Inform the delegate
    cpuDidReset();
}

void
Moira::execute()
{
    // Check the integrity of the IRQ flag
    if (reg.ipl > reg.sr.ipl || reg.ipl == 7) assert(flags & CPU_CHECK_IRQ);

    // Check the integrity of the trace flag
    assert(!!(flags & CPU_TRACE_FLAG) == reg.sr.t1);

    // Check the integrity of the program counter
    assert(reg.pc0 == reg.pc);

    // Take the fast path or the slow path
    if (!flags) {

        //
        // Fast path: Call the instruction handler and return
        //

        reg.pc += 2;
        try {
            (this->*exec[queue.ird])(queue.ird);
        } catch (const std::exception &exc) {
            processException(exc);
        }

    } else {

        //
        // Slow path: Process flags one by one
        //

        if (flags & (CPU_IS_HALTED | CPU_TRACE_EXCEPTION | CPU_TRACE_FLAG)) {

            // Only continue if the CPU is not halted
            if (flags & CPU_IS_HALTED) {
                sync(2);
                return;
            }

            // Process pending trace exception (if any)
            if (flags & CPU_TRACE_EXCEPTION) {
                execException(EXC_TRACE);
                goto done;
            }

            // Check if the T flag is set inside the status register
            if ((flags & CPU_TRACE_FLAG) && !(flags & CPU_IS_STOPPED)) {
                flags |= CPU_TRACE_EXCEPTION;
            }
        }

        // Process pending interrupt (if any)
        if (flags & CPU_CHECK_IRQ) {

            try {
                if (checkForIrq()) goto done;
            } catch (const std::exception &exc) {
                processException(exc);
            }
        }

        // If the CPU is stopped, poll the IPL lines and return
        if (flags & CPU_IS_STOPPED) {

            // Initiate a privilege exception if the supervisor bit is cleared
            if (!reg.sr.s) {
                sync(4);
                reg.pc -= 2;
                flags &= ~CPU_IS_STOPPED;
                execException(EXC_PRIVILEGE);
                return;
            }

            POLL_IPL;
            sync(MIMIC_MUSASHI ? 1 : 2);
            return;
        }

        // If logging is enabled, record the executed instruction
        if (flags & CPU_LOG_INSTRUCTION) {
            debugger.logInstruction();
        }

        // Execute the instruction
        reg.pc += 2;

        if (flags & CPU_IS_LOOPING) {

            assert(loop[queue.ird]);
            (this->*loop[queue.ird])(queue.ird);

        } else {

            try {
                (this->*exec[queue.ird])(queue.ird);
            } catch (const std::exception &exc) {
                processException(exc);
            }
        }

    done:

        // Check if a breakpoint has been reached
        if (flags & CPU_CHECK_BP) {

            // Don't break if the instruction won't be executed due to tracing
            if (flags & CPU_TRACE_EXCEPTION) return;

            // Check if a softstop has been reached
            if (debugger.softstopMatches(reg.pc0)) softstopReached(reg.pc0);

            // Check if a breakpoint has been reached
            if (debugger.breakpointMatches(reg.pc0)) breakpointReached(reg.pc0);
        }
    }

    // Check the integrity of the program counter again
    assert(reg.pc0 == reg.pc);
}

void
Moira::processException(const std::exception &exc)
{
    switch (cpuModel) {

        case M68000:    processException<C68000>(exc); break;
        case M68010:    processException<C68010>(exc); break;
        default:        processException<C68020>(exc); break;
    }
}

template <Core C> void
Moira::processException(const std::exception &exc)
{
    try {

        auto ae = dynamic_cast<const AddressError *>(&exc);
        if (ae) {

            execAddressError<C>(ae->stackFrame);
            return;
        }

        auto be = dynamic_cast<const BusErrorException *>(&exc);
        if (be) {

            execException(EXC_BUS_ERROR);
            return;
        }

        auto df = dynamic_cast<const DoubleFault *>(&exc);
        if (df) {

            throw df;
        }

    } catch (DoubleFault &df) {

        halt();
        return;
    }

    throw exc;
}

bool
Moira::checkForIrq()
{
    if (reg.ipl > reg.sr.ipl || reg.ipl == 7) {

        // Exit loop mode
        if (flags & CPU_IS_LOOPING) flags &= ~CPU_IS_LOOPING;

        // Trigger interrupt
        execInterrupt(reg.ipl);

        return true;

    } else {

        // If the polled IPL is up to date, we disable interrupt checking for
        // the time being, because no interrupt can occur as long as the
        // external IPL or the IPL mask inside the status register keep the
        // same. If one of these variables changes, we reenable interrupt
        // checking.
        if (reg.ipl == ipl) flags &= ~CPU_CHECK_IRQ;

        return false;
    }
}

void
Moira::halt()
{
    // Halt the CPU
    flags |= CPU_IS_HALTED;
    reg.pc = reg.pc0;

    // Inform the delegate
    cpuDidHalt();
}

u8
Moira::getCCR() const
{
    auto result =
    reg.sr.c << 0 |
    reg.sr.v << 1 |
    reg.sr.z << 2 |
    reg.sr.n << 3 |
    reg.sr.x << 4 ;

    return u8(result);
}

void
Moira::setCCR(u8 val)
{
    reg.sr.c = (val >> 0) & 1;
    reg.sr.v = (val >> 1) & 1;
    reg.sr.z = (val >> 2) & 1;
    reg.sr.n = (val >> 3) & 1;
    reg.sr.x = (val >> 4) & 1;
}

u16
Moira::getSR() const
{
    auto flags =
    reg.sr.t1  << 15 |
    reg.sr.t0  << 14 |
    reg.sr.s   << 13 |
    reg.sr.m   << 12 |
    reg.sr.ipl <<  8 ;

    return u16(flags | getCCR());
}

void
Moira::setSR(u16 val)
{
    bool t1  = (val >> 15) & 1;
    bool s   = (val >> 13) & 1;
    u8   ipl = (val >>  8) & 7;

    reg.sr.ipl = ipl;
    flags |= CPU_CHECK_IRQ;
    t1 ? setTraceFlag() : clearTraceFlag();

    setCCR(u8(val));
    setSupervisorMode(s);

    if (cpuModel > M68010) {

        bool t0 = (val >> 14) & 1;
        bool m = (val >> 12) & 1;

        t0 ? setTrace0Flag() : clearTrace0Flag();
        setMasterMode(m);
    }
}

void
Moira::setCACR(u32 val)
{
    reg.cacr = val & cacrMask();
    didChangeCACR(val);
}

void
Moira::setCAAR(u32 val)
{
    reg.caar = val;
    didChangeCAAR(val);
}

void
Moira::setSupervisorMode(bool s)
{
    if (s != reg.sr.s) setSupervisorFlags(s, reg.sr.m);
}

void
Moira::setMasterMode(bool m)
{
    if (m != reg.sr.m) setSupervisorFlags(reg.sr.s, m);
}

void
Moira::setSupervisorFlags(bool s, bool m)
{
    bool uspWasVisible = !reg.sr.s;
    bool ispWasVisible =  reg.sr.s && !reg.sr.m;
    bool mspWasVisible =  reg.sr.s &&  reg.sr.m;

    if (uspWasVisible) reg.usp = reg.sp;
    if (ispWasVisible) reg.isp = reg.sp;
    if (mspWasVisible) reg.msp = reg.sp;

    reg.sr.s = s;
    reg.sr.m = m;

    bool uspIsVisible  = !reg.sr.s;
    bool ispIsVisible  =  reg.sr.s && !reg.sr.m;
    bool mspIsVisible  =  reg.sr.s &&  reg.sr.m;

    if (uspIsVisible)  reg.sp = reg.usp;
    if (ispIsVisible)  reg.sp = reg.isp;
    if (mspIsVisible)  reg.sp = reg.msp;
}

template <Size S> u32
Moira::readD(int n) const
{
    return CLIP<S>(reg.d[n]);
}

template <Size S> u32
Moira::readA(int n) const
{
    return CLIP<S>(reg.a[n]);
}

template <Size S> u32
Moira::readR(int n) const
{
    return CLIP<S>(reg.r[n]);
}

template <Size S> void
Moira::writeD(int n, u32 v)
{
    reg.d[n] = WRITE<S>(reg.d[n], v);
}

template <Size S> void
Moira::writeA(int n, u32 v)
{
    reg.a[n] = WRITE<S>(reg.a[n], v);
}

template <Size S> void
Moira::writeR(int n, u32 v)
{
    reg.r[n] = WRITE<S>(reg.r[n], v);
}

u16
Moira::availabilityMask(Instr I) const
{

    switch (I) {

        case BKPT: case MOVEC: case MOVES: case MOVEFCCR: case RTD:

            return AV_68010_UP;

        case CALLM: case RTM:

            return AV_68020;

        case cpGEN: case cpRESTORE: case cpSAVE: case cpScc: case cpTRAPcc:

            return AV_68020 | AV_68030;

        case BFCHG: case BFCLR: case BFEXTS: case BFEXTU: case BFFFO:
        case BFINS: case BFSET: case BFTST: case CAS: case CAS2:
        case CHK2: case CMP2: case DIVL: case EXTB: case MULL:
        case PACK: case TRAPCC: case TRAPCS: case TRAPEQ: case TRAPGE:
        case TRAPGT: case TRAPHI: case TRAPLE: case TRAPLS: case TRAPLT:
        case TRAPMI: case TRAPNE: case TRAPPL: case TRAPVC: case TRAPVS:
        case TRAPF: case TRAPT: case UNPK:

            return AV_68020_UP;

        case CINV: case CPUSH: case MOVE16:

            return AV_68040;

        case PFLUSH: case PFLUSHA: case PFLUSHAN: case PFLUSHN: case PLOAD:
        case PMOVE: case PTEST:

            return AV_MMU;

        case FABS: case FADD: case FBcc: case FCMP: case FDBcc: case FDIV:
        case FMOVE: case FMOVEM: case FMUL: case FNEG: case FNOP:
        case FRESTORE: case FSAVE: case FScc: case FSQRT: case FSUB:
        case FTRAPcc: case FTST:

        case FSABS: case FDABS: case FSADD: case FDADD: case FSDIV: case FDDIV:
        case FSMOVE: case FDMOVE: case FSMUL: case FDMUL: case FSNEG:
        case FDNEG: case FSSQRT: case FDSQRT: case FSSUB: case FDSUB:

            return AV_FPU;

        case FACOS: case FASIN: case FATAN: case FATANH: case FCOS: case FCOSH:
        case FETOX: case FETOXM1: case FGETEXP: case FGETMAN: case FINT:
        case FINTRZ: case FLOG10: case FLOG2: case FLOGN: case FLOGNP1:
        case FMOD: case FMOVECR: case FREM: case FSCAL: case FSGLDIV:
        case FSGLMUL: case FSIN: case FSINCOS: case FSINH: case FTAN:
        case FTANH: case FTENTOX: case FTWOTOX:

            return 0; // M6888x only

        default:

            return AV_68000_UP;
    }
}

u16
Moira::availabilityMask(Instr I, Mode M, Size S) const
{
    u16 mask = availabilityMask(I);

    switch (I) {

        case CMPI:

            if (isPrgMode(M)) mask &= AV_68010_UP;
            break;

        case CHK: case LINK: case BRA: case BHI: case BLS: case BCC: case BCS:
        case BNE: case BEQ: case BVC: case BVS: case BPL: case BMI: case BGE:
        case BLT: case BGT: case BLE: case BSR:

            if (S == Long) mask &= AV_68020_UP;
            break;

        case TST:

            if (M == 1 || M >= 9) mask &= AV_68020_UP;
            break;

        default:

            break;
    }

    return mask;
}

u16 Moira::availabilityMask(Instr I, Mode M, Size S, u16 ext) const
{
    u16 mask = availabilityMask(I);

    switch (I) {

        case MOVEC:

            switch (ext & 0x0FFF) {

                case 0x000:
                case 0x001:
                case 0x800:
                case 0x801: mask &= AV_68010_UP; break;
                case 0x002:
                case 0x803:
                case 0x804: mask &=  AV_68020_UP; break;
                case 0x802: mask &=  AV_68020 | AV_68030; break;
                case 0x003:
                case 0x004:
                case 0x005:
                case 0x006:
                case 0x007:
                case 0x805:
                case 0x806:
                case 0x807: mask &= AV_68040; break;

                default:
                    break;
            }
            break;

        case MOVES:

            if (ext & 0x7FF) mask = 0;
            break;

        default:
            break;
    }

    return mask;
}

bool
Moira::isAvailable(Model model, Instr I) const
{
    return availabilityMask(I) & (1 << model);
}

bool
Moira::isAvailable(Model model, Instr I, Mode M, Size S) const
{
    return availabilityMask(I, M, S) & (1 << model);
}

bool
Moira::isAvailable(Model model, Instr I, Mode M, Size S, u16 ext) const
{
    return availabilityMask(I, M, S, ext) & (1 << model);
}

const char *
Moira::availabilityString(Instr I, Mode M, Size S, u16 ext)
{
    switch (availabilityMask(I, M, S, ext)) {

        case AV_68010_UP:           return "(1+)";
        case AV_68020:              return "(2)";
        case AV_68020 | AV_68030:   return "(2-3)";
        case AV_68020_UP:           return "(2+)";
        case AV_68040:              return "(4+)";

        default:
            return "(?)";
    }
}

bool
Moira::isValidExt(Instr I, Mode M, u16 op, u32 ext) const
{
    switch (I) {

        case BFCHG:     return (ext & 0xF000) == 0;
        case BFCLR:     return (ext & 0xF000) == 0;
        case BFEXTS:    return (ext & 0x8000) == 0;
        case BFEXTU:    return (ext & 0x8000) == 0;
        case BFFFO:     return (ext & 0x8000) == 0;
        case BFINS:     return (ext & 0x8000) == 0;
        case BFSET:     return (ext & 0xF000) == 0;
        case BFTST:     return (ext & 0xF000) == 0;
        case CAS:       return (ext & 0xFE38) == 0;
        case CAS2:      return (ext & 0x0E380E38) == 0;
        case CHK2:      return (ext & 0x07FF) == 0;
        case CMP2:      return (ext & 0x0FFF) == 0;
        case MULL:      return (ext & 0x83F8) == 0;
        case DIVL:      return (ext & 0x83F8) == 0;

        default:
            fatalError;
    }
}

u8
Moira::readFC() const
{
    switch (fcSource) {

        case 0: return u8((reg.sr.s ? 4 : 0) | fcl);
        case 1: return u8(reg.sfc);
        case 2: return u8(reg.dfc);

        default:
            fatalError;
    }
}

void
Moira::setFC(u8 value)
{
    if (!EMULATE_FC) return;

    fcl = (u8)value;
}

template <Mode M> void
Moira::setFC()
{
    if (!EMULATE_FC)  return;

    fcl = (M == MODE_DIPC || M == MODE_IXPC) ? FC_USER_PROG : FC_USER_DATA;
}

void
Moira::setIPL(u8 val)
{
    if (ipl != val) {
        ipl = val;
        flags |= CPU_CHECK_IRQ;
    }
}

u16
Moira::getIrqVector(u8 level) const {

    assert(level < 8);

    switch (irqMode) {

        case IRQ_AUTO:          return 24 + level;
        case IRQ_USER:          return readIrqUserVector(level) & 0xFF;
        case IRQ_SPURIOUS:      return 24;
        case IRQ_UNINITIALIZED: return 15;

        default:
            fatalError;
    }
}

InstrInfo
Moira::getInstrInfo(u16 op) const
{
    if constexpr (BUILD_INSTR_INFO_TABLE) {

        return info[op];

    } else {

        throw std::runtime_error("This feature requires BUILD_INSTR_INFO_TABLE = true\n");
    }
}

template u32 Moira::readD <Long> (int n) const;
template u32 Moira::readA <Long> (int n) const;
template void Moira::writeD <Long> (int n, u32 v);
template void Moira::writeA <Long> (int n, u32 v);

}
