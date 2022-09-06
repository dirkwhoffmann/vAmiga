// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

template <Instr I, Mode M, Size S> void
Moira::dasmFGen(StrWriter &str, u32 &addr, u16 op)
{
    auto ext  = dasmRead<Word>(addr);
    auto cod  = xxx_____________(ext);
    auto cmd  = _________xxxxxxx(ext);
    addr -= 2;

    if (M == MODE_AN) {
        if (ext & 0x4000) { dasmLineF<I, M, S>(str, addr, op); return; }
    }
    if (M == MODE_IP) {
        if (cod == 0b010) { dasmLineF<I, M, S>(str, addr, op); return; }
    }

    // Catch FMOVE instructions
    switch (cod) {

        case 0b010:

            if ((ext & 0xFC00) == 0x5C00) { dasmFMovecr<FMOVECR, M, S>(str, addr, op); return; }
            [[fallthrough]];

        case 0b000:

            if (cmd != 0x00 && cmd != 0x40 && cmd != 0x44) break;
            [[fallthrough]];

        case 0b011:

            dasmFMove<FMOVE, M, S>(str, addr, op);
            return;

        case 0b101:
        case 0b100:
        case 0b110:
        case 0b111:

            dasmFMovem<FMOVEM, M, S>(str, addr, op);
            return;
    }

    // Catch all other instructions
    switch (cod) {

        case 0b010:
        case 0b000:

            switch (cmd) {

                case 0x01: dasmFGeneric<FINT, M, S>(str, addr, op); return;
                case 0x02: dasmFGeneric<FSINH, M, S>(str, addr, op); return;
                case 0x03: dasmFGeneric<FINTRZ, M, S>(str, addr, op); return;
                case 0x04: dasmFGeneric<FSQRT, M, S>(str, addr, op); return;
                case 0x06: dasmFGeneric<FLOGNP1, M, S>(str, addr, op); return;
                case 0x08: dasmFGeneric<FETOXM1, M, S>(str, addr, op); return;
                case 0x09: dasmFGeneric<FTANH, M, S>(str, addr, op); return;
                case 0x0A: dasmFGeneric<FATAN, M, S>(str, addr, op); return;
                case 0x0C: dasmFGeneric<FASIN, M, S>(str, addr, op); return;
                case 0x0D: dasmFGeneric<FATANH, M, S>(str, addr, op); return;
                case 0x0E: dasmFGeneric<FSIN, M, S>(str, addr, op); return;
                case 0x0F: dasmFGeneric<FTAN, M, S>(str, addr, op); return;
                case 0x10: dasmFGeneric<FETOX, M, S>(str, addr, op); return;
                case 0x11: dasmFGeneric<FTWOTOX, M, S>(str, addr, op); return;
                case 0x12: dasmFGeneric<FTENTOX, M, S>(str, addr, op); return;
                case 0x14: dasmFGeneric<FLOGN, M, S>(str, addr, op); return;
                case 0x15: dasmFGeneric<FLOG10, M, S>(str, addr, op); return;
                case 0x16: dasmFGeneric<FLOG2, M, S>(str, addr, op); return;
                case 0x18: dasmFGeneric<FABS, M, S>(str, addr, op); return;
                case 0x19: dasmFGeneric<FCOSH, M, S>(str, addr, op); return;
                case 0x1A: dasmFGeneric<FNEG, M, S>(str, addr, op); return;
                case 0x1C: dasmFGeneric<FACOS, M, S>(str, addr, op); return;
                case 0x1D: dasmFGeneric<FCOS, M, S>(str, addr, op); return;
                case 0x1E: dasmFGeneric<FGETEXP, M, S>(str, addr, op); return;
                case 0x1F: dasmFGeneric<FGETMAN, M, S>(str, addr, op); return;
                case 0x20: dasmFGeneric<FDIV, M, S>(str, addr, op); return;
                case 0x21: dasmFGeneric<FMOD, M, S>(str, addr, op); return;
                case 0x22: dasmFGeneric<FADD, M, S>(str, addr, op); return;
                case 0x23: dasmFGeneric<FMUL, M, S>(str, addr, op); return;
                case 0x24: dasmFGeneric<FSGLDIV, M, S>(str, addr, op); return;
                case 0x25: dasmFGeneric<FREM, M, S>(str, addr, op); return;
                case 0x26: dasmFGeneric<FSCAL, M, S>(str, addr, op); return;
                case 0x27: dasmFGeneric<FSGLMUL, M, S>(str, addr, op); return;
                case 0x28: dasmFGeneric<FSUB, M, S>(str, addr, op); return;
                case 0x30: dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x31: dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x32: dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x33: dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x34: dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x35: dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x36: dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x37: dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x38: dasmFGeneric<FCMP, M, S>(str, addr, op); return;
                case 0x3A: dasmFGeneric3<FTST, M, S>(str, addr, op); return;
                case 0x41: dasmFGeneric<FSSQRT, M, S>(str, addr, op); return;
                case 0x45: dasmFGeneric<FDSQRT, M, S>(str, addr, op); return;
                case 0x58: dasmFGeneric<FSABS, M, S>(str, addr, op); return;
                case 0x5A: dasmFGeneric<FSNEG, M, S>(str, addr, op); return;
                case 0x5C: dasmFGeneric<FDABS, M, S>(str, addr, op); return;
                case 0x5E: dasmFGeneric<FDNEG, M, S>(str, addr, op); return;
                case 0x60: dasmFGeneric<FSDIV, M, S>(str, addr, op); return;
                case 0x62: dasmFGeneric<FSADD, M, S>(str, addr, op); return;
                case 0x63: dasmFGeneric<FSMUL, M, S>(str, addr, op); return;
                case 0x64: dasmFGeneric<FDDIV, M, S>(str, addr, op); return;
                case 0x66: dasmFGeneric<FDADD, M, S>(str, addr, op); return;
                case 0x67: dasmFGeneric<FDMUL, M, S>(str, addr, op); return;
                case 0x68: dasmFGeneric<FSSUB, M, S>(str, addr, op); return;
                case 0x6C: dasmFGeneric<FDSUB, M, S>(str, addr, op); return;
            }
            break;
    }

    dasmLineF<I, M, S>(str, addr, op);
}

template <Instr I, Mode M, Size S> void
Moira::dasmFBcc(StrWriter &str, u32 &addr, u16 op)
{
    auto old = addr;
    auto ext = dasmRead<S>(addr);
    auto cnd = ___________xxxxx (op);

    // Check for special FNOP opcode
    if ((op & 0x7F) == 0 && ext == 0) {

        dasmFNop<FNOP, M, S>(str, addr, op);
        return;
    }

    auto dst = old + 2;
    U32_INC(dst, SEXT<S>(ext));

    if (S == Long) {
        str << Ins<I>{} << Fcc{cnd} << Sz<S>{} << tab << UInt(dst);
    } else {
        str << Ins<I>{} << Fcc{cnd} << tab << UInt(dst);
    }
}

template <Instr I, Mode M, Size S> void
Moira::dasmFDbcc(StrWriter &str, u32 &addr, u16 op)
{
    auto old = addr;
    auto ext = dasmRead(addr);
    auto src = _____________xxx (op);
    auto cnd = ___________xxxxx (ext);

    // Catch illegal extension words
    if ((str.style == DASM_GNU || str.style == DASM_GNU_MIT) && !isValidExtFPU(I, M, op, ext)) {

        addr = old;
        dasmIllegal<I, M, S>(str, addr, op);
        return;
    }

    auto dst = addr + 2;
    U32_INC(dst, SEXT<S>(dasmRead<S>(addr)));

    str << Ins<I>{} << Fcc{cnd} << tab << Dn{src} << Sep{} << UInt(dst);
}

template <Instr I, Mode M, Size S> void
Moira::dasmFNop(StrWriter &str, u32 &addr, u16 op)
{
    str << Ins<I>{};
    if (style == DASM_GNU || style == DASM_GNU_MIT) str << " ";
}

template <Instr I, Mode M, Size S> void
Moira::dasmFRestore(StrWriter &str, u32 &addr, u16 op)
{
    auto dn = _____________xxx (op);

    str << Ins<I>{} << tab << Op<M, S>(dn, addr);
}

template <Instr I, Mode M, Size S> void
Moira::dasmFSave(StrWriter &str, u32 &addr, u16 op)
{
    auto dn = _____________xxx (op);

    str << Ins<I>{} << tab << Op<M, S>(dn, addr);
}

template <Instr I, Mode M, Size S> void
Moira::dasmFScc(StrWriter &str, u32 &addr, u16 op)
{
    auto old = addr;
    auto ext = dasmRead(addr);
    auto reg = _____________xxx (op);
    auto cnd = __________xxxxxx (ext);

    // Catch illegal extension words
    if ((str.style == DASM_GNU || str.style == DASM_GNU_MIT) && !isValidExtFPU(I, M, op, ext)) {

        addr = old;
        dasmIllegal<I, M, S>(str, addr, op);
        return;
    }

    str << Ins<I>{} << Fcc{cnd} << tab << Op<M, S>(reg, addr);
}

template <Instr I, Mode M, Size S> void
Moira::dasmFTrapcc(StrWriter &str, u32 &addr, u16 op)
{
    auto old = addr;
    auto ext = dasmRead(addr);
    auto cnd = __________xxxxxx (ext);

    // Catch illegal extension words
    if ((str.style == DASM_GNU || str.style == DASM_GNU_MIT) && !isValidExtFPU(I, M, op, ext)) {

        addr = old;
        dasmIllegal<I, M, S>(str, addr, op);
        return;
    }

    switch (S) {

        case Unsized:

            str << Ins<I>{} << Fcc{cnd} << " ";
            break;

        case Word:
        case Long:

            str << Ins<I>{} << Fcc{cnd} << Sz<S>{} << tab << Ims<S>(dasmRead<S>(addr));
            break;
    }
}

template <Instr I, Mode M, Size S> void
Moira::dasmFGeneric(StrWriter &str, u32 &addr, u16 op)
{
    auto ext = dasmRead(addr);
    auto reg = _____________xxx (op);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);

    if (ext & 0x4000) {

        if (M == MODE_IM) {

            u64 val;

            switch (src) {

                case 0: // Long-Word Integer

                    val = dasmRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << tab << Ims<Long>(u32(val));
                    break;

                case 1: // Single precision

                    val = dasmRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << tab << "#<fixme>";
                    break;

                case 2: // Double precision
                case 3: // Packed-Decimal Real

                    val = dasmRead<Long>(addr);
                    dasmRead<Long>(addr);
                    dasmRead<Long>(addr); // Why???
                    str << Ins<I>{} << Ffmt{src} << tab << "#<fixme>";
                    break;

                case 5: // Double-precision real

                    val = dasmRead<Long>(addr);
                    dasmRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << tab << "#<fixme>";
                    break;

                case 6: // Byte Integer
                    val = dasmRead<Word>(addr);
                    str << Ins<I>{} << Ffmt{src} << tab << Ims<Byte>(u32(val));
                    break;

                default:
                    str << Ins<I>{} << Ffmt{src} << tab << Op<M, Word>(reg, addr);
            }
        } else {
            str << Ins<I>{} << Ffmt{src} << tab << Op<M, Long>(reg, addr);
        }

    } else {
        str << Ins<I>{} << Ffmt{2} << tab << Fp{src};
    }

    str << Sep{} << Fp{dst};
}

template <Instr I, Mode M, Size S> void
Moira::dasmFGeneric2(StrWriter &str, u32 &addr, u16 op)
{
    auto ext = dasmRead(addr);
    auto reg = _____________xxx (op);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    auto fpc = _____________xxx (ext);

    if (ext & 0x4000) {

        str << Ins<I>{} << Ffmt{src} << tab;

        if (M == MODE_IM) {

            u64 val;

            switch (src) {

                case 0: // Long-Word Integer

                    val = dasmRead<Long>(addr);
                    str << Ims<Long>(u32(val));
                    break;

                case 1: // Single precision

                    val = dasmRead<Long>(addr);
                    str << "#<fixme>";
                    break;

                case 2: // Double precision
                case 3: // Packed-Decimal Real

                    val = dasmRead<Long>(addr);
                    dasmRead<Long>(addr);
                    dasmRead<Long>(addr); // Why???
                    str << "#<fixme>";
                    break;

                case 5: // Double-precision real

                    val = dasmRead<Long>(addr);
                    dasmRead<Long>(addr);
                    str << "#<fixme>";
                    break;

                case 6: // Byte Integer

                    val = dasmRead<Word>(addr);
                    str << Ims<Byte>(u32(val));
                    break;

                default:
                    str << Op<M, Word>(reg, addr);
            }
        } else {
            str << Op<M, Long>(reg, addr);
        }

    } else {
        str << Ins<I>{} << Ffmt{2} << tab << Fp{src};
    }

    str << Sep{} << Fp{fpc} << Sep{} << Fp{dst};
}

template <Instr I, Mode M, Size S> void
Moira::dasmFGeneric3(StrWriter &str, u32 &addr, u16 op)
{
    auto ext = dasmRead(addr);
    auto reg = _____________xxx (op);
    auto src = ___xxx__________ (ext);

    if (ext & 0x4000) {

        if (M == MODE_IM) {

            u64 val;

            switch (src) {

                case 0: // Long-Word Integer

                    val = dasmRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << tab << Ims<Long>(u32(val));
                    break;

                case 1: // Single precision

                    val = dasmRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << tab << "#<fixme>";
                    break;

                case 2: // Double precision
                case 3: // Packed-Decimal Real

                    val = dasmRead<Long>(addr);
                    dasmRead<Long>(addr);
                    dasmRead<Long>(addr); // Why???
                    str << Ins<I>{} << Ffmt{src} << tab << "#<fixme>";
                    break;

                case 5: // Double-precision real

                    val = dasmRead<Long>(addr);
                    dasmRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << tab << "#<fixme>";
                    break;

                case 6: // Byte Integer

                    val = dasmRead<Word>(addr);
                    str << Ins<I>{} << Ffmt{src} << tab << Ims<Byte>(u32(val));
                    break;

                default:
                    str << Ins<I>{} << Ffmt{src} << tab << Op<M, Word>(reg, addr);
            }
        } else {
            str << Ins<I>{} << Ffmt{src} << tab << Op<M, Long>(reg, addr);
        }

    } else {
        str << Ins<I>{} << Ffmt{2} << tab << Fp{src};
    }
}

template <Instr I, Mode M, Size S> void
Moira::dasmFMove(StrWriter &str, u32 &addr, u16 op)
{
    auto old = addr;
    auto ext = dasmRead(addr);
    auto reg = _____________xxx (op);
    auto cod = xxx_____________ (ext);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    auto fac = _________xxxxxxx (ext);

    // Catch illegal extension words
    if (str.style == DASM_GNU || str.style == DASM_GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal<I, M, S>(str, addr, op);
            return;
        }
    }

    switch (cod) {

        case 0b000:

            if (fac == 0x40) str << Ins<FSMOVE>{} << Ffmt{2};
            else if (fac == 0x44) str << Ins<FDMOVE>{} << Ffmt{2};
            else str << Ins<I>{} << Ffmt{2};

            str << tab << Fp(src) << Sep{} << Fp(dst);
            break;

        case 0b010:

            if (fac == 0x40) str << Ins<FSMOVE>{} << Ffmt{src};
            else if (fac == 0x44) str << Ins<FDMOVE>{} << Ffmt{src};
            else str << Ins<I>{} << Ffmt{src};

            if (M == MODE_IM) {

                u64 val;

                switch (src) {

                    case 0: // Long-Word Integer
                        val = dasmRead<Long>(addr);
                        str << tab << Ims<Long>(u32(val)) << Sep{} << Fp(dst);
                        break;

                    case 1: // Single precision

                        val = dasmRead<Long>(addr);
                        str << tab << "#<fixme>" << Sep{} << Fp(dst);
                        break;

                    case 2: // Double precision
                    case 3: // Packed-Decimal Real

                        val = dasmRead<Long>(addr);
                        dasmRead<Long>(addr);
                        dasmRead<Long>(addr); // Why???
                        str << tab << "#<fixme>" << Sep{} << Fp(dst);
                        break;

                    case 5: // Double-precision real

                        val = dasmRead<Long>(addr);
                        dasmRead<Long>(addr);
                        str << tab << "#<fixme>" << Sep{} << Fp(dst);
                        break;

                    case 6: // Byte Integer
                        val = dasmRead<Word>(addr);
                        str << tab << Ims<Byte>(u32(val)) << Sep{} << Fp(dst);
                        break;

                    default:
                        str << tab << Op<M, Word>(reg, addr) << Sep{} << Fp(dst);
                }
            } else {
                str << tab << Op<M, Long>(reg, addr) << Sep{} << Fp(dst);
            }
            break;

        case 0b011:

            switch (src) {

                case 0b011:

                    str << Ins<I>{} << Ffmt{src} << tab << Fp(dst) << Sep{} << Op<M, Long>(reg, addr);
                    str << "{" << Ims<Byte>(i8(fac << 1) >> 1) << "}";
                    break;

                case 0b111:

                    str << Ins<I>{} << Ffmt{3} << tab << Fp{dst} << Sep{} << Op<M, Long>(reg, addr);
                    str << Sep{} << Dn(fac >> 4);
                    break;

                default:

                    str << Ins<I>{} << Ffmt{src} << tab << Fp{dst} << Sep{} << Op<M, Long>(reg, addr);
                    break;
            }
            break;
    }
}

template <Instr I, Mode M, Size S> void
Moira::dasmFMovecr(StrWriter &str, u32 &addr, u16 op)
{
    auto old = addr;
    auto ext = dasmRead(addr);
    auto dst = ______xxx_______ (ext);
    auto ofs = _________xxxxxxx (ext);

    // Catch illegal extension words
    if (str.style == DASM_GNU || str.style == DASM_GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal<I, M, S>(str, addr, op);
            return;
        }
    }

    str << Ins<I>{} << Ffmt{2} << tab << Imu{ofs} << Sep{} << Fp{dst};
}

template <Instr I, Mode M, Size S> void
Moira::dasmFMovem(StrWriter &str, u32 &addr, u16 op)
{
    auto old = addr;
    auto ext = dasmRead(addr);
    auto reg = _____________xxx (op);
    auto cod = xxx_____________ (ext);
    auto mod = ___xx___________ (ext);
    auto rrr = _________xxx____ (ext);
    auto lll = ___xxx__________ (ext);

    // Catch illegal extension words
    if (str.style == DASM_GNU || str.style == DASM_GNU_MIT) {

        if (!isValidExtFPU(I, M, op, ext)) {

            addr = old;
            dasmIllegal<I, M, S>(str, addr, op);
            return;
        }
    }

    switch (cod) {

        case 0b100: // Ea to Cntrl

            if ((ext & 0x1C00) == 0) {

                if (str.style == DASM_GNU || str.style == DASM_GNU_MIT) {

                    str << "fmovel" << tab << Op<M, Long>(reg, addr) << Sep{};
                    return;
                }
            }
            if (lll == 0 || lll == 1 || lll == 2 || lll == 4) {
                str << Ins<FMOVE>{} << Ffmt{0} << tab;
            } else {
                str << Ins<FMOVEM>{} << Ffmt{0} << tab;
            }
            str << Op<M, Long>(reg, addr) << Sep{} << Fctrl{lll};
            break;

        case 0b101: // Cntrl to Ea

            if ((ext & 0x1C00) == 0) {

                if (str.style == DASM_GNU || str.style == DASM_GNU_MIT) {

                    str << "fmovel" << tab << Sep{} << Op<M, Long>(reg, addr);
                    return;
                }
            }
            if (lll == 0 || lll == 1 || lll == 2 || lll == 4) {
                str << Ins<FMOVE>{} << Ffmt{0} << tab;
            } else {
                str << Ins<FMOVEM>{} << Ffmt{0} << tab;
            }
            str << Fctrl{lll} << Sep{} << Op<M, Long>(reg, addr);
            break;

        case 0b110: // Memory to FPU

            switch (mod) {

                case 0b00: // Static list, predecrement addressing

                    str << Ins<I>{} << Ffmt{2} << tab;
                    str << Op<M, Long>(reg, addr) << Sep{};
                    if (ext & 0xFF) {
                        str << FRegList(ext & 0xFF);
                    } else {
                        str << Imd{0};
                    }
                    break;

                case 0b01: // Dynamic list, predecrement addressing

                    str << Ins<I>{} << Ffmt{2} << tab;
                    str << Op<M, Long>(reg, addr) << Sep{};
                    str << Dn{rrr};
                    break;

                case 0b10: // Static list, postincrement addressing

                    str << Ins<I>{} << Ffmt{2} << tab;
                    str << Op<M, Long>(reg, addr) << Sep{};
                    if (ext & 0xFF) {
                        str << FRegList(REVERSE_8(ext & 0xFF));
                    } else {
                        str << Imd{0};
                    }
                    break;

                case 0b11: // Dynamic list, postincrement addressing

                    str << Ins<I>{} << Ffmt{2} << tab;
                    str << Op<M, Long>(reg, addr) << Sep{};
                    str << Dn{rrr};
                    break;
            }
            break;

        case 0b111: // FPU to memory

            switch (mod) {

                case 0b00: // Static list, predecrement addressing

                    str << Ins<I>{} << Ffmt{2} << tab;
                    if (ext & 0xFF) {
                        str << FRegList(ext & 0xFF);
                    } else {
                        str << Imd{0};
                    }
                    str << Sep{} << Op<M, Long>(reg, addr);
                    break;

                case 0b01: // Dynamic list, predecrement addressing

                    str << Ins<I>{} << Ffmt{2} << tab;
                    str << Dn{rrr} << Sep{};
                    str << Op<M, Long>(reg, addr);
                    break;

                case 0b10: // Static list, postincrement addressing

                    str << Ins<I>{} << Ffmt{2} << tab;
                    if (ext & 0xFF) {
                        str << FRegList(REVERSE_8(ext & 0xFF)) ;
                    } else {
                        str << Imd{0};
                    }
                    str << Sep{} << Op<M, Long>(reg, addr);
                    break;

                case 0b11: // Dynamic list, postincrement addressing

                    str << Ins<I>{} << Ffmt{2} << tab;
                    str << Dn{rrr} << Sep{};
                    str << Op<M, Long>(reg, addr);
                    break;
            }
            break;
    }
}
