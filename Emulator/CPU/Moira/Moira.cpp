// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "config.h"
#include "MoiraConfig.h"
#include "Moira.h"
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <bit>
#include <vector>
#include <stdexcept>

namespace moira {

#include "MoiraMacros.h"
#include "MoiraInit_cpp.h"
#include "MoiraALU_cpp.h"
#include "MoiraDataflow_cpp.h"
#include "MoiraExceptions_cpp.h"
#include "MoiraExec_cpp.h"
#include "StrWriter_cpp.h"
#include "MoiraDasm_cpp.h"

Moira::Moira(Amiga &ref) : SubComponent(ref)
{
    if (BUILD_INSTR_INFO_TABLE) info = new InstrInfo[65536];
    if (ENABLE_DASM) dasm = new DasmPtr[65536];
    
    createJumpTable();
}

Moira::~Moira()
{
    if (info) delete [] info;
    if (dasm) delete [] dasm;
}

void
Moira::setModel(Model model)
{
    if (this->model != model) {
        
        this->model = model;
        createJumpTable();

        reg.cacr &= cacrMask();
        flags &= ~CPU_IS_LOOPING;
    }
}

void
Moira::setDasmStyle(DasmStyle value)
{
    style = value;
}

void
Moira::setDasmNumberFormat(DasmNumberFormat value)
{
    if (value.prefix == nullptr) {
        throw std::runtime_error("prefix must not be NULL");
    }
    if (value.radix != 10 && value.radix != 16) {
        throw std::runtime_error("Invalid radix: " + std::to_string(value.radix));
    }
    
    numberFormat = value;
}

void
Moira::setDasmLetterCase(DasmLetterCase value)
{
    letterCase = value;
}

void
Moira::setIndentation(int value)
{
    tab = Tab{value};
}

bool
Moira::hasCPI()
{
    switch (model) {

        case M68EC020: case M68020: case M68EC030: case M68030:
            return true;

        default:
            return false;
    }
}

bool
Moira::hasMMU()
{
    switch (model) {

        case M68030: case M68LC040: case M68040:
            return true;

        default:
            return false;
    }
}

bool
Moira::hasFPU()
{
    switch (model) {

        case M68040:
            return true;

        default:
            return false;
    }
}

template <Core C> u32
Moira::addrMask() const
{
    if constexpr (C == C68020) {
        
        return model == M68EC020 ? 0x00FFFFFF : 0xFFFFFFFF;
    }

    return 0x00FFFFFF;
}

u32
Moira::cacrMask() const
{
    switch (model) {

        case M68020: case M68EC020: return 0x0003;
        case M68030: case M68EC030: return 0x3F13;
        default:                    return 0xFFFF;
    }
}

void
Moira::reset()
{
    switch (model) {

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

    fpu = { };

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
    
    debugger.reset();
}

void
Moira::execute()
{
    // Check the integrity of the CPU flags
    if (reg.ipl > reg.sr.ipl || reg.ipl == 7) assert(flags & CPU_CHECK_IRQ);
    assert(!!(flags & CPU_TRACE_FLAG) == reg.sr.t1);
    
    // Check the integrity of the program counter
    assert(reg.pc0 == reg.pc);
    
    //
    // The quick execution path: Call the instruction handler and return
    //
    
    if (!flags) {
        
        reg.pc += 2;
        try {
            (this->*exec[queue.ird])(queue.ird);
        } catch (const BusErrorException &) {
            // TODO: TRIGGER DOUBLE FAULT IF ANOTHER EXCEPTION OCCURS
            execException(EXC_BUS_ERROR);
        } catch (const AddressErrorException &) {
            // TODO: TRIGGER DOUBLE FAULT IF ANOTHER EXCEPTION OCCURS
        }
        
        assert(reg.pc0 == reg.pc);
        return;
    }
    
    //
    // The slow execution path: Process flags one by one
    //
    
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
    
    // Process pending interrupt (if any)
    if (flags & CPU_CHECK_IRQ) {
        if (checkForIrq()) goto done;
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
        
        pollIpl();
        sync(MIMIC_MUSASHI ? 1 : 2);
        return;
    }
    
    // If logging is enabled, record the executed instruction
    if (flags & CPU_LOG_INSTRUCTION) {
        debugger.logInstruction();
    }
    
    // Execute the instruction
    if (flags & CPU_IS_LOOPING) {
        
        reg.pc += 2;
        if (loop[queue.ird] == nullptr) {
            printf("Callback missing [%d]\n", queue.ird);
            breakpointReached(reg.pc0);
        } else {
            (this->*loop[queue.ird])(queue.ird);
            assert(reg.pc0 == reg.pc);
        }
    } else {
        
        reg.pc += 2;
        try {
            (this->*exec[queue.ird])(queue.ird);
        } catch (const BusErrorException &) {
            // TODO: TRIGGER DOUBLE FAULT IF ANOTHER EXCEPTION OCCURS
            execException(EXC_BUS_ERROR);
        } catch (const AddressErrorException &) {
            // TODO: TRIGGER DOUBLE FAULT IF ANOTHER EXCEPTION OCCURS
        } catch (...) { }

        assert(reg.pc0 == reg.pc);
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

bool
Moira::checkForIrq()
{
    if (reg.ipl > reg.sr.ipl || reg.ipl == 7) {
        
        // Exit loop mode if necessary
        if (flags & CPU_IS_LOOPING) {
            flags &= ~CPU_IS_LOOPING;
        }
        
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
    signalHalt();
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

u8
Moira::getCCR(const StatusRegister &sr) const
{
    return (u8)(sr.c << 0 | sr.v << 1 | sr.z << 2 | sr.n << 3 | sr.x << 4);
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
Moira::getSR(const StatusRegister &sr) const
{
    auto flags =
    sr.t1  << 15 |
    sr.t0  << 14 |
    sr.s   << 13 |
    sr.m   << 12 |
    sr.ipl <<  8 | getCCR();
    
    return u16(flags);
}

void
Moira::setSR(u16 val)
{
    bool t1 = (val >> 15) & 1;
    bool s = (val >> 13) & 1;
    u8 ipl = (val >> 8) & 7;
    
    reg.sr.ipl = ipl;
    flags |= CPU_CHECK_IRQ;
    t1 ? setTraceFlag() : clearTraceFlag();
    
    setCCR((u8)val);
    setSupervisorMode(s);
    
    if (model > M68010) {
        
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

u16
Moira::availabilityMask(Instr I)
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
Moira::availabilityMask(Instr I, Mode M, Size S)
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

u16 Moira::availabilityMask(Instr I, Mode M, Size S, u16 ext)
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
Moira::isAvailable(Instr I)
{
    return availabilityMask(I) & (1 << model);
}

bool
Moira::isAvailable(Instr I, Mode M, Size S)
{
    return availabilityMask(I, M, S) & (1 << model);
}

bool
Moira::isAvailable(Instr I, Mode M, Size S, u16 ext)
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
Moira::isValidExt(Instr I, Mode M, u16 op, u32 ext)
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

FunctionCode
Moira::readFC() const
{
    switch (fcSource) {
            
        case 0: return FunctionCode((reg.sr.s ? 4 : 0) | fcl);
        case 1: return FunctionCode(reg.sfc);
        case 2: return FunctionCode(reg.dfc);
            
        default:
            fatalError;
    }
}

void
Moira::setFC(FunctionCode value)
{
    if (!EMULATE_FC) return;
    fcl = (u8)value;
}

template <Mode M> void
Moira::setFC()
{
    if (!EMULATE_FC) return;
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
    }
    
    assert(false);
    return 0;
}

int
Moira::disassemble(u32 addr, char *str, DasmStyle core)
{
    if constexpr (!ENABLE_DASM) {
        
        printf("This feature requires ENABLE_DASM = true\n");
        assert(false);
        return 0;
    }
    
    u32 pc     = addr;
    u16 opcode = read16Dasm(pc);
    
    StrWriter writer(str, style, numberFormat);
    
    (this->*dasm[opcode])(writer, pc, opcode);
    writer << Finish{};
    
    // Post process disassembler output
    switch (letterCase) {
            
        case DASM_MIXED_CASE:
            
            break;
            
        case DASM_LOWER_CASE:
            
            for (auto p = writer.base; p < writer.ptr; p++) {
                *p = char(std::tolower(*p));
            }
            break;
            
        case DASM_UPPER_CASE:
            
            for (auto p = writer.base; p < writer.ptr; p++) {
                *p = char(std::toupper(*p));
            }
            break;
    }
    
    return pc - addr + 2;
}

void
Moira::disassembleWord(u32 value, char *str)
{
    sprintx(str, value, { .prefix = "", .radix = 16, .upperCase = true }, 4);
}

void
Moira::disassembleMemory(u32 addr, int cnt, char *str)
{
    U32_DEC(addr, 2); // Because dasmRead increases addr first
    
    for (int i = 0; i < cnt; i++) {
        u32 value = dasmRead<Word>(addr);
        sprintx(str, value, { .prefix = "", .radix = 16, .upperCase = true }, 4);
        *str++ = (i == cnt - 1) ? 0 : ' ';
    }
}

void
Moira::disassemblePC(u32 pc, char *str)
{
    sprintx(str, pc, { .prefix = "", .radix = 16, .upperCase = true }, 6);
}

void
Moira::disassembleSR(const StatusRegister &sr, char *str)
{
    str[0]  = sr.t1 ? 'T' : 't';
    str[1]  = sr.t0 ? 'T' : 't';
    str[2]  = sr.s ? 'S' : 's';
    str[3]  = sr.m ? 'M' : 'm';
    str[4]  = '-';
    str[5]  = (sr.ipl & 0b100) ? '1' : '0';
    str[6]  = (sr.ipl & 0b010) ? '1' : '0';
    str[7]  = (sr.ipl & 0b001) ? '1' : '0';
    str[8]  = '-';
    str[9]  = '-';
    str[10] = '-';
    str[11] = sr.x ? 'X' : 'x';
    str[12] = sr.n ? 'N' : 'n';
    str[13] = sr.z ? 'Z' : 'z';
    str[14] = sr.v ? 'V' : 'v';
    str[15] = sr.c ? 'C' : 'c';
    str[16] = 0;
}

/*
 void
 Moira::disassembleSR(u16 sr, char *str)
 {
 str[0]  = (sr & 0b1000000000000000) ? 'T' : 't';
 str[1]  = '-';
 str[2]  = (sr & 0b0010000000000000) ? 'S' : 's';
 str[3]  = '-';
 str[4]  = '-';
 str[5]  = (sr & 0b0000010000000000) ? '1' : '0';
 str[6]  = (sr & 0b0000001000000000) ? '1' : '0';
 str[7]  = (sr & 0b0000000100000000) ? '1' : '0';
 str[8]  = '-';
 str[9]  = '-';
 str[10] = '-';
 str[11] = (sr & 0b0000000000010000) ? 'X' : 'x';
 str[12] = (sr & 0b0000000000001000) ? 'N' : 'n';
 str[13] = (sr & 0b0000000000000100) ? 'Z' : 'z';
 str[14] = (sr & 0b0000000000000010) ? 'V' : 'v';
 str[15] = (sr & 0b0000000000000001) ? 'C' : 'c';
 str[16] = 0;
 }
 */

InstrInfo
Moira::getInfo(u16 op)
{
    if (BUILD_INSTR_INFO_TABLE == false) {
        
        printf("This feature requires BUILD_INSTR_INFO_TABLE = true\n");
        assert(false);
        return InstrInfo { ILLEGAL, MODE_IP, (Size)0 };
    }
    
    return info[op];
}

template u32 Moira::readD <Long> (int n) const;
template u32 Moira::readA <Long> (int n) const;
template void Moira::writeD <Long> (int n, u32 v);
template void Moira::writeA <Long> (int n, u32 v);

}
