// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

void
Moira::dasmPGen(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto ext = dasmIncRead(addr, Word);
    addr -= 2;

    // PLOAD: 0010 00x0 000x xxxx
    if ((ext & 0xFDE0) == 0x2000) {

        dasmPLoad(str, addr, op, Instr::PLOAD, M, Long);
        return;
    }

    // PFLUSHA: 0010 010x xxxx xxxx
    if ((ext & 0xFE00) == 0x2400) {

        dasmPFlusha(str, addr, op, Instr::PFLUSHA, M, Long);
        return;
    }

    // PFLUSH: 001x xx0x xxxx xxxx
    if ((ext & 0xE200) == 0x2000) {

        dasmPFlush(str, addr, op, Instr::PFLUSH, M, Long);
        return;
    }

    // PTEST: 100x xxxx xxxx xxxx
    if ((ext & 0xE000) == 0x8000) {

        dasmPTest(str, addr, op, Instr::PTEST, M, Long);
        return;
    }

    // PMOVE: 010x xxxx 0000 0000 || 0110 00x0 0000 0000 || 000x xxxx 0000 0000
    if ((ext & 0xE0FF) == 0x4000 || (ext & 0xFDFF) == 0x6000 || (ext & 0xE0FF) == 0x0000) {

        dasmPMove(str, addr, op, Instr::PMOVE, M, S);
        return;
    }

    dasmIllegal(str, addr, op, I, M, S);
}

void
Moira::dasmPFlush(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old  = addr;
    auto ext  = dasmIncRead(addr, Word);
    auto reg  = _____________xxx (op);
    auto mode = ___xxx__________ (ext);
    auto mask = _______xxxx_____ (ext);
    auto fc   = ___________xxxxx (ext);

    // Only the MC68851 has four mask bits. The 68030 only has three.
    if (str.style.syntax == Syntax::MOIRA || str.style.syntax == Syntax::MOIRA_MIT) mask &= 0b111;

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtMMU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    str << Ins{I} << str.tab;
    str << Fc{fc} << Sep{} << Imu{mask};
    if (mode == 0b110) str << Sep{} << Op(M, Word, reg, addr);
}

void
Moira::dasmPFlusha(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtMMU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    str << Ins{I};
}

void
Moira::dasmPFlush40(StrWriter &str, u32 &addr, u16 op, [[maybe_unused]] Instr I, Mode M, [[maybe_unused]] Size S) const
{
    auto reg  = _____________xxx (op);
    auto mode = ___________xx___ (op);

    switch (mode) {

        case 0: str << Ins{Instr::PFLUSHN} << str.tab << Op(M, Word, reg, addr); break;
        case 1: str << Ins{Instr::PFLUSH} << str.tab << Op(M, Word, reg, addr); break;
        case 2: str << Ins{Instr::PFLUSHAN}; break;
        case 3: str << Ins{Instr::PFLUSHA}; break;
    }
}

void
Moira::dasmPLoad(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);
    auto ea  = Op(M, S, _____________xxx(op), addr);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtMMU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    str << Ins{I} << ((ext & 0x200) ? "r" : "w") << str.tab;
    str << Fc(ext & 0b11111) << Sep{} << ea;
}

void
Moira::dasmPMove(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old  = addr;
    auto ext  = dasmIncRead(addr, Word);
    auto reg  = _____________xxx (op);
    auto fmt  = xxx_____________ (ext);
    auto preg = ___xxx__________ (ext);
    auto nr   = ___________xxx__ (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtMMU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    const char *prefix = str.style.syntax == Syntax::GNU_MIT || str.style.syntax == Syntax::MOIRA_MIT ? "%" : "";
    const char *suffix = (ext & 0x100) ? "fd" : "";
    const char *r = "";
    Size s = Size(Unsized);

    switch (fmt) {

        case 0:

            switch (preg) {

                case 0b010: r = "tt0";  s = Long; break;
                case 0b011: r = "tt1";  s = Long; break;
            }
            break;

        case 2:

            switch (preg) {

                case 0b000: r = "tc";   s = Long; break;
                case 0b001: r = "drp";  s = Long; break;
                case 0b010: r = "srp";  s = Long; break;
                case 0b011: r = "crp";  s = Long; break;
                case 0b100: r = "cal";  s = Long; break;
                case 0b101: r = "val";  s = Long; break;
                case 0b110: r = "scc";  s = Long; break;
                case 0b111: r = "ac";   s = Long; break;
            }
            break;

        case 3:

            switch (preg) {

                case 0b000: r = "psr";  s = Word; break;
                case 0b001: r = "pcsr"; s = Word; break;
                case 0b100: r = "bad";  s = Word; break;
                case 0b101: r = "bac";  s = Word; break;
            }
            break;
    }

    if (!(ext & 0x200)) {

        str << Ins{I} << suffix << str.tab;
        if (s == Word) str << Op(M, Word, reg, addr) << Sep{};
        if (s == Long) str << Op(M, Long, reg, addr) << Sep{};
        str << prefix << r;
        if (fmt == 3 && preg > 1) str << Int(nr);

    } else {

        str << Ins{I} << suffix << str.tab;
        if (fmt == 3 && preg > 1) str << Int(nr);
        str << prefix << r;
        if (s == Word) str << Sep{} << Op(M, Word, reg, addr);
        if (s == Long) str << Sep{} << Op(M, Long, reg, addr);
    }
}

void
Moira::dasmPTest(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, Size S) const
{
    auto old = addr;
    auto ext = dasmIncRead(addr, Word);
    auto reg = _____________xxx (op);
    auto lev = ___xxx__________ (ext);
    auto rw  = ______x_________ (ext);
    auto a   = _______x________ (ext);
    auto an  = ________xxx_____ (ext);
    auto fc  = ___________xxxxx (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtMMU(I, M, op, ext)) {

            addr = old;
            dasmIllegal(str, addr, op, I, M, S);
            return;
        }
    }

    str << Ins{I} << (rw ? "r" : "w") << str.tab;
    str << Fc{fc} << Sep{} << Op(M, Word, reg, addr) << Sep{} << lev;
    if (a) { str << Sep{} << An{an}; }
}

void
Moira::dasmPTest40(StrWriter &str, u32 &addr, u16 op, Instr I, Mode M, [[maybe_unused]] Size S) const
{
    auto reg = _____________xxx(op);
    auto rw  = __________x_____(op);

    str << Ins{I} << (rw ? "r" : "w") << str.tab;
    str << Op(M, Word, reg, addr);
}
