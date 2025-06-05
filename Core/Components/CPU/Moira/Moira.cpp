// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "config.h"
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

using namespace Flag;

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
    if (MOIRA_BUILD_INSTR_INFO_TABLE) info = new InstrInfo[65536];
    if (MOIRA_ENABLE_DASM) dasm = new DasmPtr[65536];

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
}

void
Moira::setModel(Model cpuModel, Model dasmModel)
{
    if (this->cpuModel != cpuModel || this->dasmModel != dasmModel) {
        
        this->cpuModel = cpuModel;
        this->dasmModel = dasmModel;

        createJumpTable(cpuModel, dasmModel);
        
        reg.cacr &= cacrMask();
        flags &= ~State::LOOPING;
    }
}

void
Moira::setDasmSyntax(Syntax value)
{
    instrStyle.syntax = value;
}

void
Moira::setDasmLetterCase(LetterCase value)
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

        case Model::M68EC020: case Model::M68020: case Model::M68EC030: case Model::M68030:
            return true;

        default:
            return false;
    }
}

bool
Moira::hasMMU() const
{
    switch (cpuModel) {

        case Model::M68030: case Model::M68LC040: case Model::M68040:
            return true;

        default:
            return false;
    }
}

bool
Moira::hasFPU() const
{
    switch (cpuModel) {

        case Model::M68040:
            return true;

        default:
            return false;
    }
}

u32
Moira::cacrMask() const
{
    switch (cpuModel) {

        case Model::M68020: case Model::M68EC020: return 0x0003;
        case Model::M68030: case Model::M68EC030: return 0x3F13;
        
        default:
            return 0xFFFF;
    }
}

u32
Moira::addrMask() const
{
    switch (cpuModel) {

        case Model::M68000: return addrMask<Core::C68000>();
        case Model::M68010: return addrMask<Core::C68010>();
        
        default:
            return addrMask<Core::C68020>();
    }
}

template <Core C> u32
Moira::addrMask() const
{
    if constexpr (C == Core::C68020) {

        return cpuModel == Model::M68EC020 ? 0x00FFFFFF : 0xFFFFFFFF;

    } else {

        return 0x00FFFFFF;
    }
}

void
Moira::reset()
{
    switch (cpuModel) {

        case Model::M68000:    reset<Core::C68000>(); break;
        case Model::M68010:    reset<Core::C68010>(); break;
        
        default:
            reset<Core::C68020>();
    }
}

template <Core C> void
Moira::reset()
{
    flags = State::CHECK_IRQ;

    reg = { };
    reg.sr.s = 1;
    reg.sr.ipl = 7;

    ipl = 0;
    fcl = 2;
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
    using namespace State;

    // Check the integrity of the IRQ flag
    if (reg.ipl > reg.sr.ipl || reg.ipl == 7) assert(flags & CHECK_IRQ);

    // Check the integrity of the trace flag
    assert(!!(flags & TRACING) == reg.sr.t1);

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

        if (flags & (HALTED | TRACE_EXC | TRACING)) {

            // Only continue if the CPU is not halted
            if (flags & HALTED) {
                sync(2);
                return;
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
            }
        }

        // If the CPU is stopped, poll the IPL lines and return
        if (flags & STOPPED) {

            // Initiate a privilege exception if the supervisor bit is cleared
            if (!reg.sr.s) {
                sync(4);
                reg.pc -= 2;
                flags &= ~STOPPED;
                execException(M68kException::PRIVILEGE);
                return;
            }

            POLL_IPL;
            sync(MOIRA_MIMIC_MUSASHI ? 1 : 2);
            return;
        }

        // If logging is enabled, record the executed instruction
        if (flags & LOGGING) {
            debugger.logInstruction();
        }

        // Execute the instruction
        reg.pc += 2;

        if (flags & LOOPING) {

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
        if (flags & CHECK_BP) {

            // Don't break if the instruction won't be executed due to tracing
            if (flags & TRACE_EXC) return;

            // Check if a softstop has been reached
            if (debugger.softstopMatches(reg.pc0)) didReachSoftstop(reg.pc0);

            // Check if a breakpoint has been reached
            if (debugger.breakpointMatches(reg.pc0)) didReachBreakpoint(reg.pc0);
        }
    }

    // Check the integrity of the program counter again
    assert(reg.pc0 == reg.pc);
}

void
Moira::execute(i64 cycles)
{
    executeUntil(clock + cycles);
}

void
Moira::executeUntil(i64 cycle)
{
    while (clock < cycle) { execute(); }
}

void
Moira::processException(const std::exception &exc)
{
    switch (cpuModel) {

        case Model::M68000: processException<Core::C68000>(exc); break;
        case Model::M68010: processException<Core::C68010>(exc); break;
        
        default:
            processException<Core::C68020>(exc);
    }
}

template <Core C> void
Moira::processException(const std::exception &exc)
{
    try {

        if (auto ae = dynamic_cast<const AddressError *>(&exc); ae) {

            execAddressError<C>(ae->stackFrame);
            return;
        }

        if (auto be = dynamic_cast<const BusError *>(&exc); be) {

            execBusError<C>(be->stackFrame);
            return;
        }

        if (auto df = dynamic_cast<const DoubleFault *>(&exc); df) {

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

void
Moira::halt()
{
    // Halt the CPU
    flags |= State::HALTED;
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
    flags |= State::CHECK_IRQ;
    t1 ? setTraceFlag() : clearTraceFlag();

    setCCR(u8(val));
    setSupervisorMode(s);

    if (cpuModel > Model::M68010) {

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

        case Instr::BKPT: case Instr::MOVEC: case Instr::MOVES: case Instr::MOVEFCCR: case Instr::RTD:

            return AV::M68010_UP;

        case Instr::CALLM: case Instr::RTM:

            return AV::M68020;

        case Instr::cpGEN: case Instr::cpRESTORE: case Instr::cpSAVE: case Instr::cpScc: case Instr::cpTRAPcc:

            return AV::M68020 | AV::M68030;

        case Instr::BFCHG: case Instr::BFCLR: case Instr::BFEXTS: case Instr::BFEXTU: case Instr::BFFFO:
        case Instr::BFINS: case Instr::BFSET: case Instr::BFTST: case Instr::CAS: case Instr::CAS2:
        case Instr::CHK2: case Instr::CMP2: case Instr::DIVL: case Instr::EXTB: case Instr::MULL:
        case Instr::PACK: case Instr::TRAPCC: case Instr::TRAPCS: case Instr::TRAPEQ: case Instr::TRAPGE:
        case Instr::TRAPGT: case Instr::TRAPHI: case Instr::TRAPLE: case Instr::TRAPLS: case Instr::TRAPLT:
        case Instr::TRAPMI: case Instr::TRAPNE: case Instr::TRAPPL: case Instr::TRAPVC: case Instr::TRAPVS:
        case Instr::TRAPF: case Instr::TRAPT: case Instr::UNPK:

            return AV::M68020_UP;

        case Instr::CINV: case Instr::CPUSH: case Instr::MOVE16:

            return AV::M68040;

        case Instr::PFLUSH: case Instr::PFLUSHA: case Instr::PFLUSHAN: case Instr::PFLUSHN: case Instr::PLOAD:
        case Instr::PMOVE: case Instr::PTEST:

            return AV::MMU;

        case Instr::FABS: case Instr::FADD: case Instr::FBcc: case Instr::FCMP: case Instr::FDBcc: case Instr::FDIV:
        case Instr::FMOVE: case Instr::FMOVEM: case Instr::FMUL: case Instr::FNEG: case Instr::FNOP:
        case Instr::FRESTORE: case Instr::FSAVE: case Instr::FScc: case Instr::FSQRT: case Instr::FSUB:
        case Instr::FTRAPcc: case Instr::FTST:

        case Instr::FSABS: case Instr::FDABS: case Instr::FSADD: case Instr::FDADD: case Instr::FSDIV: case Instr::FDDIV:
        case Instr::FSMOVE: case Instr::FDMOVE: case Instr::FSMUL: case Instr::FDMUL: case Instr::FSNEG:
        case Instr::FDNEG: case Instr::FSSQRT: case Instr::FDSQRT: case Instr::FSSUB: case Instr::FDSUB:

            return AV::FPU;

        case Instr::FACOS: case Instr::FASIN: case Instr::FATAN: case Instr::FATANH: case Instr::FCOS: case Instr::FCOSH:
        case Instr::FETOX: case Instr::FETOXM1: case Instr::FGETEXP: case Instr::FGETMAN: case Instr::FINT:
        case Instr::FINTRZ: case Instr::FLOG10: case Instr::FLOG2: case Instr::FLOGN: case Instr::FLOGNP1:
        case Instr::FMOD: case Instr::FMOVECR: case Instr::FREM: case Instr::FSCAL: case Instr::FSGLDIV:
        case Instr::FSGLMUL: case Instr::FSIN: case Instr::FSINCOS: case Instr::FSINH: case Instr::FTAN:
        case Instr::FTANH: case Instr::FTENTOX: case Instr::FTWOTOX:

            return 0; // M6888x only

        default:

            return AV::M68000_UP;
    }
}

u16
Moira::availabilityMask(Instr I, Mode M, Size S) const
{
    u16 mask = availabilityMask(I);

    switch (I) {

        case Instr::CMPI:

            if (isPrgMode(M)) mask &= AV::M68010_UP;
            break;

        case Instr::CHK: case Instr::LINK: case Instr::BRA: case Instr::BHI: case Instr::BLS: case Instr::BCC: case Instr::BCS:
        case Instr::BNE: case Instr::BEQ: case Instr::BVC: case Instr::BVS: case Instr::BPL: case Instr::BMI: case Instr::BGE:
        case Instr::BLT: case Instr::BGT: case Instr::BLE: case Instr::BSR:

            if (S == Long) mask &= AV::M68020_UP;
            break;

        case Instr::TST:

            if (M == Mode(1) || M >= Mode(9)) mask &= AV::M68020_UP;
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

        case Instr::MOVEC:

            switch (ext & 0x0FFF) {

                case 0x000:
                case 0x001:
                case 0x800:
                case 0x801: mask &= AV::M68010_UP; break;
                case 0x002:
                case 0x803:
                case 0x804: mask &=  AV::M68020_UP; break;
                case 0x802: mask &=  AV::M68020 | AV::M68030; break;
                case 0x003:
                case 0x004:
                case 0x005:
                case 0x006:
                case 0x007:
                case 0x805:
                case 0x806:
                case 0x807: mask &= AV::M68040; break;

                default:
                    break;
            }
            break;

        case Instr::MOVES:

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
    return availabilityMask(I) & (1 << (int)model);
}

bool
Moira::isAvailable(Model model, Instr I, Mode M, Size S) const
{
    return availabilityMask(I, M, S) & (1 << (int)model);
}

bool
Moira::isAvailable(Model model, Instr I, Mode M, Size S, u16 ext) const
{
    return availabilityMask(I, M, S, ext) & (1 << (int)model);
}

const char *
Moira::availabilityString(Instr I, Mode M, Size S, u16 ext)
{
    switch (availabilityMask(I, M, S, ext)) {

        case AV::M68010_UP:           return "(1+)";
        case AV::M68020:              return "(2)";
        case AV::M68020 | AV::M68030: return "(2-3)";
        case AV::M68020_UP:           return "(2+)";
        case AV::M68040:              return "(4+)";

        default:
            return "(?)";
    }
}

bool
Moira::isValidExt(Instr I, Mode M, u16 op, u32 ext) const
{
    switch (I) {

        case Instr::BFCHG:     return (ext & 0xF000) == 0;
        case Instr::BFCLR:     return (ext & 0xF000) == 0;
        case Instr::BFEXTS:    return (ext & 0x8000) == 0;
        case Instr::BFEXTU:    return (ext & 0x8000) == 0;
        case Instr::BFFFO:     return (ext & 0x8000) == 0;
        case Instr::BFINS:     return (ext & 0x8000) == 0;
        case Instr::BFSET:     return (ext & 0xF000) == 0;
        case Instr::BFTST:     return (ext & 0xF000) == 0;
        case Instr::CAS:       return (ext & 0xFE38) == 0;
        case Instr::CAS2:      return (ext & 0x0E380E38) == 0;
        case Instr::CHK2:      return (ext & 0x07FF) == 0;
        case Instr::CMP2:      return (ext & 0x0FFF) == 0;
        case Instr::MULL:      return (ext & 0x83F8) == 0;
        case Instr::DIVL:      return (ext & 0x83F8) == 0;

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
    if constexpr (MOIRA_EMULATE_FC) {
        
        fcl = (u8)value;
    }
}

template <Mode M> void
Moira::setFC()
{
    if constexpr (MOIRA_EMULATE_FC) {
        
        fcl = (M == Mode::DIPC || M == Mode::IXPC) ? FC::USER_PROG : FC::USER_DATA;
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

u16
Moira::getIrqVector(u8 level) const {

    assert(level < 8);

    switch (irqMode) {

        case IrqMode::AUTO:          return 24 + level;
        case IrqMode::USER:          return readIrqUserVector(level) & 0xFF;
        case IrqMode::SPURIOUS:      return 24;
        case IrqMode::UNINITIALIZED: return 15;

        default:
            fatalError;
    }
}

InstrInfo
Moira::getInstrInfo(u16 op) const
{
    if constexpr (MOIRA_BUILD_INSTR_INFO_TABLE) {

        return info[op];

    } else {

        throw std::runtime_error("This feature requires MOIRA_BUILD_INSTR_INFO_TABLE = true\n");
    }
}

template u32 Moira::readD <Long> (int n) const;
template u32 Moira::readA <Long> (int n) const;
template void Moira::writeD <Long> (int n, u32 v);
template void Moira::writeA <Long> (int n, u32 v);

}
