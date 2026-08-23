// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

int
Moira::disassemble(char *str, u32 addr) const
{
    if constexpr (MOIRA_ENABLE_DASM == false) {
        throw std::runtime_error("This feature requires MOIRA_ENABLE_DASM = true\n");
    }

    u32 pc = addr;
    u16 opcode = read16Dasm(pc);

    StrWriter writer(str, instrStyle);

    auto &nfo = info[opcode];
    (this->*dasm[opcode])(writer, pc, opcode, nfo.I, nfo.M, nfo.S);
    writer << Finish{};

    // Post process disassembler output
    switch (instrStyle.letterCase) {

        case LetterCase::MIXED_CASE:

            break;

        case LetterCase::LOWER_CASE:

            for (auto p = writer.base; p < writer.ptr; p++) {
                *p = char(std::tolower(*p));
            }
            break;

        case LetterCase::UPPER_CASE:

            for (auto p = writer.base; p < writer.ptr; p++) {
                *p = char(std::toupper(*p));
            }
            break;
    }

    return pc - addr + 2;
}

void
Moira::disassembleSR(char *str, const StatusRegister &sr) const
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

void
Moira::dump8(char *str, u8 value) const
{
    StrWriter(str, dataStyle) << UInt8{value} << Finish{};
}

void
Moira::dump16(char *str, u16 value) const
{
    StrWriter(str, dataStyle) << UInt16{value} << Finish{};
}

void
Moira::dump24(char *str, u32 value) const
{
    StrWriter(str, dataStyle) << UInt24{value} << Finish{};
}

void
Moira::dump32(char *str, u32 value) const
{
    StrWriter(str, dataStyle) << UInt32{value} << Finish{};
}

void
Moira::dump16(char *str, u16 values[], int cnt) const
{
    StrWriter writer(str, dataStyle);

    for (int i = 0; i < cnt; i++) {

        for (int j = 0; i && j < dataStyle.tab; j++) writer << ' ';
        writer << UInt16{values[i]} << Finish{};
    }
}

void
Moira::dump16(char *str, u32 addr, int cnt) const
{
    StrWriter writer(str, dataStyle);

    for (int i = 0; i < cnt; i++) {

        for (int j = 0; i && j < dataStyle.tab; j++) writer << ' ';
        writer << UInt16{u16(dasmRead(addr, Word))} << Finish{};
        U32_INC(addr, 2);
    }
}

u32
Moira::dasmRead(u32 addr, Size S) const
{
    switch (S) {

        case Long:

            return dasmRead(addr, Word) << 16 | dasmRead(U32_ADD(addr, 2), Word);

        default:

            return read16Dasm(addr);
    }
}

u32
Moira::dasmIncRead(u32 &addr, Size S) const
{
    switch (S) {

        case Long:

            return dasmIncRead(addr, Word) << 16 | dasmIncRead(addr, Word);

        default:

            U32_INC(addr, 2);
            return read16Dasm(addr);
    }
}

Ea
Moira::Op(Mode M, Size S, u16 reg, u32 &pc) const
{
    Ea result;
    result.m = M;
    result.sz = S;
    result.reg = reg;
    result.pc = pc;

    // Read extension words
    switch (M)
    {
        case Mode(5):  // (d,An)
        case Mode(7):  // ABS.W
        case Mode(9):  // (d,PC)
        {
            result.ext1 = dasmIncRead(pc, Word);
            break;
        }
        case Mode(8):  // ABS.L
        {
            result.ext1 = dasmIncRead(pc, Word);
            result.ext1 = result.ext1 << 16 | dasmIncRead(pc, Word);
            break;
        }
        case Mode(6):  // (d,An,Xi)
        case Mode(10): // (d,PC,Xi)
        {
            result.ext1 = dasmIncRead(pc, Word);
            result.ext2 = 0;
            result.ext3 = 0;

            if (result.ext1 & 0x100) {

                result.dw = u8(baseDispWords((u16)result.ext1));
                result.ow = u8(outerDispWords((u16)result.ext1));

                // Compensate Musashi bug (?)
                if (instrStyle.syntax == Syntax::MUSASHI && (result.ext1 & 0x47) >= 0x44) {

                    result.ow = 0;
                }

                if (result.dw == 1) result.ext2 = (i16)dasmIncRead(pc, Word);
                if (result.dw == 2) result.ext2 = (i32)dasmIncRead(pc, Long);
                if (result.ow == 1) result.ext3 = (i16)dasmIncRead(pc, Word);
                if (result.ow == 2) result.ext3 = (i32)dasmIncRead(pc, Long);
            }
            break;
        }
        case Mode(11): // Imm
        {
            result.ext1 = dasmIncRead(pc, S);
            break;
        }
        default:
        {
            break;
        }
    }

    return result;
}

void
Moira::dasmIllegal(StrWriter &str, u32 &addr, u16 op, [[maybe_unused]] Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    switch (str.style.syntax) {

       case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << ".short " << Int{op};
            break;

        case Syntax::MUSASHI:

            str << "dc.w " << UInt16{op} << "; ILLEGAL";
            break;

        default:

            str << "dc.w " << str.tab << UInt16{op} << "; ILLEGAL";
            break;
    }
}

void
Moira::dasmLineA(StrWriter &str, u32 &addr, u16 op, [[maybe_unused]] Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    switch (str.style.syntax) {

       case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << ".short " << Int{op};
            break;

        default:

            str << "dc.w " << str.tab << UInt16{op} << "; opcode 1010";
    }
}

void
Moira::dasmLineF(StrWriter &str, u32 &addr, u16 op, [[maybe_unused]] Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    switch (str.style.syntax) {

       case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << ".short " << Int{op};
            break;

        default:

            str << "dc.w " << str.tab << UInt16{op} << "; opcode 1111";
    }
}

void
Moira::dasmShiftRg(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto dst = Dn ( _____________xxx(op) );
    auto src = Dn ( ____xxx_________(op) );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmShiftIm(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = Imd ( ____xxx_________(op) );
    auto dst = Dn  ( _____________xxx(op) );

    if (src.raw == 0) src.raw = 8;
    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmShiftEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << src;
}

void
Moira::dasmAbcdRg(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(M, S, ____xxx_________(op), addr);

    str << Ins{I} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAbcdEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(M, S, ____xxx_________(op), addr);

    str << Ins{I} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAddEaRg(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAddRgEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Dn       ( ____xxx_________(op)       );
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAdda(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = An       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAddiRg(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);
    auto dst = Dn ( _____________xxx(op) );

    str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << dst;
}

void
Moira::dasmAddiEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << dst;
}

void
Moira::dasmAddqDn(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = ____xxx_________(op);
    auto dst = _____________xxx(op);

    if (src == 0) src = 8;

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << Sz{S} << str.tab << Imd{src} << Sep{} << Dn{dst};
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << Dn{dst};
    }
}

void
Moira::dasmAddqAn(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = ____xxx_________(op);
    auto dst = _____________xxx(op);

    if (src == 0) src = 8;

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << Sz{S} << str.tab << Imd{src} << Sep{} << An{dst};
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << An{dst};
    }
}

void
Moira::dasmAddqEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src =          ( ____xxx_________(op)       );
    auto dst = Op(M, S, _____________xxx(op), addr);

    if (src == 0) src = 8;

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << Sz{S} << str.tab << Imd{src} << Sep{} << dst;
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << dst;
    }
}

void
Moira::dasmAddxRg(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(M, S, ____xxx_________(op), addr);

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAddxEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(M, S, ____xxx_________(op), addr);

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAndEaRg(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAndRgEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Dn       ( ____xxx_________(op)       );
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmAndiRg(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);
    auto dst = _____________xxx(op);

    if (str.style.syntax == Syntax::MUSASHI) {
        str << Ins{I} << Sz{S} << str.tab << Imu(src, S) << Sep{} << Dn{dst};
    } else {
        str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << Dn{dst};
    }
}

void
Moira::dasmAndiEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);
    auto dst = Op(M, S, _____________xxx(op), addr);

    if (str.style.syntax == Syntax::MUSASHI) {
        str << Ins{I} << Sz{S} << str.tab << Imu(src, S) << Sep{} << dst;
    } else {
        str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << "," << dst;
    }
}

void
Moira::dasmAndiccr(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);

    if (str.style.syntax == Syntax::MUSASHI) {
        str << Ins{I} << str.tab << Imu(src, S) << Sep{} << Ccr{};
    } else {
        str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << Ccr{};
    }
}

void
Moira::dasmAndisr(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);

    if (str.style.syntax == Syntax::MUSASHI) {
        str << Ins{I} << str.tab << Imu{src} << Sep{} << Sr{};
    } else {
        str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << Sr{};
    }
}

void
Moira::dasmBitFieldDn(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);
    auto dst = _____________xxx(op);
    auto o   = _____xxxxx______(ext);
    auto w   = ___________xxxxx(ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExt(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    str << Ins{I} << str.tab;

    if (I == Instr::BFINS) {
        str << Dn ( _xxx____________(ext) ) << Sep{};
    }

    str << Op(M, S, dst, addr);

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            switch (ext & 0x0820) {

                case 0x0000: str << "," << o << "," << w; break;
                case 0x0020: str << "," << o << "," << Dn{w&7}; break;
                case 0x0800: str << "," << Dn{o&7} << "," << w; break;
                case 0x0820: str << "," << Dn{o&7} << "," << Dn{w&7}; break;
            }
            break;

        default:

            if (w == 0) w = 32;

            switch (ext & 0x0820) {

                case 0x0000: str << " {" << o << ":" << w << "}"; break;
                case 0x0020: str << " {" << o << ":" << Dn{w&7} << "}"; break;
                case 0x0800: str << " {" << Dn{o&7} << ":" << w << "}"; break;
                case 0x0820: str << " {" << Dn{o&7} << ":" << Dn{w&7} << "}"; break;
            }
    }

    if (I == Instr::BFEXTU || I == Instr::BFEXTS || I == Instr::BFFFO) {
        str << Sep{} << Dn ( _xxx____________(ext) );
    }
    str << Av{I, M, S};
}

void
Moira::dasmBitFieldEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    dasmBitFieldDn(str, addr, op, I, M, S);
}

void
Moira::dasmBkpt(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto nr = _____________xxx(op);

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << str.tab << Imd(nr) << Av{I, M, S};
            break;

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << str.tab << Int(nr);
            break;

        default:

            str << Ins{I} << str.tab << Imu(nr);
    }
}

void
Moira::dasmBsr(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    dasmBcc(str, addr, op, I, M, S);
}

void
Moira::dasmCallm(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = dasmIncRead(addr) & 0xFF;
    auto dst = Op(M, S, _____________xxx(op), addr);

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << str.tab << Ims(src, Byte) << Sep{} << dst;
            break;

        default:

            str << Ins{I} << str.tab << Imu(src) << Sep{} << dst << Av{I, M, S};
    }
}

void
Moira::dasmCas(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);
    auto dc  = Dn ( _____________xxx(ext) );
    auto du  = Dn ( _______xxx______(ext) );
    auto dst = Op(M, S, _____________xxx(op), addr);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExt(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    str << Ins{I} << Sz{S} << str.tab << dc << Sep{} << du << Sep{} << dst;
    str << Av{I, M, S};
}

void
Moira::dasmCas2(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Long);
    auto dc1 = Dn ( (ext >> 16) & 0b111  );
    auto dc2 = Dn ( (ext >> 0)  & 0b111  );
    auto du1 = Dn ( (ext >> 22) & 0b111  );
    auto du2 = Dn ( (ext >> 6)  & 0b111  );
    auto rn1 = Rn ( (ext >> 28) & 0b1111 );
    auto rn2 = Rn ( (ext >> 12) & 0b1111 );

    // Catch illegal extension words (binutils only checks the first word)
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExt(I, M, op, u16(ext >> 16))) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    auto fill = str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT ? ',' : ':';

    str << Ins{I} << Sz{S} << str.tab;

    switch (str.style.syntax) {

        case Syntax::MOIRA_MIT:
        case Syntax::GNU_MIT:

            str << dc1 << fill << dc2 << Sep{} << du1 << fill << du2 << Sep{};
            rn1.raw < 8 ? str << "@(" << rn1 << ')' : str << rn1 << '@';
            str << fill;
            rn2.raw < 8 ? str << "@(" << rn2 << ')' : str << rn2 << '@';
            break;

        default:

            str << dc1 << fill << dc2 << Sep{} << du1 << fill << du2 << Sep{};
            str << '(' << rn1 << ')' << fill << '(' << rn2 << ')';
    }

    str << Av{I, M, S};
}

void
Moira::dasmChk(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Dn       ( ____xxx_________(op)       );

    if (str.style.syntax != Syntax::MUSASHI && !isAvailable(dasmModel, I, M, S)) {

        addr = old;
        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
    str << Av{I, M, S};
}

void
Moira::dasmChkCmp2(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Rn       ( xxxx____________(ext)      );

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExt(I, M, op, ext)) {
            
            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    if (ext & 0x0800) {
        str << Ins{Instr::CHK2} << Sz{S} << str.tab << src << Sep{} << dst;
    } else {
        str << Ins{Instr::CMP2} << Sz{S} << str.tab << src << Sep{} << dst;
    }
    str << Av{I, M, S};
}

void
Moira::dasmClr(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << dst;
}

void
Moira::dasmCmp(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    Ea src = Op(M, S, _____________xxx(op), addr);
    Dn      dst = Dn       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmCmpa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = An       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmCmpiRg(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);
    auto dst = Dn ( _____________xxx(op) );

    str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << dst;
    str << Av{I, M, S};
}

void
Moira::dasmCmpiEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << Ims(src, S) << Sep{} << dst;
    str << Av{I, M, S};
}

void
Moira::dasmCmpm(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(M, S, ____xxx_________(op), addr);

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmCpBcc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    auto id   = ( ____xxx_________(op) );
    auto cnd  = ( __________xxxxxx(op) );
    auto pc   = addr + 2;
    auto ext1 = dasmIncRead(addr, Word);
    auto disp = dasmIncRead(addr, S);
    auto ext2 = dasmIncRead(addr, Word);

    pc += SEXT(disp, S);

    str << id << Ins{I} << Cpcc{cnd} << str.tab << Ims(ext2, Word);
    str << "; " << UInt(pc) << " (extension = " << Int(ext1) << ") (2-3)";
}

void
Moira::dasmCpDbcc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    auto pc   = addr + 2;
    auto ext1 = dasmIncRead(addr, Word);
    auto ext2 = dasmIncRead(addr, Word);

    auto dn   = ( _____________xxx(op)   );
    auto id   = ( ____xxx_________(op)   );
    auto cnd  = ( __________xxxxxx(ext1) );

    auto ext3 = dasmIncRead(addr, Word);
    auto ext4 = dasmIncRead(addr, Word);

    pc += i16(ext3);

    str << id << Ins{I} << Cpcc{cnd} << str.tab << Dn{dn} << "," << Ims(ext4, Word);
    str << "; " << UInt(pc) << " (extension = " << Int(ext2) << ") (2-3)";
}

void
Moira::dasmCpGen(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    auto id  = ( ____xxx_________(op) );
    auto ext = Imu ( dasmIncRead(addr, Long) );

    str << id << Ins{I} << str.tab << ext;
    str << Av{I, M, S};
}

void
Moira::dasmCpRestore(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    auto dn = ( _____________xxx(op) );
    auto id = ( ____xxx_________(op) );
    auto ea = Op(M, S, dn, addr);

    str << id << Ins{I} << " " << ea;
    str << Av{I, M, S};
}

void
Moira::dasmCpSave(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    auto dn = ( _____________xxx(op) );
    auto id = ( ____xxx_________(op) );
    auto ea = Op(M, S, dn, addr);

    str << id << Ins{I} << str.tab << ea;
    str << Av{I, M, S};
}

void
Moira::dasmCpScc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    auto dn   = ( _____________xxx(op) );
    auto id   = ( ____xxx_________(op) );
    auto ext1 = dasmIncRead(addr, Word);
    auto cnd  = ( __________xxxxxx(ext1) );
    auto ext2 = dasmIncRead(addr, Word);
    auto ea   = Op(M, S, dn, addr);

    str << id << Ins{I} << Cpcc{cnd} << str.tab << ea;
    str << "; (extension = " << Int(ext2) << ") (2-3)";
}

void
Moira::dasmCpTrapcc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    auto ext1 = dasmIncRead(addr, Word);
    auto id   = ( ____xxx_________(op)   );
    auto cnd  = ( __________xxxxxx(ext1) );

    if (id == 0) {

        auto ext2 = dasmIncRead(addr, Word);

        switch (op & 0b111) {

            case 0b010:
            {
                auto ext = dasmIncRead(addr, Word);
                str << id << Ins{I} << Cpcc{cnd} << Tab{9};
                str << Tab{10} << Imu(ext);
                str << "; (extension = " << Int(ext2) << ") (2-3)";
                break;
            }
            case 0b011:
            {
                auto ext = dasmIncRead(addr, Long);
                str << id << Ins{I} << Cpcc{cnd} << Tab{9};
                str << Tab{10} << Imu(ext);
                str << "; (extension = " << Int(ext2) << ") (2-3)";
                break;
            }
            case 0b100:
            {
                // (void)dasmIncRead(addr, Long);
                str << id << Ins{I} << Cpcc{cnd} << Tab{9};
                str << "; (extension = " << Int(ext2) << ") (2-3)";
                break;
            }
            default:

                addr -= 4;
                str << "MMU 001 group";
                break;
        }

    } else {

        auto ext2 = dasmIncRead(addr, Word);

        switch (op & 0b111) {

            case 0b010:
            {
                auto ext = dasmIncRead(addr, Word);
                str << id << Ins{I} << Cpcc{cnd} << Tab{9};
                str << Tab{10} << Imu(ext);
                str << "; (extension = " << Int(ext2) << ") (2-3)";
                break;
            }
            case 0b011:
            {
                auto ext = dasmIncRead(addr, Long);
                str << id << Ins{I} << Cpcc{cnd} << Tab{9};
                str << Tab{10} << Imu(ext);
                str << "; (extension = " << Int(ext2) << ") (2-3)";
                break;
            }
            case 0b100:
            {
                // (void)dasmIncRead(addr, Long);
                str << id << Ins{I} << Cpcc{cnd} << Tab{9};
                str << "; (extension = " << Int(ext2) << ") (2-3)";
                break;
            }
            default:

                addr -= 4;
                dasmLineF(str, addr, op, I, M, S);
                break;
        }
    }
}

void
Moira::dasmBcc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    u32 dst = addr;
    U32_INC(dst, 2);
    U32_INC(dst, S == Byte ? (i8)op : SEXT(dasmIncRead(addr, S), S));

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            if (S == Byte) {

                if ((u8)op == 0xFF) {

                    dasmIllegal(str, addr, op, I, M, S);
                    break;
                }
            }

            str << Ins{I} << str.tab << UInt(dst) << Av{I, M, S};
            break;

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << Szb{S} << str.tab << UInt(dst);
            break;

        default:

            str << Ins{I} << str.tab << UInt(dst) << Av{I, M, S};
            break;
    }
}

void
Moira::dasmBra(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    dasmBcc(str, addr, op, I, M, S);
}

void
Moira::dasmBitDxDy(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Dn       ( ____xxx_________(op)       );
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmBitDxEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Dn       ( ____xxx_________(op)       );
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmBitImDy(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);
    auto dst = Op(M, S, _____________xxx(op), addr);

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << str.tab << Ims(src, S) << Sep{} << dst;
            break;

        default:

            str << Ins{I} << str.tab << Imu(src, S) << Sep{} << dst;
    }
}

void
Moira::dasmBitImEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);
    auto dst = Op(M, S, _____________xxx(op), addr);

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << str.tab << Ims(src, S) << Sep{} << dst;
            break;

        default:

            str << Ins{I} << str.tab << Imu(src, S) << Sep{} << dst;
    }
}

void
Moira::dasmDbcc(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto src = Dn ( _____________xxx(op) );
    auto dst = addr + 2;

    U32_INC(dst, (i16)dasmIncRead(addr, Word));

    str << Ins{I} << str.tab << src << Sep{} << UInt(dst);
}

void
Moira::dasmExgDxDy(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto src = Dn ( ____xxx_________(op) );
    auto dst = Dn ( _____________xxx(op) );

    str << Ins{I} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmExgAxDy(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto src = Dn ( ____xxx_________(op) );
    auto dst = An ( _____________xxx(op) );

    str << Ins{I} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmExgAxAy(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto src = An ( ____xxx_________(op) );
    auto dst = An ( _____________xxx(op) );

    str << Ins{I} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmExt(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = Dn ( _____________xxx(op) );

    str << Ins{I} << Sz{S} << str.tab << Dn{src};
}

void
Moira::dasmExtb(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Dn ( _____________xxx(op) );

    str << Ins{I} << Sz{S} << str.tab << Dn{src};
    str << Av{I, M, S};
}

void
Moira::dasmJmp(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << str.tab << src;
}

void
Moira::dasmJsr(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << str.tab << src;
}

void
Moira::dasmLea(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = An       ( ____xxx_________(op)       );

    str << Ins{I} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmLink(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dsp = dasmIncRead(addr, S);
    auto src = An ( _____________xxx(op) );

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << Sz{S} << str.tab << src << Sep{} << Ims(dsp, S);
            break;

        default:

            str << Ins{I} << str.tab << src << Sep{} << Ims(dsp, S) << Av{I, M, S};
    }
}

void
Moira::dasmMove0(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMove2(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(Mode::AI, S, ____xxx_________(op), addr );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMove3(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(Mode::PI, S, ____xxx_________(op), addr );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMove4(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(Mode::PD, S, ____xxx_________(op), addr );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMove5(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(Mode::DI, S, ____xxx_________(op), addr );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMove6(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(Mode::IX, S, ____xxx_________(op), addr );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMove7(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(Mode::AW, S, ____xxx_________(op), addr );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMove8(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Op(Mode::AL, S, ____xxx_________(op), addr );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMovea(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = An       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMovecRcRx(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = u16(dasmIncRead(addr, Word));
    auto src = Cn(____xxxxxxxxxxxx(ext));
    auto dst = Rn(xxxx____________(ext));

    str << Ins{I} << str.tab << src << Sep{} << dst;
    str << Av{I, M, S, ext};
}

void
Moira::dasmMovecRxRc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = u16(dasmIncRead(addr, Word));
    auto dst = Cn(____xxxxxxxxxxxx(ext));
    auto src = Rn(xxxx____________(ext));

    str << Ins{I} << str.tab << src << Sep{} << dst;
    str << Av{I, M, S, ext};
}

void
Moira::dasmMovemEaRg(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dst = RegRegList ( (u16)dasmIncRead(addr, Word)  );
    auto src = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMovemRgEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = RegRegList ( (u16)dasmIncRead(addr, Word)  );
    auto dst = Op(M, S, _____________xxx(op), addr);

    if (M == Mode(4)) { src.raw = REVERSE_16(src.raw); }
    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMovepDxEa(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = Dn             ( ____xxx_________(op)       );
    auto dst = Op(Mode::DI, S, _____________xxx(op), addr );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;

}

void
Moira::dasmMovepEaDx(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = Op(Mode::DI, S, _____________xxx(op), addr );
    auto dst = Dn             ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMoveq(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto dst = Dn ( ____xxx_________(op) );

    str << Ins{I} << str.tab << Ims(op, Byte) << Sep{} << dst;
}

void
Moira::dasmMoves(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = (u16)dasmIncRead(addr, Word);
    auto ea = Op(M, S, _____________xxx(op), addr);
    auto rg = Rn ( xxxx____________(ext) );

    if (str.style.syntax != Syntax::MUSASHI && !isAvailable(dasmModel, I, M, S, ext)) {

        addr = old;
        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    if (ext & 0x800) {      // Rg -> Ea
        str << Ins{I} << Sz{S} << str.tab << rg << Sep{} << ea;
    } else {                // Ea -> Rg
        str << Ins{I} << Sz{S} << str.tab << ea << Sep{} << rg;
    }
    str << Av{I, M, S};
}

void
Moira::dasmMoveCcrRg(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dst = Dn ( _____________xxx(op) );

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << str.tab << Ccr{} << Sep{} << dst << Av{I, M, S};
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << Ccr{} << Sep{} << dst;
    }
}

void
Moira::dasmMoveCcrEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dst = Op(M, S, _____________xxx(op), addr);

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << str.tab << Ccr{} << Sep{} << dst << Av{I, M, S};
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << Ccr{} << Sep{} << dst;
    }
}

void
Moira::dasmMoveToCcr(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = _____________xxx(op);

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << str.tab << Op(M, Byte, src, addr) << Sep{} << Ccr{};
            break;

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << Sz{S} << str.tab << Op(M, S, src, addr) << Sep{} << Ccr{};
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << Op(M, Byte, src, addr) << Sep{} << Ccr{};
    }
}

void
Moira::dasmMoveSrRg(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto dst = Dn ( _____________xxx(op) );

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << str.tab << Sr{} << Sep{} << dst;
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << Sr{} << Sep{} << dst;
    }
}

void
Moira::dasmMoveSrEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dst = Op(M, S, _____________xxx(op), addr);

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << str.tab << Sr{} << Sep{} << dst;
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << Sr{} << Sep{} << dst;
    }
}

void
Moira::dasmMoveToSr(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);

    switch (str.style.syntax) {

        case  Syntax::MUSASHI:

            str << Ins{I} << str.tab << src << Sep{} << Sr{};
            break;

        default:

            str << Ins{I} << Sz{S} << str.tab << src << Sep{} << Sr{};
    }
}

void
Moira::dasmMoveUspAn(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto dst = An ( _____________xxx(op) );

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << Sz{S} << str.tab << Usp{} << Sep{} << dst;
            break;

        default:

            str << Ins{I} << str.tab << Usp{} << Sep{} << dst;
    }
}

void
Moira::dasmMoveAnUsp(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = An ( _____________xxx(op) );

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << Sz{S} << str.tab << src << Sep{} << Usp{};
            break;

        default:

            str << Ins{I} << str.tab << src << Sep{} << Usp{};
    }
}

void
Moira::dasmMuls(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmMulu(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    dasmMuls(str, addr, op, I, M, S);
}

void
Moira::dasmMull(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dl  = Dn       ( _xxx____________(ext)      );
    auto dh  = Dn       ( _____________xxx(ext)      );

    auto fill = str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT ? "," : ":";

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            // Catch illegal extension words
            if (!isValidExt(I, M, op, ext)) {

                addr = old;
                dasmIllegal(str, addr, op, I, M, S);
                return;
            }
            [[fallthrough]];

        default:

            (ext & 1 << 11) ? str << Ins{Instr::MULS} : str << Ins{Instr::MULU};
            str << Sz{S} << str.tab << src << Sep{};
            (ext & 1 << 10) ? str << dh << fill << dl : str << dl;
            str << Av{I, M, S};
    }
}

void
Moira::dasmDivs(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmDivu(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins{I} << Sz{S} << str.tab << src << Sep{} << dst;
}

void
Moira::dasmDivl(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);
    auto src = Op(M, S, _____________xxx(op), addr);
    auto dl  = Dn       ( _xxx____________(ext)      );
    auto dh  = Dn       ( _____________xxx(ext)      );

    auto fill = str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT ? "," : ":";

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            // Catch illegal extension words
            if (!isValidExt(I, M, op, ext)) {

                addr = old;
                dasmIllegal(str, addr, op, I, M, S);
                return;
            }
            [[fallthrough]];

        default:

            (ext & 1 << 11) ? str << Ins{Instr::DIVS} : str << Ins{Instr::DIVU};

            if (ext & 1 << 10) {

                // DIVS.L <ea>,Dr:Dq    (64-bit dividend)
                str << Sz{S} << str.tab << src << Sep{} << dh << fill << dl;

            } else {

                // DIVSL.L <ea>,Dr:Dq   (32-bit dividend)
                if (dl.raw == dh.raw && str.style.syntax == Syntax::MUSASHI) {
                    str << Sz{S} << str.tab << src << Sep{} << dh;
                } else {
                    str << "l" << Sz{S} << str.tab << src << Sep{} << dh << fill << dl;

                }
            }
            str << Av{I, M, S};
    }
}

void
Moira::dasmNbcdRg(StrWriter &str, u32 &addr, u16 op, [[maybe_unused]] Instr I, Mode M, Size S) const
{
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{Instr::NBCD} << str.tab << dst;
}

void
Moira::dasmNbcdEa(StrWriter &str, u32 &addr, u16 op, [[maybe_unused]] Instr I, Mode M, Size S) const
{
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{Instr::NBCD} << str.tab << dst;
}

void
Moira::dasmNop(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    str << Ins{I};
}

void
Moira::dasmPackDn(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = dasmIncRead(addr, Word);
    auto rx = Op(M, S, _____________xxx(op), addr);
    auto ry = Op(M, S, ____xxx_________(op), addr);

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << str.tab << rx << Sep{} << ry << Sep{} << Ims(ext, S);
            break;

        default:

            str << Ins{I} << str.tab << rx << Sep{} << ry << Sep{} << Imu(ext);
            str << Av{I, M, S};
    }
}

void
Moira::dasmPackPd(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    dasmPackDn(str, addr, op, I, M, S);
}

void
Moira::dasmPea(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << str.tab << src;
}

void
Moira::dasmReset(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    str << Ins{I};
}

void
Moira::dasmRtd(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto disp = dasmIncRead(addr, Word);

    str << Ins{I} << str.tab << Ims(disp, Word);
    str << Av{I, M, S};
}

void
Moira::dasmRte(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    str << Ins{I};
}

void
Moira::dasmRtm(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Rn ( ____________xxxx(op) );

    str << Ins{I} << str.tab << src;
    str << Av{I, M, S};
}

void
Moira::dasmRtr(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    str << Ins{I};
}

void
Moira::dasmRts(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    str << Ins{I};
}

void
Moira::dasmSccRg(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto src = Dn ( _____________xxx(op) );

    str << Ins{I} << str.tab << src;
}

void
Moira::dasmSccEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto src = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << str.tab << src;
}

void
Moira::dasmStop(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto src = dasmIncRead(addr, S);

    str << Ins{I} << str.tab << Ims(src, S);
}

void
Moira::dasmNegRg(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, Size S) const
{
    auto dst = Dn ( _____________xxx(op) );

    str << Ins{I} << Sz{S} << str.tab << dst;
}

void
Moira::dasmNegEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << dst;
}

void
Moira::dasmSwap(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    Dn reg = Dn ( _____________xxx(op) );

    str << Ins{I} << str.tab << reg;
}

void
Moira::dasmTasRg(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto dst = Dn ( _____________xxx(op) );

    str << Ins{I} << str.tab << dst;
}

void
Moira::dasmTasEa(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto dst = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << str.tab << dst;
}

void
Moira::dasmTrap(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto nr = Imu ( ____________xxxx(op) );

    str << Ins{I} << str.tab << nr;
}

void
Moira::dasmTrapv(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    str << Ins{I};
}

void
Moira::dasmTrapcc(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            switch (S) {

                case Byte:

                    str << Ins{I};
                    break;

                case Word:
                case Long:

                    auto ext = dasmIncRead(addr, S);
                    str << Ins{I} << Sz{S} << str.tab << Ims(ext, S);
                    break;
            }
            break;

        default:

            switch (S) {

                case Byte:

                    str << Ins{I} << str.tab;
                    break;

                case Word:
                case Long:

                    auto ext = dasmIncRead(addr, S);
                    str << Ins{I} << str.tab << Imu(ext);
                    break;
            }
    }

    str << Av{I, M, S};
}

void
Moira::dasmTst(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ea = Op(M, S, _____________xxx(op), addr);

    str << Ins{I} << Sz{S} << str.tab << ea;
    str << Av{I, M, S};
}

void
Moira::dasmUnlk(StrWriter &str, u32 &addr, u16 op, Instr I, [[maybe_unused]] Mode M, [[maybe_unused]] Size S) const
{
    auto reg = An ( _____________xxx(op) );

    str << Ins{I} << str.tab << reg;
}

void
Moira::dasmUnpkDn(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = dasmIncRead(addr, Word);
    auto rx = Op(M, S, _____________xxx(op), addr);
    auto ry = Op(M, S, ____xxx_________(op), addr);

    switch (str.style.syntax) {

        case Syntax::GNU:
        case Syntax::GNU_MIT:

            str << Ins{I} << str.tab << rx << Sep{} << ry << Sep{} << Ims(ext, S);
            break;

        default:

            str << Ins{I} << str.tab << rx << Sep{} << ry << Sep{} << Imu(ext);
            str << Av{I, M, S};
    }
}

void
Moira::dasmUnpkPd(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    dasmUnpkDn(str, addr, op, I, M, S);
}

void
Moira::dasmCinv(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto reg   = _____________xxx(op);
    auto scope = ___________xx___(op);
    auto cache = ________xx______(op);

    const char *cname[4] = { "nc", "dc", "ic", "bc" };

    if (scope == 0) {

        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    switch (scope) {

        case 0b00:

            str << Ins{I} << str.tab << "(illegal scope)";
            break;

        case 0b01:

            str << Ins{I} << "l" << str.tab << cname[cache] << Sep{} << Op(M, Long, reg, addr);
            break;

        case 0b10:

            str << Ins{I} << "p" << str.tab << cname[cache] << Sep{} << Op(M, Long, reg, addr);
            break;

        case 0b11:

            str << Ins{I} << "a" << str.tab << cname[cache];
            break;
    }

    str << Av{I, M, S};
}

void
Moira::dasmCpush(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    dasmCinv(str, addr, op, I, M, S);
}

void
Moira::dasmMove16PiPi(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);
    auto ax  = _____________xxx(op);
    auto ay  = _xxx____________(ext);

    if ((ext & 0x8FFF) != 0x8000) {

        addr = old;
        dasmIllegal(str, addr, op, I, M, S);
        return;
    }

    str << Ins{I} << str.tab << Op(Mode::PI, Word, ax, addr) << Sep{} << Op(Mode::PI, Word, ay, addr);
    str << Av{I, M, S};
}

void
Moira::dasmMove16PiAl(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = dasmIncRead(addr, Long);
    auto ay  = _____________xxx(op);

    str << Ins{I} << str.tab << Op(Mode::PI, Word, ay, addr) << Sep{} << UInt(ext);
    str << Av{I, M, S};
}

void
Moira::dasmMove16AlPi(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = dasmIncRead(addr, Long);
    auto ay  = _____________xxx(op);

    str << Ins{I} << str.tab << UInt(ext) << Sep{} << Op(Mode::PI, Word, ay, addr);
    str << Av{I, M, S};
}

void
Moira::dasmMove16AiAl(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = dasmIncRead(addr, Long);
    auto ay  = _____________xxx(op);

    str << Ins{I} << str.tab << Op(Mode::AI, Word, ay, addr) << Sep{} << UInt(ext);
    str << Av{I, M, S};
}

void
Moira::dasmMove16AlAi(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = dasmIncRead(addr, Long);
    auto ay  = _____________xxx(op);

    str << Ins{I} << str.tab << UInt(ext) << Sep{} << Op(Mode::AI, Word, ay, addr);
    str << Av{I, M, S};
}

#include "MoiraDasmMMU_cpp.h"
#include "MoiraDasmFPU_cpp.h"
