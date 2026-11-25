// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

// The following definitions are not templated. They are compiled into the
// main translation unit only (see MoiraCore_cpp.h).
#ifdef MOIRA_MAIN_TU

u8
Moira::mmuFC(u16 ext) const
{
    switch (ext & 0x18) {

        case 0x10: return ext & 7;
        case 0x08: return reg.d[ext & 7] & 7;
        default:   return (ext & 1) ? reg.dfc : reg.sfc;
    }
}

bool
Moira::isValidMovecReg(u16 rc) const
{
    // The 68010 supports SFC, DFC, USP, and VBR only
    if (cpuModel == CPUModel::M68010) {
        return rc == 0x000 || rc == 0x001 || rc == 0x800 || rc == 0x801;
    }

    // The 68040 has no CAAR but a set of MMU registers
    if (cpuModel >= CPUModel::M68EC040) {
        return rc <= 0x007 || rc == 0x800 || rc == 0x801 || (rc >= 0x803 && rc <= 0x807);
    }

    return rc == 0x000 || rc == 0x001 || rc == 0x002 || rc == 0x800 || rc == 0x801 ||
           rc == 0x802 || rc == 0x803 || rc == 0x804;
}

bool
Moira::isValidExtMMU(Instr I, Mode M, u16 op, u32 ext) const
{
    auto preg  = [ext]() { return ext >> 10 & 0b111;   };
    auto a     = [ext]() { return ext >>  8 & 0b1;     };
    auto mode  = [ext]() { return ext >> 10 & 0b111;   };
    auto mask  = [ext]() { return ext >>  5 & 0b1111;  }; // 68851 mask is 4 bit
    auto reg   = [ext]() { return ext >>  5 & 0b111;   };
    auto fc    = [ext]() { return ext       & 0b11111; };

    auto validFC = [&]() {
        return fc() <= 1 || (fc() >= 8); // Binutils checks M68851
    };

    switch (I) {

        case Instr::PFLUSHA:

            return (op & 0xFF) == 0 && mask() == 0 && fc() == 0;

        case Instr::PFLUSH:

            // Check mode
            if (mode() != 0b100 && mode() != 0b110) return false;

            // Check EA mode
            if (mode() == 0b110) {
                if (M != Mode::AI && M != Mode::DI && M != Mode::IX && M != Mode::AW && M != Mode::AL) {
                    return false;
                }
            }
            return validFC();

        case Instr::PLOAD:

            // Check EA mode
            if (M != Mode::AI && M != Mode::DI && M != Mode::IX && M != Mode::AW && M != Mode::AL) {
                return false;
            }

            return validFC();

        case Instr::PMOVE:

            if ((ext & 0x200)) {
                if (M == Mode::DIPC || M == Mode::IXPC || M == Mode::IM) return false;
            }
            if (M == Mode::IP) return false;

            switch (ext >> 13 & 0b111) {

                case 0b000:

                    // Check register field
                    if (preg() != 0b010 && preg() != 0b011) return false;

                    // If memory is written, flushing is mandatory
                    if ((ext & 0x300) == 0x300) return false;
                    return true;

                case 0b010:

                    // If memory is written, flushing is mandatory
                    if ((ext & 0x300) == 0x300) return false;

                    if ((ext & 0x300) == 0) {
                        if (preg() != 0) {
                            if (M == Mode::PI || M == Mode::PD || M == Mode::IM || M == Mode::IP) { return false;
                            }
                        }
                    }

                    // Check register field (binutils accepts all M68851 registers)
                    if ((ext & 0x100) == 0) {
                        if (preg() != 0) {
                            if (M == Mode::DN || M == Mode::AN) {
                                return false;
                            }
                        }
                    }
                    return true;

                case 0b011:

                    return true;

                default:
                    return false;
            }
            break;

        case Instr::PTEST:

            // When A is 0, reg must be 0
            if (a() == 0 && reg() != 0) return false;

            // Check FC
            if ((fc() & 0b11000) == 0 && (fc() & 0b110) != 0) return false;

            // Check EA mode
            if (M != Mode::AI && M != Mode::DI && M != Mode::IX && M != Mode::AW && M != Mode::AL) return false;

            return true;

        default:
            fatalError;
    }
}

#endif


//
// Address translation
//

template <Core C, bool write, Size S> u32
Moira::translate(u32 addr, u8 fc, bool *ci)
{
    if (ci) *ci = false;

    // Only proceed if a MMU capable core is present
    if constexpr (C == Core::C68000 || C == Core::C68010) {
        return addr;
    } else {

        // Only proceed if the MMU is present and enabled
        if (!mmu.isEnabled()) return addr;

        std::optional<u32> physical = mmu.translate(addr, fc, write, ci);
    
        if (!physical) {

            auto frame = makeFrame<write ? AE_WRITE : 0>(addr);

            // Special status word. On the 68030, a fault in program space belongs
            // to the instruction pipe, all other faults belong to a data cycle.
            if (cpuModel >= CPUModel::M68EC040) {

                // The 68040 reports the transfer type, the size and the cause
                u16 size = S == Byte ? SSW040::SIZ_B : S == Word ? SSW040::SIZ_W : SSW040::SIZ_L;

                frame.ssw = SSW040::ATC | size | (fc & 7);
                if (!write) frame.ssw |= SSW040::RW;

            } else {

                u16 size = S == Byte ? SSW030::SIZ_B : S == Word ? SSW030::SIZ_W : SSW030::SIZ_L;
                bool prog = (fc & 3) == 2;

                frame.ssw = (prog ? SSW030::FB | SSW030::RB : SSW030::DF) | size | (fc & 7);
                if (!write) frame.ssw |= SSW030::RW;
            }

            throw BusError(frame, false);
        }

        return physical.value();
    }
}

//
// Instruction handlers
//

template <Core C, Instr I, Mode M, Size S> void
Moira::execPGen(u16 opcode)
{
    auto ext = queue.irc;

    // PLOAD: 0010 00x0 000x xxxx
    if ((ext & 0xFDE0) == 0x2000) {

        execPLoad<C, Instr::PLOAD, M, S>(opcode);
        return;
    }

    // PFLUSHA: 0010 010x xxxx xxxx
    if ((ext & 0xFE00) == 0x2400) {

        execPFlusha<C, Instr::PFLUSHA, M, S>(opcode);
        return;
    }

    // PFLUSH: 001x xx0x xxxx xxxx
    if ((ext & 0xE200) == 0x2000) {

        execPFlush<C, Instr::PFLUSH, M, S>(opcode);
        return;
    }

    // PTEST: 100x xxxx xxxx xxxx
    if ((ext & 0xE000) == 0x8000) {

        execPTest<C, Instr::PTEST, M, S>(opcode);
        return;
    }

    // PMOVE: 010x xxxx 0000 0000 || 0110 00x0 0000 0000 || 000x xxxx 0000 0000
    if ((ext & 0xE0FF) == 0x4000 || (ext & 0xFDFF) == 0x6000 || (ext & 0xE0FF) == 0x0000) {

        execPMove<C, Instr::PMOVE, M, S>(opcode);
        return;
    }

    execIllegal<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlush(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    u16 ext   = queue.irc;
    auto reg  = _____________xxx (opcode);
    auto mode = ___xxx__________ (ext);
    auto mask = ________xxx_____ (ext);

    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext)) {

        execIllegal<C, Instr::ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();

    u8 fcBase = mmuFC(ext);

    if (mode == 0b110) {

        // Flush by function code and effective address
        u32 ea = computeEA<C, M, Long>(reg);
        mmu.mmu030.atcFlushPage(ea, fcBase, mask);

    } else {

        // Flush by function code
        mmu.mmu030.atcFlush(fcBase, mask);
    }

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlusha(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    u16 ext = queue.irc;

    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext)) {

        execIllegal<C, Instr::ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();

    mmu.mmu030.atcFlush();

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlush40(u16 opcode)
{
    AVAILABILITY(Core::C68020)
    SUPERVISOR_MODE_ONLY

    auto rg   = _____________xxx (opcode);
    auto mode = ___________xx___ (opcode);

    // Mode 0: PFLUSHN (An)   Flush a page, keep global entries
    // Mode 1: PFLUSH (An)    Flush a page
    // Mode 2: PFLUSHAN       Flush all entries, keep global entries
    // Mode 3: PFLUSHA        Flush all entries
    bool global = mode & 1;

    if (mode < 2) {

        // The DFC register selects user or supervisor space
        mmu.mmu040.atcFlushPage(readA(rg), reg.dfc & 4, global);

    } else {

        mmu.mmu040.atcFlush(global);
    }

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPLoad(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    u16 ext   = queue.irc;
    auto reg  = _____________xxx (opcode);
    auto rw   = ______x_________ (ext);

    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext)) {

        execIllegal<C, Instr::ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();

    u32 ea = computeEA<C, M, Long>(reg);
    mmu.mmu030.load(ea, mmuFC(ext), !rw);

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}

/* Checks if a PMOVE instruction can be executed (stricter than the disassembler check).
 *
 * The rules follow the instruction description in Motorola's M68000 Family
 * Programmer's Reference (PMOVE for the 68030):
 *
 *   Extension word, TC / SRP / CRP:  010 rrr w f 00000000    (rrr: TC = 000, SRP = 010, CRP = 011)
 *   Extension word, TT0 / TT1:       000 rrr w f 00000000    (rrr: TT0 = 010, TT1 = 011)
 *   Extension word, MMUSR:           011 000 w 000000000
 *
 * The effective address must be a memory-alterable or control mode, i.e., one of (An), (d16,An),
 * (d8,An,Xn) and its extended forms, (xxx).W, and (xxx).L. Dn, An, (An)+, -(An), the PC-relative
 * modes, and immediate data are not allowed.
 *
 * Note: The description doesn't say what happens if FD is set for a transfer from a register to
 * memory. Such an encoding is rejected, because the flush disable bit is meaningless in this case.
 */
template <Mode M> static bool
isExecutablePMove(u16 ext)
{
    auto fmt  = ext >> 13 & 0b111;      // Register group
    auto preg = ext >> 10 & 0b111;      // Register within the group
    bool rw   = ext & 0x200;            // 1: Register to memory
    bool fd   = ext & 0x100;            // 1: Don't flush the ATC

    // Check the effective address mode
    switch (M) {

        case Mode::AI: case Mode::DI: case Mode::IX: case Mode::AW: case Mode::AL: break;
        default: return false;
    }

    // The lower byte is reserved
    if (ext & 0xFF) return false;

    // FD is meaningless when a register is stored
    if (rw && fd) return false;

    // Check the register selection
    switch (fmt) {

        case 0b000: return preg == 0b010 || preg == 0b011;                  // TT0, TT1
        case 0b010: return preg == 0b000 || preg == 0b010 || preg == 0b011; // TC, SRP, CRP
        case 0b011: return preg == 0b000 && !fd;                            // MMUSR
        default:    return false;
    }
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPMove(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    u16 ext = queue.irc;
    auto fmt  = xxx_____________ (ext);
    auto preg = ___xxx__________ (ext);
    bool rw   = ______x_________ (ext);

    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext) || !isExecutablePMove<M>(ext)) {

        execIllegal<C, Instr::ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();

    switch (fmt) {

        case 0:

            switch (preg) {

                case 0b010: execPMove<C, M>(opcode, MmuReg::TT0, rw); break;
                case 0b011: execPMove<C, M>(opcode, MmuReg::TT1, rw); break;
            }
            break;

        case 2:

            switch (preg) {

                case 0b000: execPMove<C, M>(opcode, MmuReg::TC, rw); break;
                case 0b010: execPMove<C, M>(opcode, MmuReg::SRP, rw); break;
                case 0b011: execPMove<C, M>(opcode, MmuReg::CRP, rw); break;
            }
            break;

        case 3:

            switch (preg) {

                case 0b000: execPMove<C, M>(opcode, MmuReg::MMUSR, rw); break;
            }
            break;
    }

    // Writing a translation register flushes the ATC unless the FD bit is set
    if (!rw && fmt != 3 && !(ext & 0x100)) mmu.mmu030.atcFlush();

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Mode M> void
Moira::execPMove(u16 opcode, MmuReg mmuReg, bool rw)
{
    u32 ea;
    u32 data32;
    u64 data64;

    auto reg  = _____________xxx (opcode);

    if (rw) {

        switch (mmuReg) {

            case MmuReg::MMUSR:

                writeOp<C, M, Word>(reg, mmu.mmu030.mmusr);
                break;

            case MmuReg::TT0:

                writeOp<C, M, Long>(reg, mmu.mmu030.tt0);
                break;

            case MmuReg::TT1:

                writeOp<C, M, Long>(reg, mmu.mmu030.tt1);
                break;

            case MmuReg::TC:

                writeOp<C, M, Long>(reg, mmu.mmu030.tc);
                break;

            case MmuReg::CRP:

                writeOp64<C, M>(reg, mmu.mmu030.crp);
                break;

            case MmuReg::SRP:

                writeOp64<C, M>(reg, mmu.mmu030.srp);
                break;

            default:
                fatalError;
        }

    } else {

        switch (mmuReg) {

            case MmuReg::MMUSR:

                readOp<C, M, Word>(reg, &ea, &data32);
                mmu.mmu030.mmusr = u16(data32);
                break;

            case MmuReg::TT0:

                readOp<C, M, Long>(reg, &ea, &data32);
                mmu.mmu030.setTT0(data32);
                break;

            case MmuReg::TT1:

                readOp<C, M, Long>(reg, &ea, &data32);
                mmu.mmu030.setTT1(data32);
                break;

            case MmuReg::TC:

                readOp<C, M, Long>(reg, &ea, &data32);
                mmu.mmu030.setTC(data32);
                break;

            case MmuReg::CRP:

                readOp64<C, M>(reg, &ea, &data64);
                mmu.mmu030.crp = data64;
                break;

            case MmuReg::SRP:

                readOp64<C, M>(reg, &ea, &data64);
                mmu.mmu030.srp = data64;
                break;

            default:
                fatalError;
        }
    }
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPTest(u16 opcode)
{

    AVAILABILITY(Core::C68020)
    
    u16 ext   = queue.irc;
    auto rg   = _____________xxx (opcode);
    auto fc   = ___________xxxxx (ext);
    auto rw   = ______x_________ (ext);
    auto a    = _______x________ (ext);
    auto areg = ________xxx_____ (ext);
    auto lvl  = ___xxx__________ (ext);

    // Catch illegal extension words. Level 0 doesn't support the A bit and
    // function code field 11xxx is reserved.
    if (!isValidExtMMU(I, M, opcode, ext) || (lvl == 0 && a) || (fc & 0b11000) == 0b11000) {

        execIllegal<C, Instr::ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();

    // Compute effective address
    u32 ea = computeEA<C, M, Long>(rg);

    // u8 fcode = mmuFC(ext);

    u32 descrAddr;
    mmu.mmu030.mmusr = mmu.mmu030.test(ea, mmuFC(ext), !rw, lvl, descrAddr);
    if (a) writeA(areg, descrAddr);

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPTest40(u16 opcode)
{
    AVAILABILITY(Core::C68020)
    SUPERVISOR_MODE_ONLY

    auto rg  = _____________xxx (opcode);
    auto rw  = __________x_____ (opcode);

    // The DFC register selects the address space
    mmu.mmu040.mmusr = mmu.mmu040.test(readA(rg), reg.dfc, !rw);

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}
