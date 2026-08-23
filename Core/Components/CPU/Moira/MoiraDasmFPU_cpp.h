// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

void
Moira::dasmFBcc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, S);
    auto cnd = ___________xxxxx (op);

    // Check for special FNOP opcode
    if ((op & 0x7F) == 0 && ext == 0) {

        dasmFNop(str, addr, op, Instr::FNOP, M, S);
        return;
    }

    auto dst = old + 2;
    U32_INC(dst, SEXT(ext, S));

    if (S == Long) {
        str << Ins{I} << Fcc{cnd} << Sz{S} << str.tab << UInt(dst);
    } else {
        str << Ins{I} << Fcc{cnd} << str.tab << UInt(dst);
    }
}

void
Moira::dasmFDbcc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr);
    auto src = _____________xxx (op);
    auto cnd = ___________xxxxx (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    auto dst = addr + 2;
    U32_INC(dst, SEXT(dasmIncRead(addr, S), S));

    str << Ins{I} << Fcc{cnd} << str.tab << Dn{src} << Sep{} << UInt(dst);
}

void
Moira::dasmFGen(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext  = dasmIncRead(addr, Word);
    auto cod  = xxx_____________(ext);
    auto cmd  = _________xxxxxxx(ext);
    addr -= 2;

    if (M == Mode::AN) {
        if (ext & 0x4000) { dasmLineF(str, addr, op, I, M, S); return; }
    }
    if (M == Mode::IP) {
        if (cod == 0b010) { dasmLineF(str, addr, op, I, M, S); return; }
    }

    // Catch FMOVE instructions
    switch (cod) {

        case 0b010:

            if ((ext & 0xFC00) == 0x5C00) { dasmFMovecr(str, addr, op, Instr::FMOVECR, M, S); return; }
            [[fallthrough]];

        case 0b000:

            if (cmd != 0x00 && cmd != 0x40 && cmd != 0x44) break;
            [[fallthrough]];

        case 0b011:

            dasmFMove(str, addr, op, Instr::FMOVE, M, S);
            return;

        case 0b101:
        case 0b100:
        case 0b110:
        case 0b111:

            dasmFMovem(str, addr, op, Instr::FMOVEM, M, S);
            return;
    }

    // Catch all other instructions
    switch (cod) {

        case 0b010:
        case 0b000:

            switch (cmd) {

                case 0x01: dasmFGeneric(str, addr, op, Instr::FINT, M, S); return;
                case 0x02: dasmFGeneric(str, addr, op, Instr::FSINH, M, S); return;
                case 0x03: dasmFGeneric(str, addr, op, Instr::FINTRZ, M, S); return;
                case 0x04: dasmFGeneric(str, addr, op, Instr::FSQRT, M, S); return;
                case 0x06: dasmFGeneric(str, addr, op, Instr::FLOGNP1, M, S); return;
                case 0x08: dasmFGeneric(str, addr, op, Instr::FETOXM1, M, S); return;
                case 0x09: dasmFGeneric(str, addr, op, Instr::FTANH, M, S); return;
                case 0x0A: dasmFGeneric(str, addr, op, Instr::FATAN, M, S); return;
                case 0x0C: dasmFGeneric(str, addr, op, Instr::FASIN, M, S); return;
                case 0x0D: dasmFGeneric(str, addr, op, Instr::FATANH, M, S); return;
                case 0x0E: dasmFGeneric(str, addr, op, Instr::FSIN, M, S); return;
                case 0x0F: dasmFGeneric(str, addr, op, Instr::FTAN, M, S); return;
                case 0x10: dasmFGeneric(str, addr, op, Instr::FETOX, M, S); return;
                case 0x11: dasmFGeneric(str, addr, op, Instr::FTWOTOX, M, S); return;
                case 0x12: dasmFGeneric(str, addr, op, Instr::FTENTOX, M, S); return;
                case 0x14: dasmFGeneric(str, addr, op, Instr::FLOGN, M, S); return;
                case 0x15: dasmFGeneric(str, addr, op, Instr::FLOG10, M, S); return;
                case 0x16: dasmFGeneric(str, addr, op, Instr::FLOG2, M, S); return;
                case 0x18: dasmFGeneric(str, addr, op, Instr::FABS, M, S); return;
                case 0x19: dasmFGeneric(str, addr, op, Instr::FCOSH, M, S); return;
                case 0x1A: dasmFGeneric(str, addr, op, Instr::FNEG, M, S); return;
                case 0x1C: dasmFGeneric(str, addr, op, Instr::FACOS, M, S); return;
                case 0x1D: dasmFGeneric(str, addr, op, Instr::FCOS, M, S); return;
                case 0x1E: dasmFGeneric(str, addr, op, Instr::FGETEXP, M, S); return;
                case 0x1F: dasmFGeneric(str, addr, op, Instr::FGETMAN, M, S); return;
                case 0x20: dasmFGeneric(str, addr, op, Instr::FDIV, M, S); return;
                case 0x21: dasmFGeneric(str, addr, op, Instr::FMOD, M, S); return;
                case 0x22: dasmFGeneric(str, addr, op, Instr::FADD, M, S); return;
                case 0x23: dasmFGeneric(str, addr, op, Instr::FMUL, M, S); return;
                case 0x24: dasmFGeneric(str, addr, op, Instr::FSGLDIV, M, S); return;
                case 0x25: dasmFGeneric(str, addr, op, Instr::FREM, M, S); return;
                case 0x26: dasmFGeneric(str, addr, op, Instr::FSCAL, M, S); return;
                case 0x27: dasmFGeneric(str, addr, op, Instr::FSGLMUL, M, S); return;
                case 0x28: dasmFGeneric(str, addr, op, Instr::FSUB, M, S); return;
                case 0x30: dasmFGeneric2(str, addr, op, Instr::FSINCOS, M, S); return;
                case 0x31: dasmFGeneric2(str, addr, op, Instr::FSINCOS, M, S); return;
                case 0x32: dasmFGeneric2(str, addr, op, Instr::FSINCOS, M, S); return;
                case 0x33: dasmFGeneric2(str, addr, op, Instr::FSINCOS, M, S); return;
                case 0x34: dasmFGeneric2(str, addr, op, Instr::FSINCOS, M, S); return;
                case 0x35: dasmFGeneric2(str, addr, op, Instr::FSINCOS, M, S); return;
                case 0x36: dasmFGeneric2(str, addr, op, Instr::FSINCOS, M, S); return;
                case 0x37: dasmFGeneric2(str, addr, op, Instr::FSINCOS, M, S); return;
                case 0x38: dasmFGeneric(str, addr, op, Instr::FCMP, M, S); return;
                case 0x3A: dasmFGeneric3(str, addr, op, Instr::FTST, M, S); return;
            }

            // 68040 only instructions
            if (hasFPU()) {

                switch (cmd) {

                    case 0x41: dasmFGeneric(str, addr, op, Instr::FSSQRT, M, S); return;
                    case 0x45: dasmFGeneric(str, addr, op, Instr::FDSQRT, M, S); return;
                    case 0x58: dasmFGeneric(str, addr, op, Instr::FSABS, M, S); return;
                    case 0x5A: dasmFGeneric(str, addr, op, Instr::FSNEG, M, S); return;
                    case 0x5C: dasmFGeneric(str, addr, op, Instr::FDABS, M, S); return;
                    case 0x5E: dasmFGeneric(str, addr, op, Instr::FDNEG, M, S); return;
                    case 0x60: dasmFGeneric(str, addr, op, Instr::FSDIV, M, S); return;
                    case 0x62: dasmFGeneric(str, addr, op, Instr::FSADD, M, S); return;
                    case 0x63: dasmFGeneric(str, addr, op, Instr::FSMUL, M, S); return;
                    case 0x64: dasmFGeneric(str, addr, op, Instr::FDDIV, M, S); return;
                    case 0x66: dasmFGeneric(str, addr, op, Instr::FDADD, M, S); return;
                    case 0x67: dasmFGeneric(str, addr, op, Instr::FDMUL, M, S); return;
                    case 0x68: dasmFGeneric(str, addr, op, Instr::FSSUB, M, S); return;
                    case 0x6C: dasmFGeneric(str, addr, op, Instr::FDSUB, M, S); return;
                }
            }

            break;
    }

    dasmLineF(str, addr, op, I, M, S);
}

void
Moira::dasmFNop(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    str << Ins{I};
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) str << " ";
}

void
Moira::dasmFRestore(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dn = _____________xxx (op);

    str << Ins{I} << str.tab << Op(M, S, dn, addr);
}

void
Moira::dasmFSave(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dn = _____________xxx (op);

    str << Ins{I} << str.tab << Op(M, S, dn, addr);
}

void
Moira::dasmFScc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr);
    auto reg = _____________xxx (op);
    auto cnd = __________xxxxxx (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    str << Ins{I} << Fcc{cnd} << str.tab << Op(M, S, reg, addr);
}

void
Moira::dasmFTrapcc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr);
    auto cnd = __________xxxxxx (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    switch (S) {

        case Unsized:

            str << Ins{I} << Fcc{cnd} << " ";
            break;

        case Word:
        case Long:

            str << Ins{I} << Fcc{cnd} << Sz{S} << str.tab << Ims(dasmIncRead(addr, S), S);
            break;
    }
}

void
Moira::dasmFGeneric(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, [[maybe_unused]] Size S) const
{
    auto ext = dasmIncRead(addr);
    auto reg = _____________xxx (op);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);

    if (ext & 0x4000) {

        if (M == Mode::IM) {

            u64 val;

            switch (src) {

                case 0: // Long-Word Integer

                    val = dasmIncRead(addr, Long);
                    str << Ins{I} << Ffmt{src} << str.tab << Ims(u32(val), Long);
                    break;

                case 1: // Single precision

                    val = dasmIncRead(addr, Long);
                    str << Ins{I} << Ffmt{src} << str.tab << "#<fixme>";
                    break;

                case 2: // Double precision
                case 3: // Packed-Decimal Real

                    val = dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long); // Why???
                    str << Ins{I} << Ffmt{src} << str.tab << "#<fixme>";
                    break;

                case 5: // Double-precision real

                    val = dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long);
                    str << Ins{I} << Ffmt{src} << str.tab << "#<fixme>";
                    break;

                case 6: // Byte Integer
                    val = dasmIncRead(addr, Word);
                    str << Ins{I} << Ffmt{src} << str.tab << Ims(u32(val), Byte);
                    break;

                default:
                    str << Ins{I} << Ffmt{src} << str.tab << Op(M, Word, reg, addr);
            }
        } else {
            str << Ins{I} << Ffmt{src} << str.tab << Op(M, Long, reg, addr);
        }

    } else {
        str << Ins{I} << Ffmt{2} << str.tab << Fp{src};
    }

    str << Sep{} << Fp{dst};
}

void
Moira::dasmFGeneric2(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, [[maybe_unused]] Size S) const
{
    auto ext = dasmIncRead(addr);
    auto reg = _____________xxx (op);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    auto fpc = _____________xxx (ext);

    if (ext & 0x4000) {

        str << Ins{I} << Ffmt{src} << str.tab;

        if (M == Mode::IM) {

            u64 val;

            switch (src) {

                case 0: // Long-Word Integer

                    val = dasmIncRead(addr, Long);
                    str << Ims(u32(val), Long);
                    break;

                case 1: // Single precision

                    val = dasmIncRead(addr, Long);
                    str << "#<fixme>";
                    break;

                case 2: // Double precision
                case 3: // Packed-Decimal Real

                    val = dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long); // Why???
                    str << "#<fixme>";
                    break;

                case 5: // Double-precision real

                    val = dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long);
                    str << "#<fixme>";
                    break;

                case 6: // Byte Integer

                    val = dasmIncRead(addr, Word);
                    str << Ims(u32(val), Byte);
                    break;

                default:
                    str << Op(M, Word, reg, addr);
            }
        } else {
            str << Op(M, Long, reg, addr);
        }

    } else {
        str << Ins{I} << Ffmt{2} << str.tab << Fp{src};
    }

    str << Sep{} << Fp{fpc} << Sep{} << Fp{dst};
}

void
Moira::dasmFGeneric3(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, [[maybe_unused]] Size S) const
{
    auto ext = dasmIncRead(addr);
    auto reg = _____________xxx (op);
    auto src = ___xxx__________ (ext);

    if (ext & 0x4000) {

        if (M == Mode::IM) {

            u64 val;

            switch (src) {

                case 0: // Long-Word Integer

                    val = dasmIncRead(addr, Long);
                    str << Ins{I} << Ffmt{src} << str.tab << Ims(u32(val), Long);
                    break;

                case 1: // Single precision

                    val = dasmIncRead(addr, Long);
                    str << Ins{I} << Ffmt{src} << str.tab << "#<fixme>";
                    break;

                case 2: // Double precision
                case 3: // Packed-Decimal Real

                    val = dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long); // Why???
                    str << Ins{I} << Ffmt{src} << str.tab << "#<fixme>";
                    break;

                case 5: // Double-precision real

                    val = dasmIncRead(addr, Long);
                    dasmIncRead(addr, Long);
                    str << Ins{I} << Ffmt{src} << str.tab << "#<fixme>";
                    break;

                case 6: // Byte Integer

                    val = dasmIncRead(addr, Word);
                    str << Ins{I} << Ffmt{src} << str.tab << Ims(u32(val), Byte);
                    break;

                default:
                    str << Ins{I} << Ffmt{src} << str.tab << Op(M, Word, reg, addr);
            }
        } else {
            str << Ins{I} << Ffmt{src} << str.tab << Op(M, Long, reg, addr);
        }

    } else {
        str << Ins{I} << Ffmt{2} << str.tab << Fp{src};
    }
}

void
Moira::dasmFMove(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr);
    auto reg = _____________xxx (op);
    auto cod = xxx_____________ (ext);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    auto fac = _________xxxxxxx (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    switch (cod) {

        case 0b000:

            if (fac == 0x40) str << Ins{Instr::FSMOVE} << Ffmt{2};
            else if (fac == 0x44) str << Ins{Instr::FDMOVE} << Ffmt{2};
            else str << Ins{I} << Ffmt{2};

            str << str.tab << Fp(src) << Sep{} << Fp(dst);
            break;

        case 0b010:

            if (fac == 0x40) str << Ins{Instr::FSMOVE} << Ffmt{src};
            else if (fac == 0x44) str << Ins{Instr::FDMOVE} << Ffmt{src};
            else str << Ins{I} << Ffmt{src};

            if (M == Mode::IM) {

                u64 val;

                switch (src) {

                    case 0: // Long-Word Integer
                        val = dasmIncRead(addr, Long);
                        str << str.tab << Ims(u32(val), Long) << Sep{} << Fp(dst);
                        break;

                    case 1: // Single precision

                        val = dasmIncRead(addr, Long);
                        str << str.tab << "#<fixme>" << Sep{} << Fp(dst);
                        break;

                    case 2: // Double precision
                    case 3: // Packed-Decimal Real

                        val = dasmIncRead(addr, Long);
                        dasmIncRead(addr, Long);
                        dasmIncRead(addr, Long); // Why???
                        str << str.tab << "#<fixme>" << Sep{} << Fp(dst);
                        break;

                    case 5: // Double-precision real

                        val = dasmIncRead(addr, Long);
                        dasmIncRead(addr, Long);
                        str << str.tab << "#<fixme>" << Sep{} << Fp(dst);
                        break;

                    case 6: // Byte Integer
                        val = dasmIncRead(addr, Word);
                        str << str.tab << Ims(u32(val), Byte) << Sep{} << Fp(dst);
                        break;

                    default:
                        str << str.tab << Op(M, Word, reg, addr) << Sep{} << Fp(dst);
                }
            } else {
                str << str.tab << Op(M, Long, reg, addr) << Sep{} << Fp(dst);
            }
            break;

        case 0b011:

            switch (src) {

                case 0b011:

                    str << Ins{I} << Ffmt{src} << str.tab << Fp(dst) << Sep{} << Op(M, Long, reg, addr);
                    str << "{" << Ims(i8(fac << 1) >> 1, Byte) << "}";
                    break;

                case 0b111:

                    str << Ins{I} << Ffmt{3} << str.tab << Fp{dst} << Sep{} << Op(M, Long, reg, addr);
                    str << Sep{} << Dn(fac >> 4);
                    break;

                default:

                    str << Ins{I} << Ffmt{src} << str.tab << Fp{dst} << Sep{} << Op(M, Long, reg, addr);
                    break;
            }
            break;
    }
}

void
Moira::dasmFMovecr(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr);
    auto dst = ______xxx_______ (ext);
    auto ofs = _________xxxxxxx (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    str << Ins{I} << Ffmt{2} << str.tab << Imu{ofs} << Sep{} << Fp{dst};
}

void
Moira::dasmFMovem(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr);
    auto reg = _____________xxx (op);
    auto cod = xxx_____________ (ext);
    auto mod = ___xx___________ (ext);
    auto rrr = _________xxx____ (ext);
    auto lll = ___xxx__________ (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    switch (cod) {

        case 0b100: // Ea to Cntrl

            if ((ext & 0x1C00) == 0) {

                if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

                    str << "fmovel" << str.tab << Op(M, Long, reg, addr) << Sep{};
                    return;
                }
            }
            if (lll == 0 || lll == 1 || lll == 2 || lll == 4) {
                str << Ins{Instr::FMOVE} << Ffmt{0} << str.tab;
            } else {
                str << Ins{Instr::FMOVEM} << Ffmt{0} << str.tab;
            }
            str << Op(M, Long, reg, addr) << Sep{} << Fctrl{lll};
            break;

        case 0b101: // Cntrl to Ea

            if ((ext & 0x1C00) == 0) {

                if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

                    str << Ins{Instr::FMOVE} << Ffmt{0} << str.tab << Sep{} << Op(M, Long, reg, addr);
                    return;
                }
            }
            if (lll == 0 || lll == 1 || lll == 2 || lll == 4) {
                str << Ins{Instr::FMOVE} << Ffmt{0} << str.tab;
            } else {
                str << Ins{Instr::FMOVEM} << Ffmt{0} << str.tab;
            }
            str << Fctrl{lll} << Sep{} << Op(M, Long, reg, addr);
            break;

        case 0b110: // Memory to FPU

            switch (mod) {

                case 0b00: // Static list, predecrement addressing

                    str << Ins{I} << Ffmt{2} << str.tab;
                    str << Op(M, Long, reg, addr) << Sep{};
                    if (ext & 0xFF) {
                        str << FRegList(ext & 0xFF);
                    } else {
                        str << Imd{0};
                    }
                    break;

                case 0b01: // Dynamic list, predecrement addressing

                    str << Ins{I} << Ffmt{2} << str.tab;
                    str << Op(M, Long, reg, addr) << Sep{};
                    str << Dn{rrr};
                    break;

                case 0b10: // Static list, postincrement addressing

                    str << Ins{I} << Ffmt{2} << str.tab;
                    str << Op(M, Long, reg, addr) << Sep{};
                    if (ext & 0xFF) {
                        str << FRegList(REVERSE_8(ext & 0xFF));
                    } else {
                        str << Imd{0};
                    }
                    break;

                case 0b11: // Dynamic list, postincrement addressing

                    str << Ins{I} << Ffmt{2} << str.tab;
                    str << Op(M, Long, reg, addr) << Sep{};
                    str << Dn{rrr};
                    break;
            }
            break;

        case 0b111: // FPU to memory

            switch (mod) {

                case 0b00: // Static list, predecrement addressing

                    str << Ins{I} << Ffmt{2} << str.tab;
                    if (ext & 0xFF) {
                        str << FRegList(ext & 0xFF);
                    } else {
                        str << Imd{0};
                    }
                    str << Sep{} << Op(M, Long, reg, addr);
                    break;

                case 0b01: // Dynamic list, predecrement addressing

                    str << Ins{I} << Ffmt{2} << str.tab;
                    str << Dn{rrr} << Sep{};
                    str << Op(M, Long, reg, addr);
                    break;

                case 0b10: // Static list, postincrement addressing

                    str << Ins{I} << Ffmt{2} << str.tab;
                    if (ext & 0xFF) {
                        str << FRegList(REVERSE_8(ext & 0xFF)) ;
                    } else {
                        str << Imd{0};
                    }
                    str << Sep{} << Op(M, Long, reg, addr);
                    break;

                case 0b11: // Dynamic list, postincrement addressing

                    str << Ins{I} << Ffmt{2} << str.tab;
                    str << Dn{rrr} << Sep{};
                    str << Op(M, Long, reg, addr);
                    break;
            }
            break;
    }
}
