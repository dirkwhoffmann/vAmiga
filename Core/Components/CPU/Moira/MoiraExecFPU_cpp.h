// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

// The following definitions are not templated. They are compiled into the
// main translation unit only (see MoiraCore_cpp.h).

#ifdef MOIRA_MAIN_TU

void
Moira::finishFpuOp(int dst, const MFloat80 &old)
{
    // An enabled SNAN, OPERR or DZ exception leaves the destination register unchanged
    if (fpu.suppressesResult()) fpu.fpr[dst] = old;

    // Enabled exceptions are taken when the next FPU instruction starts
    fpu.pendingTrap = fpu.trapVector();
}

bool
Moira::isValidExtFPU(Instr I, Mode M, u16 op, u32 ext, CPUModel model) const
{
    // Some FMOVE variants (FSMOVE, FDMOVE) exist on the 68040 FPU only
    auto has040Fpu = model == CPUModel::M68040;

    auto cod  = xxx_____________ (ext);
    auto mode = ___xx___________ (ext);
    auto fmt  = ___xxx__________ (ext);
    auto lst  = ___xxx__________ (ext);
    auto cmd  = _________xxxxxxx (ext);

    switch (I) {

        case Instr::FDBcc:
        case Instr::FScc:
        case Instr::FTRAPcc:

            return (ext & 0xFFE0) == 0;

        case Instr::FMOVECR:

            return (op & 0x3F) == 0;

        case Instr::FMOVE:

            switch (cod) {

                case 0b010:

                    if (M == Mode::IP) break;
                    if ((cmd == 0x40 || cmd == 0x44) && !has040Fpu) break;
                    return true;

                case 0b000:

                    if (cmd == 0 && cod == 0 && (op & 0x3F)) break;
                    if (cmd != 0 && !has040Fpu) break;
                    return true;

                case 0b011:

                    if (fmt != 0b011 && fmt != 0b111 && (ext & 0x7F)) break;

                    if (M == Mode::DN) {
                        if (fmt == 0b010 || fmt == 0b011 || fmt == 0b101 || fmt == 0b111) break;
                    }
                    if (M == Mode::AN) {
                        if (fmt == 0b011 || fmt == 0b111) break;
                    }
                    if (M == Mode::DIPC || M == Mode::IXPC || M == Mode::IM || M == Mode::IP) {
                        break;
                    } else {
                        if (fmt == 0b111 && (ext & 0xF)) break;
                    }

                    return true;
            }
            return false;

        case Instr::FMOVEM:

            switch (cod) {

                case 0b101:
                {

                    if (ext & 0x3FF) break;

                    if (M == Mode::DN || M == Mode::AN) {
                        if (lst != 0b000 && lst != 0b001 && lst != 0b010 && lst != 0b100) break;
                    }
                    if (M == Mode::DIPC || M == Mode::IXPC || M == Mode::IM || M == Mode::IP) {
                        break;
                    }
                    return true;
                }
                case 0b100:

                    if (ext & 0x3FF) break;
                    if (M == Mode::IP) break;
                    return true;

                case 0b110:
                case 0b111:

                    if (ext & 0x0700) break;
                    if (mode == 3 && (ext & 0x8F)) break;

                    if (M == Mode::DN || M == Mode::AN) {
                        break;
                    }
                    if (M == Mode::DIPC || M == Mode::IXPC || M == Mode::IM || M == Mode::IP) {
                        break;
                    }
                    if (M == Mode::AI) {
                        if (mode == 0 || mode == 1) break;
                    }
                    if (M == Mode::PI) {
                        if (mode == 0 || mode == 1 || cod == 0b111) break;
                    }
                    if (M == Mode::PD) {
                        if (cod == 0b110) break;
                        if (cod == 0b111 && (mode == 1) && (ext & 0x8F)) break;
                        if (cod == 0b111 && (mode == 2 || mode == 3)) break;
                    }
                    if (M == Mode::DI || M == Mode::IX || M == Mode::AW || M == Mode::AL) {
                        if (mode == 0 || mode == 1) break;
                    }
                    return true;
            }
            return false;

        default:
            fatalError;
    }
}

#endif


//
// Dataflow
//

template <Core C, Mode M> u32
Moira::computeFpuEA(int n, FpuFormat fmt)
{
    switch (fmt) {

        case FpuFormat::BYTE:       return computeEA<C, M, Byte>(n);
        case FpuFormat::WORD:       return computeEA<C, M, Word>(n);
        case FpuFormat::LONG:
        case FpuFormat::SINGLE:     return computeEA<C, M, Long>(n);
        case FpuFormat::DOUBLE:     return computeEA<C, M, Quad>(n);
        default:                    return computeEA<C, M, Extended>(n);
    }
}

template <Core C, Mode M> MFloat80
Moira::readFpuOp(int n, FpuFormat fmt)
{
    if constexpr (M == Mode::DN) return readFpuOpRg<C>(n, fmt);
    if constexpr (M == Mode::IM) return readFpuOpIm<C>(fmt);
    if constexpr (M == Mode::AN || M == Mode::IP) throw IllegalInstruction();

    if constexpr (M != Mode::AN && M != Mode::IP) {

        u32 ea = computeFpuEA<C, M>(n, fmt);
        return readFpuOpEa<C, M>(n, ea, fmt);
    }
}

template <Core C> MFloat80
Moira::readFpuOpRg(int n, FpuFormat fmt)
{
    switch (fmt) {

        case FpuFormat::BYTE:       return FpuFormats::fromInteger((i8)reg.d[n]);
        case FpuFormat::WORD:       return FpuFormats::fromInteger((i16)reg.d[n]);
        case FpuFormat::LONG:       return FpuFormats::fromInteger((i32)reg.d[n]);
        case FpuFormat::SINGLE:     return FpuFormats::fromSingle(fpu.env, reg.d[n]);

        default:                    throw IllegalInstruction();
    }
}

template <Core C> MFloat80
Moira::readFpuOpIm(FpuFormat fmt)
{
    u32 w[3];

    switch (fmt) {

        case FpuFormat::BYTE:       return FpuFormats::fromInteger((i8)readI<C, Word>());
        case FpuFormat::WORD:       return FpuFormats::fromInteger((i16)readI<C, Word>());
        case FpuFormat::LONG:       return FpuFormats::fromInteger((i32)readI<C, Long>());
        case FpuFormat::SINGLE:     return FpuFormats::fromSingle(fpu.env, readI<C, Long>());

        case FpuFormat::DOUBLE:
        {
            u64 hi = readI<C, Long>();
            u64 lo = readI<C, Long>();
            return FpuFormats::fromDouble(fpu.env, hi << 32 | lo);
        }
        case FpuFormat::EXTENDED:
        {
            w[0] = readI<C, Long>();
            w[1] = readI<C, Long>();
            w[2] = readI<C, Long>();
            return FpuFormats::fromExtended(w);
        }
        case FpuFormat::PACKED:
        case FpuFormat::PACKED_DYN:
        {
            w[0] = readI<C, Long>();
            w[1] = readI<C, Long>();
            w[2] = readI<C, Long>();
            return FpuFormats::fromPacked(fpu.env, w);
        }
    }
    throw IllegalInstruction();
}

template <Core C, Mode M> MFloat80
Moira::readFpuOpEa(int n, u32 ea, FpuFormat fmt)
{
    u32 w[3];

    switch (fmt) {

        case FpuFormat::BYTE:
        {
            auto data = readM<C, M, Byte>(ea);
            updateAn<M, Byte>(n);
            return FpuFormats::fromInteger((i8)data);
        }
        case FpuFormat::WORD:
        {
            auto data = readM<C, M, Word>(ea);
            updateAn<M, Word>(n);
            return FpuFormats::fromInteger((i16)data);
        }
        case FpuFormat::LONG:
        {
            auto data = readM<C, M, Long>(ea);
            updateAn<M, Long>(n);
            return FpuFormats::fromInteger((i32)data);
        }
        case FpuFormat::SINGLE:
        {
            auto data = readM<C, M, Long>(ea);
            updateAn<M, Long>(n);
            return FpuFormats::fromSingle(fpu.env, data);
        }
        case FpuFormat::DOUBLE:
        {
            u64 hi = readM<C, M, Long>(ea);
            u64 lo = readM<C, M, Long>(U32_ADD(ea, 4));
            updateAn<M, Quad>(n);
            return FpuFormats::fromDouble(fpu.env, hi << 32 | lo);
        }
        case FpuFormat::EXTENDED:
        {
            w[0] = readM<C, M, Word>(ea) << 16;
            w[1] = readM<C, M, Long>(U32_ADD(ea, 4));
            w[2] = readM<C, M, Long>(U32_ADD(ea, 8));
            updateAn<M, Extended>(n);
            return FpuFormats::fromExtended(w);
        }
        case FpuFormat::PACKED:
        case FpuFormat::PACKED_DYN:
        {
            w[0] = readM<C, M, Long>(ea);
            w[1] = readM<C, M, Long>(U32_ADD(ea, 4));
            w[2] = readM<C, M, Long>(U32_ADD(ea, 8));
            updateAn<M, Extended>(n);
            return FpuFormats::fromPacked(fpu.env, w);
        }
    }
    throw IllegalInstruction();
}

template <Core C, Mode M> void
Moira::writeFpuOp(int n, const MFloat80 &val, FpuFormat fmt, int k)
{
    if constexpr (M == Mode::DN) { writeFpuOpRg<C>(n, val, fmt); return; }
    if constexpr (M == Mode::AN || M == Mode::IM || M == Mode::IP) throw IllegalInstruction();

    if constexpr (M != Mode::AN && M != Mode::IM && M != Mode::IP) {

        u32 ea = computeFpuEA<C, M>(n, fmt);
        writeFpuOpEa<C, M>(n, ea, val, fmt, k);
    }
}

template <Core C> void
Moira::writeFpuOpRg(int n, const MFloat80 &val, FpuFormat fmt)
{
    switch (fmt) {

        case FpuFormat::BYTE:       writeD<Byte>(n, FpuFormats::toInteger(fpu.env, val, 8)); break;
        case FpuFormat::WORD:       writeD<Word>(n, FpuFormats::toInteger(fpu.env, val, 16)); break;
        case FpuFormat::LONG:       writeD<Long>(n, FpuFormats::toInteger(fpu.env, val, 32)); break;
        case FpuFormat::SINGLE:     writeD<Long>(n, FpuFormats::toSingle(fpu.env, val)); break;

        default:                    throw IllegalInstruction();
    }
}

template <Core C, Mode M> void
Moira::writeFpuOpEa(int n, u32 ea, const MFloat80 &val, FpuFormat fmt, int k)
{
    u32 w[3];

    switch (fmt) {

        case FpuFormat::BYTE:
        {
            writeM<C, M, Byte>(ea, FpuFormats::toInteger(fpu.env, val, 8));
            updateAn<M, Byte>(n);
            break;
        }
        case FpuFormat::WORD:
        {
            writeM<C, M, Word>(ea, FpuFormats::toInteger(fpu.env, val, 16));
            updateAn<M, Word>(n);
            break;
        }
        case FpuFormat::LONG:
        {
            writeM<C, M, Long>(ea, FpuFormats::toInteger(fpu.env, val, 32));
            updateAn<M, Long>(n);
            break;
        }
        case FpuFormat::SINGLE:
        {
            writeM<C, M, Long>(ea, FpuFormats::toSingle(fpu.env, val));
            updateAn<M, Long>(n);
            break;
        }
        case FpuFormat::DOUBLE:
        {
            auto data = FpuFormats::toDouble(fpu.env, val);
            writeM<C, M, Long>(ea, u32(data >> 32));
            writeM<C, M, Long>(U32_ADD(ea, 4), u32(data));
            updateAn<M, Quad>(n);
            break;
        }
        case FpuFormat::EXTENDED:
        {
            // The register contents are stored as they are (no rounding to the FPCR precision)
            FpuFormats::toExtended(val, w);
            writeM<C, M, Word>(ea, w[0] >> 16);
            writeM<C, M, Word>(U32_ADD(ea, 2), 0);
            writeM<C, M, Long>(U32_ADD(ea, 4), w[1]);
            writeM<C, M, Long>(U32_ADD(ea, 8), w[2]);
            updateAn<M, Extended>(n);
            break;
        }
        case FpuFormat::PACKED:
        case FpuFormat::PACKED_DYN:
        {
            FpuFormats::toPacked(fpu.env, val, k, w);
            writeM<C, M, Long>(ea, w[0]);
            writeM<C, M, Long>(U32_ADD(ea, 4), w[1]);
            writeM<C, M, Long>(U32_ADD(ea, 8), w[2]);
            updateAn<M, Extended>(n);
            break;
        }
    }
}


//
// Instruction handlers
//

template <Core C, Instr I, Mode M, Size S> void
Moira::execFGen(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    // First cycle of the coprocessor protocol
    if (hasExternalFPU()) notifyCPCycle(opcode, CIR::COMMAND);

    // A pending arithmetic exception is taken before the instruction starts
    if (takePendingFpuTrap<C>()) { FINALIZE return; }

    auto ext = queue.irc;
    auto cod = xxx_____________(ext);
    auto cmd = _________xxxxxxx(ext);

    if constexpr (M == Mode::AN) {
        if (ext & 0x4000) { execLineF<C, I, M, S>(opcode); return; }
    }
    if constexpr (M == Mode::IP) {
        if (cod == 0b010) { execLineF<C, I, M, S>(opcode); return; }
    }

    // Catch FMOVE instructions
    switch (cod) {

        case 0b010:

            if ((ext & 0xFC00) == 0x5C00) { execFMovecr<C, Instr::FMOVECR, M, S>(opcode); return; }
            [[fallthrough]];

        case 0b000:

            if (cmd != 0x00 && cmd != 0x40 && cmd != 0x44) break;
            [[fallthrough]];

        case 0b011:

            execFMove<C, Instr::FMOVE, M, S>(opcode);
            return;

        case 0b101:
        case 0b100:
        case 0b110:
        case 0b111:

            execFMovem<C, Instr::FMOVEM, M, S>(opcode);
            return;
    }

    // Catch all other instructions
    switch (cod) {

        case 0b010:
        case 0b000:

            switch (cmd) {

                case 0x01: execFGeneric<C, Instr::FINT, M, S>(opcode); return;
                case 0x02: execFGeneric<C, Instr::FSINH, M, S>(opcode); return;
                case 0x03: execFGeneric<C, Instr::FINTRZ, M, S>(opcode); return;
                case 0x04: execFGeneric<C, Instr::FSQRT, M, S>(opcode); return;
                case 0x06: execFGeneric<C, Instr::FLOGNP1, M, S>(opcode); return;
                case 0x08: execFGeneric<C, Instr::FETOXM1, M, S>(opcode); return;
                case 0x09: execFGeneric<C, Instr::FTANH, M, S>(opcode); return;
                case 0x0A: execFGeneric<C, Instr::FATAN, M, S>(opcode); return;
                case 0x0C: execFGeneric<C, Instr::FASIN, M, S>(opcode); return;
                case 0x0D: execFGeneric<C, Instr::FATANH, M, S>(opcode); return;
                case 0x0E: execFGeneric<C, Instr::FSIN, M, S>(opcode); return;
                case 0x0F: execFGeneric<C, Instr::FTAN, M, S>(opcode); return;
                case 0x10: execFGeneric<C, Instr::FETOX, M, S>(opcode); return;
                case 0x11: execFGeneric<C, Instr::FTWOTOX, M, S>(opcode); return;
                case 0x12: execFGeneric<C, Instr::FTENTOX, M, S>(opcode); return;
                case 0x14: execFGeneric<C, Instr::FLOGN, M, S>(opcode); return;
                case 0x15: execFGeneric<C, Instr::FLOG10, M, S>(opcode); return;
                case 0x16: execFGeneric<C, Instr::FLOG2, M, S>(opcode); return;
                case 0x18: execFGeneric<C, Instr::FABS, M, S>(opcode); return;
                case 0x19: execFGeneric<C, Instr::FCOSH, M, S>(opcode); return;
                case 0x1A: execFGeneric<C, Instr::FNEG, M, S>(opcode); return;
                case 0x1C: execFGeneric<C, Instr::FACOS, M, S>(opcode); return;
                case 0x1D: execFGeneric<C, Instr::FCOS, M, S>(opcode); return;
                case 0x1E: execFGeneric<C, Instr::FGETEXP, M, S>(opcode); return;
                case 0x1F: execFGeneric<C, Instr::FGETMAN, M, S>(opcode); return;
                case 0x20: execFGeneric<C, Instr::FDIV, M, S>(opcode); return;
                case 0x21: execFGeneric<C, Instr::FMOD, M, S>(opcode); return;
                case 0x22: execFGeneric<C, Instr::FADD, M, S>(opcode); return;
                case 0x23: execFGeneric<C, Instr::FMUL, M, S>(opcode); return;
                case 0x24: execFGeneric<C, Instr::FSGLDIV, M, S>(opcode); return;
                case 0x25: execFGeneric<C, Instr::FREM, M, S>(opcode); return;
                case 0x26: execFGeneric<C, Instr::FSCAL, M, S>(opcode); return;
                case 0x27: execFGeneric<C, Instr::FSGLMUL, M, S>(opcode); return;
                case 0x28: execFGeneric<C, Instr::FSUB, M, S>(opcode); return;
                case 0x30:
                case 0x31:
                case 0x32:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x36:
                case 0x37: execFGeneric<C, Instr::FSINCOS, M, S>(opcode); return;
                case 0x38: execFGeneric<C, Instr::FCMP, M, S>(opcode); return;
                case 0x3A: execFGeneric<C, Instr::FTST, M, S>(opcode); return;
                case 0x41: execFGeneric<C, Instr::FSSQRT, M, S>(opcode); return;
                case 0x45: execFGeneric<C, Instr::FDSQRT, M, S>(opcode); return;
                case 0x58: execFGeneric<C, Instr::FSABS, M, S>(opcode); return;
                case 0x5A: execFGeneric<C, Instr::FSNEG, M, S>(opcode); return;
                case 0x5C: execFGeneric<C, Instr::FDABS, M, S>(opcode); return;
                case 0x5E: execFGeneric<C, Instr::FDNEG, M, S>(opcode); return;
                case 0x60: execFGeneric<C, Instr::FSDIV, M, S>(opcode); return;
                case 0x62: execFGeneric<C, Instr::FSADD, M, S>(opcode); return;
                case 0x63: execFGeneric<C, Instr::FSMUL, M, S>(opcode); return;
                case 0x64: execFGeneric<C, Instr::FDDIV, M, S>(opcode); return;
                case 0x66: execFGeneric<C, Instr::FDADD, M, S>(opcode); return;
                case 0x67: execFGeneric<C, Instr::FDMUL, M, S>(opcode); return;
                case 0x68: execFGeneric<C, Instr::FSSUB, M, S>(opcode); return;
                case 0x6C: execFGeneric<C, Instr::FDSUB, M, S>(opcode); return;
            }
            break;
    }

    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFBcc(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    // First cycle of the coprocessor protocol
    if (hasExternalFPU()) notifyCPCycle(opcode, CIR::CONDITION);

    // A pending arithmetic exception is taken before the instruction starts
    if (takePendingFpuTrap<C>()) { FINALIZE return; }

    u32 oldpc = reg.pc;

    // Conditional instructions don't leave the reset state of the 68040
    if (hasExternalFPU()) fpu.resetState = false;

    bool taken = fpu.test(opcode & 0x1F);

    // An enabled BSUN exception is taken before the branch
    if (fpu.bsunTrap(opcode & 0x1F)) { execFpuTrap<C>(48); FINALIZE return; }

    if (taken) {

        u32 disp = queue.irc;

        if constexpr (S == Long) {

            readExt<C>();
            disp = disp << 16 | queue.irc;
        }

        u32 newpc = U32_ADD(oldpc, SEXT<S>(disp));

        // Check for address error
        if (misaligned<C>(newpc)) {
            throw AddressError(makeFrame(newpc));
        }

        // Take branch
        reg.pc = newpc;
        fullPrefetch<C, POLL>();
        changeOfFlow<C>();

    } else {

        // Fall through to next instruction
        readExt<C>();
        if constexpr (S == Long) readExt<C>();
        prefetch<C, POLL>();
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFDbcc(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    // First cycle of the coprocessor protocol
    if (hasExternalFPU()) notifyCPCycle(opcode, CIR::CONDITION);

    // A pending arithmetic exception is taken before the instruction starts
    if (takePendingFpuTrap<C>()) { FINALIZE return; }

    auto ext = queue.irc;
    auto dn  = _____________xxx (opcode);

    // Catch illegal extension words
    if (!isValidExtFPU(I, M, opcode, ext, cpuModel)) { execLineF<C, I, M, S>(opcode); return; }

    if (hasExternalFPU()) fpu.resetState = false;
    readI<C, Word>();

    bool cond = fpu.test(ext & 0x3F);

    // An enabled BSUN exception is taken before the counter is decremented
    if (fpu.bsunTrap(ext & 0x3F)) { execFpuTrap<C>(48); FINALIZE return; }

    if (!cond) {

        u32 newpc = U32_ADD(reg.pc, (i16)queue.irc);
        bool takeBranch = readD<Word>(dn) != 0;

        // Check for address error
        if (takeBranch && misaligned<C>(newpc)) {
            throw AddressError(makeFrame(newpc));
        }

        // Decrement loop counter
        writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));

        if (takeBranch) {

            reg.pc = newpc;
            fullPrefetch<C, POLL>();
            changeOfFlow<C>();
            FINALIZE
            return;
        }
    }

    // Fall through to next instruction
    reg.pc += 2;
    fullPrefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFNop(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    // First cycle of the coprocessor protocol
    if (hasExternalFPU()) notifyCPCycle(opcode, CIR::CONDITION);

    // FNOP synchronizes the FPU. A pending arithmetic exception is taken here.
    if (takePendingFpuTrap<C>()) { FINALIZE return; }

    // FNOP is encoded as FBcc.W with condition F and displacement 0
    readExt<C>();
    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFRestore(u16 opcode)
{
    AVAILABILITY(Core::C68000)
    SUPERVISOR_MODE_ONLY

    // First cycle of the coprocessor protocol
    if (hasExternalFPU()) notifyCPCycle(opcode, CIR::RESTORE);

    auto n = _____________xxx (opcode);

    // Locate the state frame
    u32 ea;
    if constexpr (M == Mode::PI) ea = readA(n); else ea = computeEA<C, M, Long>(n);

    u32 fmt = readM<C, M, Long>(ea);
    u32 version = fmt >> 24;

    if (version == 0) {

        // A NULL frame resets the FPU
        fpu.reset();
        if constexpr (M == Mode::PI) writeA(n, U32_ADD(ea, 4));

    } else {

        // The size of the frame is stored in the format word. Idle frames carry no exception.
        u32 size = 4 + ((fmt >> 16) & 0xFF);
        fpu.resetState = false;
        fpu.pendingTrap = 0;
        if constexpr (M == Mode::PI) writeA(n, U32_ADD(ea, size));
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFSave(u16 opcode)
{
    AVAILABILITY(Core::C68000)
    SUPERVISOR_MODE_ONLY

    // First cycle of the coprocessor protocol
    if (hasExternalFPU()) notifyCPCycle(opcode, CIR::SAVE);

    auto n = _____________xxx (opcode);

    // The frame carries no exception state. A pending arithmetic exception is discarded (as in UAE).
    fpu.pendingTrap = 0;

    // Frame to be written: A NULL frame if the FPU is in reset state, an IDLE frame otherwise
    bool null = fpu.inResetState();
    u32 fmt   = null ? 0 : fpu.computeFormatWord(FpuFrameType::IDLE);
    u32 size  = null ? 4 : 4 + fpu.stateFrameSize(FpuFrameType::IDLE);

    u32 ea;
    if constexpr (M == Mode::PD) ea = U32_SUB(readA(n), size); else ea = computeEA<C, M, Long>(n);

    writeM<C, M, Long>(ea, fmt);
    for (u32 i = 4; i < size; i += 4) writeM<C, M, Long>(U32_ADD(ea, i), 0);

    if constexpr (M == Mode::PD) writeA(n, ea);

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFScc(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    // First cycle of the coprocessor protocol
    if (hasExternalFPU()) notifyCPCycle(opcode, CIR::CONDITION);

    // A pending arithmetic exception is taken before the instruction starts
    if (takePendingFpuTrap<C>()) { FINALIZE return; }

    auto ext = queue.irc;
    auto dst = _____________xxx (opcode);

    // Catch illegal extension words
    if (!isValidExtFPU(I, M, opcode, ext, cpuModel)) { execLineF<C, I, M, S>(opcode); return; }

    if (hasExternalFPU()) fpu.resetState = false;
    readI<C, Word>();

    u32 data = fpu.test(ext & 0x3F) ? 0xFF : 0;

    // An enabled BSUN exception is taken before the destination is written
    if (fpu.bsunTrap(ext & 0x3F)) { execFpuTrap<C>(48); FINALIZE return; }

    if constexpr (M == Mode::DN) {

        writeD<Byte>(dst, data);

    } else {

        u32 ea = computeEA<C, M, Byte>(dst);
        updateAn<M, Byte>(dst);
        writeM<C, M, Byte>(ea, data);
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFTrapcc(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    // First cycle of the coprocessor protocol
    if (hasExternalFPU()) notifyCPCycle(opcode, CIR::CONDITION);

    // A pending arithmetic exception is taken before the instruction starts
    if (takePendingFpuTrap<C>()) { FINALIZE return; }

    auto ext = queue.irc;

    // Catch illegal extension words
    if (!isValidExtFPU(I, M, opcode, ext, cpuModel)) { execLineF<C, I, M, S>(opcode); return; }

    if (hasExternalFPU()) fpu.resetState = false;
    readI<C, Word>();

    // Skip the optional operand
    switch (opcode & 0b111) {

        case 0b010: (void)readI<C, Word>(); break;
        case 0b011: (void)readI<C, Long>(); break;
    }

    bool cond = fpu.test(ext & 0x3F);

    // An enabled BSUN exception is taken before the trap
    if (fpu.bsunTrap(ext & 0x3F)) { execFpuTrap<C>(48); FINALIZE return; }

    if (cond) {

        execException<C>(M68kException::TRAPV);
        FINALIZE
        return;
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMove(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    auto ext = queue.irc;
    auto reg = _____________xxx (opcode);
    auto cod = xxx_____________ (ext);
    auto fmt = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    auto cmd = _________xxxxxxx (ext);

    // Catch illegal extension words and operand formats
    bool illegal = !isValidExtFPU(I, M, opcode, ext, cpuModel);

    // The disassembler accepts the following combinations, because binutils does
    if (cod == 0b010) {
        if (fmt == 0b111) illegal = true;
        if (M == Mode::DN && (fmt == 0b010 || fmt == 0b011 || fmt == 0b101)) illegal = true;
    }

    if (illegal) { execLineF<C, I, M, S>(opcode); return; }

    fpu.resetState = false;
    readI<C, Word>();
    fpu.clearExcStatusByte();

    // The 68040 FPU updates FPIAR on every data operation, older FPUs only if an
    // exception other than BSUN is enabled (only conditional instructions raise BSUN)
    if (cpuModel >= CPUModel::M68EC040 || (fpu.getFPCR() & 0x7F00)) fpu.setFPIAR(getPC0());

    auto old = fpu.fpr[dst];
    u32 ea = 0;

    {
        // FSMOVE and FDMOVE round to single or double precision
        FPU::Scope scope(fpu);
        if (cmd == 0x40) fpu.env.setPrecision(Precision::SINGLE);
        if (cmd == 0x44) fpu.env.setPrecision(Precision::DOUBLE);

        switch (cod) {

            case 0b000:                                     // FMOVE FPm,FPn
            {
                auto src = fmt;
                fpu.fpr[dst] = fpu.round(fpu.fpr[src]);
                fpu.setConditionCodes(fpu.fpr[dst]);
                break;
            }
            case 0b010:                                     // FMOVE <ea>,FPn
            {
                auto format = FpuFormat(fmt);
                MFloat80 value;

                if (format == FpuFormat::PACKED) {

                    // Packed decimal numbers are converted exactly, the inexact flag has its own bit
                    FpuEnv::Scope exact(fpu.env);
                    fpu.env.setPrecision(Precision::EXTENDED);
                    fpu.env.clearFlags();
                    value = readFpuOp<C, M>(reg, format);
                    if (fpu.env.flags() & FloatFlags::INEXACT) fpu.setExcStatusBit(FPEXP::INEX1);
                    fpu.env.clearFlags(FloatFlags::INEXACT);

                } else {

                    value = readFpuOp<C, M>(reg, format);
                }

                if (auto nan = fpu.resolveNan(value)) value = *nan;

                fpu.fpr[dst] = fpu.round(value);
                fpu.setConditionCodes(fpu.fpr[dst]);
                break;
            }
            case 0b011:                                     // FMOVE FPn,<ea>
            {
                auto format = FpuFormat(fmt);
                int k = 0;

                if (format == FpuFormat::PACKED || format == FpuFormat::PACKED_DYN) {

                    // The k-factor is a sign-extended 7-bit value (bit 7 of a register is ignored)
                    k = (format == FpuFormat::PACKED) ? cmd : readD(cmd >> 4 & 7);
                    k = i8(k << 1) >> 1;
                }

                auto value = fpu.fpr[dst];

                // Signaling NaNs and denormal numbers are reported and converted
                // (FMOVEM, in contrast, transfers the register contents unchanged)
                if (format == FpuFormat::EXTENDED) {
                    value = fpu.moveOutExtended(value);
                }

                if constexpr (isMemMode(M)) {

                    ea = computeFpuEA<C, M>(reg, format);
                    writeFpuOpEa<C, M>(reg, ea, value, format, k);

                } else {

                    writeFpuOp<C, M>(reg, value, format, k);
                }
                break;
            }
        }
    }

    if (cod == 0b011) {

        // The 68040 takes an enabled exception right after a move to memory or to
        // a data register (post-instruction exception). The 68881 and 68882 report
        // it when the next FPU instruction starts, like all other exceptions.
        if (auto vector = fpu.trapVector(); vector && hasInternalFPU()) {

            execFpuTrap<C>(vector, true, ea);
            FINALIZE
            return;
        }
        fpu.pendingTrap = fpu.trapVector();

    } else {

        finishFpuOp(dst, old);
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMovecr(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    auto ext = queue.irc;
    auto dst = ______xxx_______ (ext);
    auto ofs = _________xxxxxxx (ext);

    // Catch illegal extension words
    if (!isValidExtFPU(I, M, opcode, ext, cpuModel)) { execLineF<C, I, M, S>(opcode); return; }

    // The 68040 leaves FMOVECR to the FPSP
    if (hasInternalFPU()) {

        execFUnimplemented<C, Mode::DN>(0, FpuFormat::EXTENDED, false);
        FINALIZE
        return;
    }

    fpu.resetState = false;
    readI<C, Word>();
    fpu.clearExcStatusByte();

    // The 68040 FPU updates FPIAR on every data operation, older FPUs only if an
    // exception other than BSUN is enabled (only conditional instructions raise BSUN)
    if (cpuModel >= CPUModel::M68EC040 || (fpu.getFPCR() & 0x7F00)) fpu.setFPIAR(getPC0());

    {
        FPU::Scope scope(fpu);

        fpu.fpr[dst] = fpu.round(fpu.readCR(ofs));
        fpu.setConditionCodes(fpu.fpr[dst]);
    }

    // The constant is stored in any case. An enabled exception is taken later.
    fpu.pendingTrap = fpu.trapVector();

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMovem(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    auto ext = queue.irc;
    auto reg = _____________xxx (opcode);
    auto cod = xxx_____________ (ext);
    auto mod = ___xx___________ (ext);
    auto lll = ___xxx__________ (ext);
    auto rrr = _________xxx____ (ext);

    // Catch illegal extension words
    if (!isValidExtFPU(I, M, opcode, ext, cpuModel)) { execLineF<C, I, M, S>(opcode); return; }

    fpu.resetState = false;
    readI<C, Word>();

    switch (cod) {

        case 0b100:             // FMOVEM <ea>,CR
        case 0b101:             // FMOVEM CR,<ea>
        {
            bool load = (cod == 0b100);

            // Data and address registers hold a single control register
            if constexpr (M == Mode::DN || M == Mode::AN) {

                if (load) {

                    u32 data = (M == Mode::DN) ? readD(reg) : readA(reg);
                    if (lll & 4) fpu.setFPCR(data);
                    if (lll & 2) fpu.setFPSR(data);
                    if (lll & 1) fpu.setFPIAR(data);

                } else {

                    u32 data = 0;
                    if (lll & 4) data = fpu.getFPCR();
                    if (lll & 2) data = fpu.getFPSR();
                    if (lll & 1) data = fpu.getFPIAR();

                    if constexpr (M == Mode::DN) writeD(reg, data); else writeA(reg, data);
                }
                break;
            }

            if constexpr (M == Mode::IM) {

                // Immediate data is read from the instruction stream
                if (lll & 4) fpu.setFPCR(readI<C, Long>());
                if (lll & 2) fpu.setFPSR(readI<C, Long>());
                if (lll & 1) fpu.setFPIAR(readI<C, Long>());
                break;
            }

            if constexpr (M != Mode::DN && M != Mode::AN && M != Mode::IM && M != Mode::IP) {

                // Compute the address of the first transferred register
                int count = (lll & 1) + (lll >> 1 & 1) + (lll >> 2 & 1);
                u32 ea = computeEA<C, M, Long>(reg);
                if constexpr (M == Mode::PD) { U32_DEC(ea, 4 * (count - 1)); }

                auto transfer = [&](bool selected, auto get, auto set) {

                    if (!selected) return;

                    if (load) {
                        set(readM<C, M, Long>(ea));
                    } else {
                        writeM<C, M, Long>(ea, get());
                    }
                    U32_INC(ea, 4);
                    if constexpr (M == Mode::PI) updateAnPI<M, Long>(reg);
                    if constexpr (M == Mode::PD) updateAnPD<M, Long>(reg);
                };

                // The registers are always located in memory in the order FPCR, FPSR, FPIAR
                transfer(lll & 4, [&] { return fpu.getFPCR(); },  [&](u32 v) { fpu.setFPCR(v); });
                transfer(lll & 2, [&] { return fpu.getFPSR(); },  [&](u32 v) { fpu.setFPSR(v); });
                transfer(lll & 1, [&] { return fpu.getFPIAR(); }, [&](u32 v) { fpu.setFPIAR(v); });
            }
            break;
        }
        case 0b110:             // FMOVEM <ea>,FPn-list
        case 0b111:             // FMOVEM FPn-list,<ea>
        {
            bool load = (cod == 0b110);
            bool dynamic = mod & 1;
            bool predec = (mod & 2) == 0;

            u32 list = dynamic ? (readD(rrr) & 0xFF) : (ext & 0xFF);

            // Count the registers to be transferred
            int count = 0;
            for (int i = 0; i < 8; i++) if (list & (1 << i)) count++;

            if constexpr (M != Mode::DN && M != Mode::AN && M != Mode::IM && M != Mode::IP) {

                u32 ea = computeEA<C, M, Extended>(reg);

                if (predec) {

                    // The mask is reversed for predecrement mode (bit 0 = FP0). Registers are stored from FP7 downwards.
                    for (int i = 7; i >= 0; i--) {

                        if (!(list & (1 << i))) continue;

                        if (load) {
                            fpu.fpr[i] = readFpuOpEa<C, M>(reg, ea, FpuFormat::EXTENDED);
                        } else {
                            writeFpuOpEa<C, M>(reg, ea, fpu.fpr[i], FpuFormat::EXTENDED, 0);
                        }
                        U32_DEC(ea, 12);
                    }

                } else {

                    // The mask is not reversed in all other modes (bit 7 = FP0)
                    for (int i = 0; i < 8; i++) {

                        if (!(list & (0x80 >> i))) continue;

                        if (load) {
                            fpu.fpr[i] = readFpuOpEa<C, M>(reg, ea, FpuFormat::EXTENDED);
                        } else {
                            writeFpuOpEa<C, M>(reg, ea, fpu.fpr[i], FpuFormat::EXTENDED, 0);
                        }
                        U32_INC(ea, 12);
                    }
                }
            }
            break;
        }
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Mode M> void
Moira::execFUnimplemented(int n, FpuFormat fmt, bool rm)
{
    /* The 68040 raises an unimplemented floating-point instruction exception
     * (MC68040UM, 9.6.1). It reads the extension word and the source operand,
     * sets FPIAR, and writes a frame of format 2 with the address of the next
     * instruction and the effective address (0 for register operands, as in UAE).
     */
    u32 ea = 0;

    readI<C, Word>();
    fpu.setFPIAR(getPC0());

    if (rm) {

        if constexpr (isMemMode(M)) {

            ea = computeFpuEA<C, M>(n, fmt);
            (void)readFpuOpEa<C, M>(n, ea, fmt);

        } else {

            (void)readFpuOp<C, M>(n, fmt);
        }
    }

    execException<C>(M68kException::FPU_UNIMPLEMENTED, int(ea));
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFGeneric(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    auto ext = queue.irc;
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    auto reg = _____________xxx (opcode);

    auto precision = fpu.getPrecision(I);
    auto supported = fpu.isSupported(I);
    auto illegal = !supported && !hasInternalFPU();

    // Bit 14 of the extension byte indicated whether source is an <ea>
    auto rm  = (ext >> 14) & 1;

    if (rm) {

        // Source format 111 is reserved. Only 32-bit formats can be read from a data register.
        if (src == 0b111) illegal = true;
        if (M == Mode::DN && (src == 0b010 || src == 0b011 || src == 0b101)) illegal = true;
    }

    if (illegal) {
        
        execLineF<C, I, M, S>(opcode);
        return;
    }

    if (!supported) {

        // The 68040 reads the operand and leaves the instruction to the FPSP
        execFUnimplemented<C, M>(reg, rm ? FpuFormat(src) : FpuFormat::EXTENDED, rm);
        FINALIZE
        return;
    }

    fpu.resetState = false;
    readI<C, Word>();
    fpu.clearExcStatusByte();

    // The 68040 FPU updates FPIAR on every data operation, older FPUs only if an
    // exception other than BSUN is enabled (only conditional instructions raise BSUN)
    if (cpuModel >= CPUModel::M68EC040 || (fpu.getFPCR() & 0x7F00)) fpu.setFPIAR(getPC0());

    auto old = fpu.fpr[dst];

    {
        FPU::Scope scope(fpu, precision);

        // Read the source operand
        MFloat80 val;

        if (rm) {

            if (FpuFormat(src) == FpuFormat::PACKED) {

                FpuEnv::Scope exact(fpu.env);
                fpu.env.setPrecision(Precision::EXTENDED);
                fpu.env.clearFlags();
                val = readFpuOp<C, M>(reg, FpuFormat(src));
                if (fpu.env.flags() & FloatFlags::INEXACT) fpu.setExcStatusBit(FPEXP::INEX1);
                fpu.env.clearFlags(FloatFlags::INEXACT);

            } else {

                val = readFpuOp<C, M>(reg, FpuFormat(src));
            }

        } else {

            val = fpu.fpr[src];
        }

        auto &d = fpu.fpr[dst];
        MFloat80 result;
        bool roundResult = true;
        bool setCodes = true;

        switch (I) {

            case Instr::FTST:       fpu.ftst(val); result = d; roundResult = setCodes = false; break;
            case Instr::FCMP:       fpu.fcmp(d, val); result = d; roundResult = setCodes = false; break;
            case Instr::FABS:
            case Instr::FSABS:
            case Instr::FDABS:      result = fpu.fabs(val); break;
            case Instr::FNEG:
            case Instr::FSNEG:
            case Instr::FDNEG:      result = fpu.fneg(val); break;
            case Instr::FSQRT:
            case Instr::FSSQRT:
            case Instr::FDSQRT:     result = fpu.fsqrt(val); break;
            case Instr::FINT:       result = fpu.fint(val); roundResult = false; break;
            case Instr::FINTRZ:     result = fpu.fintrz(val); roundResult = false; break;
            case Instr::FGETEXP:    result = fpu.fgetexp(val); roundResult = false; break;
            case Instr::FGETMAN:    result = fpu.fgetman(val); roundResult = false; break;
            case Instr::FSIN:       result = fpu.fsin(val); break;
            case Instr::FCOS:       result = fpu.fcos(val); break;
            case Instr::FTAN:       result = fpu.ftan(val); break;
            case Instr::FASIN:      result = fpu.fasin(val); break;
            case Instr::FACOS:      result = fpu.facos(val); break;
            case Instr::FATAN:      result = fpu.fatan(val); break;
            case Instr::FSINH:      result = fpu.fsinh(val); break;
            case Instr::FCOSH:      result = fpu.fcosh(val); break;
            case Instr::FTANH:      result = fpu.ftanh(val); break;
            case Instr::FATANH:     result = fpu.fatanh(val); break;
            case Instr::FETOX:      result = fpu.fetox(val); break;
            case Instr::FETOXM1:    result = fpu.fetoxm1(val); break;
            case Instr::FTWOTOX:    result = fpu.ftwotox(val); break;
            case Instr::FTENTOX:    result = fpu.ftentox(val); break;
            case Instr::FLOGN:      result = fpu.flogn(val); break;
            case Instr::FLOGNP1:    result = fpu.flognp1(val); break;
            case Instr::FLOG2:      result = fpu.flog2(val); break;
            case Instr::FLOG10:     result = fpu.flog10(val); break;
            case Instr::FADD:
            case Instr::FSADD:
            case Instr::FDADD:      result = fpu.fadd(d, val); break;
            case Instr::FSUB:
            case Instr::FSSUB:
            case Instr::FDSUB:      result = fpu.fsub(d, val); break;
            case Instr::FMUL:
            case Instr::FSMUL:
            case Instr::FDMUL:      result = fpu.fmul(d, val); break;
            case Instr::FDIV:
            case Instr::FSDIV:
            case Instr::FDDIV:      result = fpu.fdiv(d, val); break;
            case Instr::FSGLMUL:    result = fpu.fsglmul(d, val); roundResult = false; break;
            case Instr::FSGLDIV:    result = fpu.fsgldiv(d, val); roundResult = false; break;
            case Instr::FMOD:       result = fpu.fmod(d, val); break;
            case Instr::FREM:       result = fpu.frem(d, val); roundResult = false; break;
            case Instr::FSCAL:      result = fpu.fscal(d, val); break;
                
            case Instr::FSINCOS: {
                
                MFloat80 sine, cosine;
                fpu.fsincos(val, sine, cosine);
                auto &c = fpu.fpr[ext & 7];
                c = fpu.round(cosine);
                result = sine;

                // The cosine is stored in any case. If an exception suppresses the sine
                // and both go to the same register, the register keeps the cosine.
                if ((ext & 7) == dst) old = c;
                break;
            }

            default:
                fatalError;
        }

        d = roundResult ? fpu.round(result) : result;
        if (setCodes) fpu.setConditionCodes(d);
    }

    finishFpuOp(dst, old);

    prefetch<C, POLL>();

    FINALIZE
}
