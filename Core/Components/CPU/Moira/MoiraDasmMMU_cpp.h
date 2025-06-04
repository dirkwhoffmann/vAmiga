// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Instr I, Mode M, Size S> void
Moira::dasmPGen(StrWriter &str, u32 &addr, u16 op) const
{
    auto ext = dasmIncRead<Word>(addr);
    addr -= 2;

    // PLOAD: 0010 00x0 000x xxxx
    if ((ext & 0xFDE0) == 0x2000) {

        dasmPLoad<Instr::PLOAD, M, Long>(str, addr, op);
        return;
    }

    // PFLUSHA: 0010 010x xxxx xxxx
    if ((ext & 0xFE00) == 0x2400) {

        dasmPFlusha<Instr::PFLUSHA, M, Long>(str, addr, op);
        return;
    }

    // PFLUSH: 001x xx0x xxxx xxxx
    if ((ext & 0xE200) == 0x2000) {

        dasmPFlush<Instr::PFLUSH, M, Long>(str, addr, op);
        return;
    }

    // PTEST: 100x xxxx xxxx xxxx
    if ((ext & 0xE000) == 0x8000) {

        dasmPTest<Instr::PTEST, M, Long>(str, addr, op);
        return;
    }

    // PMOVE: 010x xxxx 0000 0000 || 0110 00x0 0000 0000 || 000x xxxx 0000 0000
    if ((ext & 0xE0FF) == 0x4000 || (ext & 0xFDFF) == 0x6000 || (ext & 0xE0FF) == 0x0000) {

        dasmPMove<Instr::PMOVE, M, S>(str, addr, op);
        return;
    }

    dasmIllegal<I, M, S>(str, addr, op);
}

template <Instr I, Mode M, Size S> void
Moira::dasmPFlush(StrWriter &str, u32 &addr, u16 op) const
{
    auto old  = addr;
    auto ext  = dasmIncRead<Word>(addr);
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
            dasmIllegal<I, M, S>(str, addr, op);
            return;
        }
    }

    str << Ins<I>{} << str.tab;
    str << Fc{fc} << Sep{} << Imu{mask};
    if (mode == 0b110) str << Sep{} << Op<M>(reg, addr);
}

template <Instr I, Mode M, Size S> void
Moira::dasmPFlusha(StrWriter &str, u32 &addr, u16 op) const
{
    auto old = addr;
    auto ext = dasmIncRead<Word>(addr);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtMMU(I, M, op, ext)) {

            addr = old;
            dasmIllegal<I, M, S>(str, addr, op);
            return;
        }
    }

    str << Ins<I>{};
}

template <Instr I, Mode M, Size S> void
Moira::dasmPFlush40(StrWriter &str, u32 &addr, u16 op) const
{
    auto reg  = _____________xxx (op);
    auto mode = ___________xx___ (op);

    switch (mode) {

        case 0: str << Ins<Instr::PFLUSHN>{} << str.tab << Op<M>(reg, addr); break;
        case 1: str << Ins<Instr::PFLUSH>{} << str.tab << Op<M>(reg, addr); break;
        case 2: str << Ins<Instr::PFLUSHAN>{}; break;
        case 3: str << Ins<Instr::PFLUSHA>{}; break;
    }
}

template <Instr I, Mode M, Size S> void
Moira::dasmPLoad(StrWriter &str, u32 &addr, u16 op) const
{
    auto old = addr;
    auto ext = dasmIncRead<Word>(addr);
    auto ea  = Op <M,S> ( _____________xxx(op), addr );

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtMMU(I, M, op, ext)) {

            addr = old;
            dasmIllegal<I, M, S>(str, addr, op);
            return;
        }
    }

    str << Ins<I>{} << ((ext & 0x200) ? "r" : "w") << str.tab;
    str << Fc(ext & 0b11111) << Sep{} << ea;
}

template <Instr I, Mode M, Size S> void
Moira::dasmPMove(StrWriter &str, u32 &addr, u16 op) const
{
    auto old  = addr;
    auto ext  = dasmIncRead<Word>(addr);
    auto reg  = _____________xxx (op);
    auto fmt  = xxx_____________ (ext);
    auto preg = ___xxx__________ (ext);
    auto nr   = ___________xxx__ (ext);

    // Catch illegal extension words
    if (str.style.syntax == Syntax::GNU || str.style.syntax == Syntax::GNU_MIT) {

        if (!isValidExtMMU(I, M, op, ext)) {

            addr = old;
            dasmIllegal<I, M, S>(str, addr, op);
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

        str << Ins<I>{} << suffix << str.tab;
        if (s == Word) str << Op<M, Word>(reg, addr) << Sep{};
        if (s == Long) str << Op<M, Long>(reg, addr) << Sep{};
        str << prefix << r;
        if (fmt == 3 && preg > 1) str << Int(nr);

    } else {

        str << Ins<I>{} << suffix << str.tab;
        if (fmt == 3 && preg > 1) str << Int(nr);
        str << prefix << r;
        if (s == Word) str << Sep{} << Op<M, Word>(reg, addr);
        if (s == Long) str << Sep{} << Op<M, Long>(reg, addr);
    }
}

template <Instr I, Mode M, Size S> void
Moira::dasmPTest(StrWriter &str, u32 &addr, u16 op) const
{
    auto old = addr;
    auto ext = dasmIncRead<Word>(addr);
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
            dasmIllegal<I, M, S>(str, addr, op);
            return;
        }
    }

    str << Ins<I>{} << (rw ? "r" : "w") << str.tab;
    str << Fc{fc} << Sep{} << Op<M>(reg, addr) << Sep{} << lev;
    if (a) { str << Sep{} << An{an}; }
}

template <Instr I, Mode M, Size S> void
Moira::dasmPTest40(StrWriter &str, u32 &addr, u16 op) const
{
    auto reg = _____________xxx(op);
    auto rw  = __________x_____(op);

    str << Ins<I>{} << (rw ? "r" : "w") << str.tab;
    str << Op<M>(reg, addr);
}
