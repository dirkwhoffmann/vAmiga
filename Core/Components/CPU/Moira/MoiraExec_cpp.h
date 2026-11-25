// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#define AVAILABILITY(core) \
if constexpr ((core) == Core::C68010) { static_assert(C != Core::C68000); } \
if constexpr ((core) == Core::C68020) { static_assert(C != Core::C68000 && C != Core::C68010); } \
if constexpr (C == Core::C68020) budget.penalty = 0; \
if constexpr (MOIRA_WILL_EXECUTE) willExecute(__func__, I, M, S, opcode);

#define FINALIZE \
if constexpr (MOIRA_DID_EXECUTE) didExecute(__func__, I, M, S, opcode);

#define SUPERVISOR_MODE_ONLY \
if (!reg.sr.s) { \
execException<C>(M68kException::PRIVILEGE); \
CYCLES_68000(34) \
CYCLES_68010(38) \
CYCLES_68020(34) \
return; }

#define CLRC ((!isRegMode(M) || S == Long) ? COMMIT_IRD : 0)   // CLR defers the IRD update only if the prefetch is not the last cycle

#define STD_AE_FRAME (\
(M == Mode::PD && S != Long) ? AE_INC_PC : \
(M == Mode::DI)              ? AE_DEC_PC : \
(M == Mode::IX)              ? AE_DEC_PC : \
(M == Mode::DIPC)            ? AE_DEC_PC : \
(M == Mode::IXPC)            ? AE_DEC_PC : 0)

template <Core C, Instr I, Mode M, Size S> void
Moira::execLineA(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    // Check if a software trap is set for this instruction
    if (debugger.swTraps.traps.contains(opcode)) {

        auto &trap = debugger.swTraps.traps[opcode];

        // Restore the original instruction
        reg.pc = reg.pc0;
        queue.irc = trap.instruction;
        prefetch<C>();

        // Inform the delegate
        didReachSoftwareTrap(reg.pc0);
        return;
    }

    execException<C>(M68kException::LINEA);

    CYCLES_68000(34)
    CYCLES_68010(4)
    CYCLES_68020(20)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execLineF(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    execException<C>(M68kException::LINEF);

    CYCLES_68000(34)
    CYCLES_68010(38)
    CYCLES_68020(34)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execIllegal(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    execException<C>(M68kException::ILLEGAL);

    CYCLES_68000(34)
    CYCLES_68010(38)
    CYCLES_68020(20)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execShiftRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    int cnt = readD(src) & 0x3F;
    int cyc = (S == Long ? 4 : 2) + 2 * cnt;

    u32 operand = readD<S>(dst);

    try { prefetchLate<C, POLL>(); } catch (BusError &e) {
        rethrow<C>(e, CCR::N | CCR::Z | CCR::V | CCR::C, shiftPendingCCR(I, S, operand));
    }
    SYNC(cyc);

    writeD<S>(dst, shift<C, I, S>(cnt, operand));

    if constexpr (C == Core::C68000 || C == Core::C68010) {

        CYCLES(4 + cyc);

    } else {

        constexpr int base =
        (I == Instr::ROL || I == Instr::ROR || I == Instr::ASL) ? 8 :
        (I == Instr::ROXL || I == Instr::ROXR) ? 12 : 6;

        CYCLES(base + cnt);
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execShiftIm(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);
    int cnt = src ? src : 8;
    int cyc = (S == Long ? 4 : 2) + 2 * cnt;

    u32 operand = readD<S>(dst);

    try { prefetchLate<C, POLL>(); } catch (BusError &e) {
        rethrow<C>(e, CCR::N | CCR::Z | CCR::V | CCR::C, shiftPendingCCR(I, S, operand));
    }
    SYNC(cyc);

    writeD<S>(dst, shift<C, I, S>(cnt, operand));

    if constexpr (C == Core::C68000 || C == Core::C68010) {

        CYCLES(4 + cyc);

    } else {

        constexpr int cycles =
        (I == Instr::LSL || I == Instr::LSR) ? 4 :
        (I == Instr::ROL || I == Instr::ROR || I == Instr::ASL) ? 8 :
        (I == Instr::ROXL || I == Instr::ROXR) ? 12 : 6;

        CYCLES(cycles);
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execShiftEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);

    u32 ea, data;
    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    u32 result = shift<C, I, S>(1, data);
    looping<I>() ? noPrefetch<C>(4) : fetchIrc<C, POLL>();
    writeM<C, M, S, COMMIT_IRD>(ea, result);

    if constexpr (I == Instr::ROR || I == Instr::ROL) {

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_AI   ( 0,  0,  0,       12, 12, 11,        0,  0,  0)
        CYCLES_PI   ( 0,  0,  0,       12, 12, 11,        0,  0,  0)
        CYCLES_PD   ( 0,  0,  0,       14, 14, 12,        0,  0,  0)
        CYCLES_DI   ( 0,  0,  0,       16, 16, 12,        0,  0,  0)
        CYCLES_IX   ( 0,  0,  0,       18, 18, 14,        0,  0,  0)
        CYCLES_AW   ( 0,  0,  0,       16, 16, 11,        0,  0,  0)
        CYCLES_AL   ( 0,  0,  0,       20, 20, 11,        0,  0,  0)

    } else if constexpr (I == Instr::ASL) {

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_AI   ( 0,  0,  0,       12, 12, 10,        0,  0,  0)
        CYCLES_PI   ( 0,  0,  0,       12, 12, 10,        0,  0,  0)
        CYCLES_PD   ( 0,  0,  0,       14, 14, 11,        0,  0,  0)
        CYCLES_DI   ( 0,  0,  0,       16, 16, 11,        0,  0,  0)
        CYCLES_IX   ( 0,  0,  0,       18, 18, 13,        0,  0,  0)
        CYCLES_AW   ( 0,  0,  0,       16, 16, 10,        0,  0,  0)
        CYCLES_AL   ( 0,  0,  0,       20, 20, 10,        0,  0,  0)

    } else {

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_AI   ( 0,  0,  0,       12, 12,  9,        0,  0,  0)
        CYCLES_PI   ( 0,  0,  0,       12, 12,  9,        0,  0,  0)
        CYCLES_PD   ( 0,  0,  0,       14, 14, 10,        0,  0,  0)
        CYCLES_DI   ( 0,  0,  0,       16, 16, 10,        0,  0,  0)
        CYCLES_IX   ( 0,  0,  0,       18, 18, 12,        0,  0,  0)
        CYCLES_AW   ( 0,  0,  0,       16, 16,  9,        0,  0,  0)
        CYCLES_AL   ( 0,  0,  0,       20, 20,  9,        0,  0,  0)
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAbcdRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 result = bcd<C, I, Byte>(readD<Byte>(src), readD<Byte>(dst));

    prefetchLate<C, POLL>();
    SYNC(2);
    writeD<Byte>(dst, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 6,  6,  4,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAbcdEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea1, ea2, data1, data2;
    readOp<C, M, S, AE_INC_PC>(src, &ea1, &data1);
    POLL_IPL;
    readOp<C, M, S, IMPL_DEC | AE_INC_PC>(dst, &ea2, &data2);

    u32 result = bcd<C, I, Byte>(data1, data2);
    looping<I>() ? noPrefetch<C>(4) : fetchIrc<C>();
    writeM<C, M, Byte, COMMIT_IRD>(ea2, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_PD   (18, 18, 16,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddEaRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data, result;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    if constexpr (C == Core::C68000 && S == Long && isMemMode(M)) {

        result = addsub<C, I, S>(data, readD<S>(dst));
        try { prefetchLate<C, POLL, true>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, result);
        }
        SYNC(2);
        writeD<S>(dst, result);

    } else if constexpr (C == Core::C68000 && S == Long) {

        try { prefetchLate<C, POLL, true>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, addsub<C, I, Word>(data, readD<Word>(dst)));
        }
        result = addsub<C, I, S>(data, readD<S>(dst));
        SYNC(4);
        writeD<S>(dst, result);

    } else if constexpr (C == Core::C68000) {

        result = addsub<C, I, S>(data, readD<S>(dst));

        try { prefetch<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, S>(e, dst, result);
        }
        writeD<S>(dst, result);

    } else {

        result = addsub<C, I, S>(data, readD<S>(dst));
        writeD<S>(dst, result);
        POLL_IPL;
        if constexpr (S == Long) SYNC(2);
        looping<I>() ? noPrefetch<C>(6) : prefetch<C>();
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        8,  6,  2)
    CYCLES_AN   ( 0,  0,  0,        4,  4,  2,        8,  6,  2)
    CYCLES_AI   ( 8,  8,  6,        8,  8,  6,       14, 14,  6)
    CYCLES_PI   ( 8,  8,  6,        8,  8,  6,       14, 14,  6)
    CYCLES_PD   (10, 10,  7,       10, 10,  7,       16, 16,  7)
    CYCLES_DI   (12, 12,  7,       12, 12,  7,       18, 18,  7)
    CYCLES_IX   (14, 14,  9,       14, 14,  9,       20, 20,  9)
    CYCLES_AW   (12, 12,  6,       12, 12,  6,       18, 18,  6)
    CYCLES_AL   (16, 16,  6,       16, 16,  6,       22, 22,  6)
    CYCLES_DIPC (12, 12,  7,       12, 12,  7,       18, 18,  7)
    CYCLES_IXPC (14, 14,  9,       14, 14,  9,       20, 20,  9)
    CYCLES_IM   ( 8,  8,  4,        8,  8,  4,       16, 14,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddRgEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data, result;

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);

    result = addsub<C, I, S>(readD<S>(src), data);

    // If the prefetch of a long operation fails, the flags of the low word operation show up
    try { looping<I>() ? noPrefetch<C>(2) : fetchIrc<C, POLL>(); } catch (BusError &e) {
        if constexpr (S == Long) addsub<C, I, Word>(readD<Word>(src), data);
        rethrow<C>(e);
    }

    writeM<C, M, S, REVERSE | COMMIT_IRD>(ea, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PD   (14, 14,  9,       14, 14,  9,       22, 22,  9)
    CYCLES_DI   (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_IX   (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_AW   (16, 16,  8,       16, 16,  8,       24, 24,  8)
    CYCLES_AL   (20, 20,  8,       20, 20,  8,       28, 28,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAdda(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data, result;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);
    data = SEXT<S>(data);

    result = (I == Instr::ADDA || I == Instr::ADDA_LOOP) ? U32_ADD(readA(dst), data) : U32_SUB(readA(dst), data);

    if constexpr (C == Core::C68000) {

        try { prefetchLate<C, POLL>(); } catch (BusError &e) {
            rethrowA<C, Word>(e, dst, result);
        }
        SYNC(2);
        if constexpr (S == Word || isRegMode(M) || isImmMode(M)) SYNC(2);
        writeA(dst, result);

    } else {

        writeA(dst, result);
        SYNC(2);
        POLL_IPL;
        if constexpr (S == Word || isRegMode(M) || isImmMode(M)) SYNC(2);

        constexpr int loopDelay =
        M == Mode::DN ? (S == Long ? 6 : 0) :
        (M == Mode::AI || M == Mode::PI || M == Mode::PD) ? (S == Long ? 6 : 4) : 0;

        looping<I>() ? noPrefetch<C>(loopDelay) : prefetch<C>();
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        8,  8,  2,        8,  6,  2)
    CYCLES_AN   ( 0,  0,  0,        8,  8,  2,        8,  6,  2)
    CYCLES_AI   ( 0,  0,  0,       12, 12,  6,       14, 14,  6)
    CYCLES_PI   ( 0,  0,  0,       12, 12,  6,       14, 14,  6)
    CYCLES_PD   ( 0,  0,  0,       14, 14,  7,       16, 16,  7)
    CYCLES_DI   ( 0,  0,  0,       16, 16,  7,       18, 18,  7)
    CYCLES_IX   ( 0,  0,  0,       18, 18,  9,       20, 20,  9)
    CYCLES_AW   ( 0,  0,  0,       16, 16,  6,       18, 18,  6)
    CYCLES_AL   ( 0,  0,  0,       20, 20,  6,       22, 22,  6)
    CYCLES_DIPC ( 0,  0,  0,       16, 16,  7,       18, 18,  7)
    CYCLES_IXPC ( 0,  0,  0,       18, 18,  9,       20, 20,  9)
    CYCLES_IM   ( 0,  0,  0,       12, 12,  4,       16, 14,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddiRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 src = readI<C, S>();
    int dst = _____________xxx(opcode);

    u32 ea, data, result;
    readOp<C, M, S>(dst, &ea, &data);

    if constexpr (S == Long) {

        try { prefetchLate<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, addsub<C, I, Word>(src, data));
        }
        result = addsub<C, I, S>(src, data);
        SYNC_68000(4);
        SYNC_68010(2);

    } else {

        result = addsub<C, I, S>(src, data);
        try { prefetch<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, S>(e, dst, result);
        }
    }
    writeD<S>(dst, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (8,   8,  2,        8,  8,  2,       16, 14,  2)
    CYCLES_AI   (16, 16,  8,       16, 16,  8,       28, 28,  8)
    CYCLES_PI   (16, 16,  8,       16, 16,  8,       28, 28,  8)
    CYCLES_PD   (18, 18,  9,       18, 18,  9,       30, 30,  9)
    CYCLES_DI   (20, 20,  9,       20, 20,  9,       32, 32,  9)
    CYCLES_IX   (22, 22, 11,       22, 22, 11,       34, 34, 11)
    CYCLES_AW   (20, 20,  8,       20, 20,  8,       32, 32,  8)
    CYCLES_AL   (24, 24,  8,       24, 24,  8,       36, 36,  8)
    CYCLES_IM   (22, 22, 11,       22, 22, 11,       34, 34, 11)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddiEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 src = readIFrame<C, S>();
    int dst = _____________xxx(opcode);

    u32 ea, data, result;
    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);
    result = addsub<C, I, S>(src, data);

    try { fetchIrc<C>(); } catch (BusError &e) {
        if constexpr (S == Long) addsub<C, I, Word>(src, data);
        rethrow<C>(e);
    }

    writeOp<C, M, S, POLL | REVERSE | COMMIT_IRD>(dst, ea, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (8,   8,  2,        8,  8,  2,       16, 14,  2)
    CYCLES_AI   (16, 16,  8,       16, 16,  8,       28, 28,  8)
    CYCLES_PI   (16, 16,  8,       16, 16,  8,       28, 28,  8)
    CYCLES_PD   (18, 18,  9,       18, 18,  9,       30, 30,  9)
    CYCLES_DI   (20, 20,  9,       20, 20,  9,       32, 32,  9)
    CYCLES_IX   (22, 22, 11,       22, 22, 11,       34, 34, 11)
    CYCLES_AW   (20, 20,  8,       20, 20,  8,       32, 32,  8)
    CYCLES_AL   (24, 24,  8,       24, 24,  8,       36, 36,  8)
    CYCLES_IM   (22, 22, 11,       22, 22, 11,       34, 34, 11)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddqDn(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    i8  src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    if (src == 0) src = 8;
    u32 result;

    if constexpr (S == Long) {

        try { prefetchLate<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, addsub<C, I, Word>(src, readD<Word>(dst)));
        }
        result = addsub<C, I, S>(src, readD<S>(dst));
        SYNC(4);

    } else {

        result = addsub<C, I, S>(src, readD<S>(dst));
        try { prefetch<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, S>(e, dst, result);
        }
    }
    writeD<S>(dst, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        8,  8,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddqAn(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    i8  src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    if (src == 0) src = 8;
    u32 result = (I == Instr::ADDQ) ? U32_ADD(readA(dst), src) : U32_SUB(readA(dst), src);

    try { prefetchLate<C, POLL>(); } catch (BusError &e) {
        rethrowA<C>(e, dst, result);
    }
    SYNC(4);
    writeA(dst, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AN   ( 0,  0,  0,        8,  4,  2,        8,  8,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddqEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    i8  src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);
    if (src == 0) src = 8;

    u32 ea, data, result;
    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);

    result = addsub<C, I, S>(src, data);

    try { fetchIrc<C, POLL>(); } catch (BusError &e) {
        if constexpr (S == Long) addsub<C, I, Word>(src, data);
        rethrow<C>(e);
    }

    writeOp<C, M, S, REVERSE | COMMIT_IRD>(dst, ea, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PD   (14, 14,  9,       14, 14,  9,       22, 22,  9)
    CYCLES_DI   (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_IX   (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_AW   (16, 16,  8,       16, 16,  8,       24, 24,  8)
    CYCLES_AL   (20, 20,  8,       20, 20,  8,       28, 28,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddxRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 result;

    if constexpr (C == Core::C68000 && S == Long) {

        try { prefetchLate<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, addsub<C, I, Word>(readD<Word>(src), readD<Word>(dst)));
        }
        result = addsub<C, I, S>(readD<S>(src), readD<S>(dst));
        SYNC(4);

    } else if constexpr (C == Core::C68000) {

        result = addsub<C, I, S>(readD<S>(src), readD<S>(dst));
        try { prefetch<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, S>(e, dst, result);
        }

    } else {

        result = addsub<C, I, S>(readD<S>(src), readD<S>(dst));
        POLL_IPL;
        if constexpr (S == Long) SYNC(2);
        prefetch<C>();
    }

    writeD<S>(dst, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        8,  6,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAddxEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea1, ea2, data1, data2, result;

    if constexpr (C == Core::C68000 && S == Long) {

        // The 68000 reads and writes the lower word of a long operand first
        constexpr Flags flags = AE_INC_PC | AE_INC_A | REVERSE | DEC_LATE;

        readOp<C, M, S, flags>(src, &ea1, &data1);

        // If the high word fails, the low words have been added already
        try { readOp<C, M, S, flags|IMPL_DEC>(dst, &ea2, &data2); } catch (BusError &e) {
            if (e.upperWord) addsub<C, I, Word>(data1, readBuffer);
            rethrow<C>(e);
        }

        u8 ccr = getCCR();
        result = addsub<C, I, S>(data1, data2);

        // If the low word fails, the stacked flags are those of the low word addition
        try { writeM<C, M, Word, POLL | AE_INC_PC>(ea2 + 2, READ<Word>(result)); } catch (BusError &e) {
            setCCR(ccr);
            addsub<C, I, Word>(data1, data2);
            rethrow<C>(e);
        }

        looping<I>() ? noPrefetch<C>() : fetchIrc<C>();
        writeM<C, M, Word, COMMIT_IRD>(ea2, READ<Word, 1>(result));

    } else if constexpr (C == Core::C68010 && S == Long) {

        // The 68010 stacks the PC of the executing instruction and has undone the decrement already
        readOp<C, M, S, AE_INC_A>(src, &ea1, &data1);
        readOp<C, M, S, AE_INC_A|IMPL_DEC>(dst, &ea2, &data2);

        result = addsub<C, I, S>(data1, data2);

        writeM<C, M, S, REVERSE>(ea2, result);
        looping<I>() ? noPrefetch<C>() : prefetchLate<C, POLL>();

    } else if constexpr (C == Core::C68020 && S == Long) {

        readOp<C, M, S>(src, &ea1, &data1);
        readOp<C, M, S, IMPL_DEC>(dst, &ea2, &data2);

        result = addsub<C, I, S>(data1, data2);

        looping<I>() ? noPrefetch<C>() : fetchIrc<C>();
        writeM<C, M, S, COMMIT_IRD>(ea2, result);

    } else {

        // The 68010 stacks the PC of the executing instruction
        constexpr Flags flags = C == Core::C68010 ? 0 : AE_INC_PC;

        readOp<C, M, S, flags>(src, &ea1, &data1);
        POLL_IPL;
        readOp<C, M, S, flags | IMPL_DEC> (dst, &ea2, &data2);

        result = addsub<C, I, S>(data1, data2);

        looping<I>() ? noPrefetch<C>(2) : fetchIrc<C>();
        writeM<C, M, S, COMMIT_IRD>(ea2, result);
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_PD   (18, 18, 12,       18, 18, 12,       30, 30, 12)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAndEaRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea, data;
    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    u32 result;

    if constexpr (C == Core::C68000 && S == Long && isMemMode(M)) {

        // A failing prefetch shows the flags of the long operation and the low word of the result
        result = logic<C, I, S>(data, readD<S>(dst));
        try { looping<I>() ? noPrefetch<C>() : prefetchLate<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, result);
        }
        SYNC(2);
        writeD<S>(dst, result);

    } else if constexpr (C == Core::C68000 && S == Long) {

        // A failing prefetch shows the flags and the result of the low word operation
        try { looping<I>() ? noPrefetch<C>() : prefetchLate<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, logic<C, I, Word>(data, readD<Word>(dst)));
        }
        result = logic<C, I, S>(data, readD<S>(dst));
        SYNC(4);
        writeD<S>(dst, result);

    } else if constexpr (C == Core::C68000) {

        result = logic<C, I, S>(data, readD<S>(dst));

        try { looping<I>() ? noPrefetch<C>() : prefetch<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, S>(e, dst, result);
        }
        writeD<S>(dst, result);

    } else {

        result = logic<C, I, S>(data, readD<S>(dst));

        writeD<S>(dst, result);
        POLL_IPL;
        if constexpr (S == Long) SYNC(2);
        looping<I>() ? noPrefetch<C>(6) : prefetch<C>();
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        8,  6,  2)
    CYCLES_AI   ( 8,  8,  6,        8,  8,  6,       14, 14,  6)
    CYCLES_PI   ( 8,  8,  6,        8,  8,  6,       14, 14,  6)
    CYCLES_PD   (10, 10,  7,       10, 10,  7,       16, 16,  7)
    CYCLES_DI   (12, 12,  7,       12, 12,  7,       18, 18,  7)
    CYCLES_IX   (14, 14,  9,       14, 14,  9,       20, 20,  9)
    CYCLES_AW   (12, 12,  6,       12, 12,  6,       18, 18,  6)
    CYCLES_AL   (16, 16,  6,       16, 16,  6,       22, 22,  6)
    CYCLES_DIPC (12, 12,  7,       12, 12,  7,       18, 18,  7)
    CYCLES_IXPC (14, 14,  9,       14, 14,  9,       20, 20,  9)
    CYCLES_IM   ( 8,  8,  4,        8,  8,  4,       16, 14,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAndRgRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    u32 result;

    if constexpr (S == Long) {

        try { prefetchLate<C, POLL, true>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, logic<C, I, Word>(readD<Word>(src), readD<Word>(dst)));
        }
        result = logic<C, I, S>(readD<S>(src), readD<S>(dst));
        SYNC_68000(4);
        SYNC_68010(2);

    } else {

        result = logic<C, I, S>(readD<S>(src), readD<S>(dst));
        try { prefetch<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, S>(e, dst, result);
        }
    }

    writeD<S>(dst, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        8,  6,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAndRgEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    u32 ea, data, result;
    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);

    result = logic<C, I, S>(readD<S>(src), data);

    try { looping<I>() ? noPrefetch<C>(2) : fetchIrc<C, POLL>(); } catch (BusError &e) {
        
        if constexpr (S == Long) logic<C, I, Word>(readD<Word>(src), data);
        rethrow<C>(e);
    }
    
    writeOp<C, M, S, REVERSE | COMMIT_IRD>(dst, ea, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PD   (14, 14,  9,       14, 14,  9,       22, 22,  9)
    CYCLES_DI   (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_IX   (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_AW   (16, 16,  8,       16, 16,  8,       24, 24,  8)
    CYCLES_AL   (20, 20,  8,       20, 20,  8,       28, 28,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAndiRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 src = readIFrame<C, S>();
    int dst = _____________xxx(opcode);

    u32 result;
    if constexpr (S == Long) {

        try { prefetchLate<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, Word>(e, dst, logic<C, I, Word>(src, readD<Word>(dst)));
        }
        result = logic<C, I, S>(src, readD<S>(dst));
        SYNC_68000(4);
        SYNC_68010(2);

    } else {

        result = logic<C, I, S>(src, readD<S>(dst));

        try { prefetch<C, POLL>(); } catch (BusError &e) {
            rethrowD<C, S>(e, dst, result);
        }
    }
    writeD<S>(dst, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 8,  8,  2,        8,  8,  2,       16, 14,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAndiEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data, result;

    u32 src = readIFrame<C, S>();
    int dst = _____________xxx(opcode);

    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);

    result = logic<C, I, S>(src, data);

    try { fetchIrc<C, POLL>(); } catch (BusError &e) {
        if constexpr (S == Long) logic<C, I, Word>(src, data);
        rethrow<C>(e);
    }

    writeOp<C, M, S, REVERSE | COMMIT_IRD>(dst, ea, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (16, 16,  8,       16, 16,  8,       28, 28,  8)
    CYCLES_PI   (16, 16,  8,       16, 16,  8,       28, 28,  8)
    CYCLES_PD   (18, 18,  9,       18, 18,  9,       30, 30,  9)
    CYCLES_DI   (20, 20,  9,       20, 20,  9,       32, 32,  9)
    CYCLES_IX   (22, 22, 11,       22, 22, 11,       34, 34, 11)
    CYCLES_AW   (20, 20,  8,       20, 20,  8,       32, 32,  8)
    CYCLES_AL   (24, 24,  8,       24, 24,  8,       36, 36,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAndiccr(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    changeOfFlow<C>();

    if constexpr (C == Core::C68000) {

        u32 src = readI<C, S>();
        u8  dst = getCCR();
        setCCR(u8(logic<C, I, S>(src, dst)));
        SYNC(8);
        (void)read<C, AddrSpace::PROG, Word>(reg.pc);

    } else {

        SYNC(8);
        u32 src = readI<C, S>();
        u8  dst = getCCR();
        setCCR(u8(logic<C, I, S>(src, dst)));
    }

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IM   (20, 16, 12,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execAndisr(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    SUPERVISOR_MODE_ONLY

    changeOfFlow<C>();

    u32 src = readI<C, S>();
    u16 dst = getSR();

    SYNC_68000(8);
    SYNC_68010(4);

    u32 result = logic<C, I, S>(src, dst);
    setSR((u16)result);

    if constexpr (C == Core::C68000) {
        (void)read<C, AddrSpace::PROG, Word>(reg.pc);
    } else {
        (void)read<C, AddrSpace::DATA, Word>(reg.pc + 2);
    }

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IM   ( 0,  0,  0,       20, 16, 12,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBra(u16 opcode)
{
    AVAILABILITY(S == Long ? Core::C68020 : Core::C68000)

    u32 oldpc = reg.pc;
    u32 disp = S == Byte ? (u8)opcode : queue.irc;

    SYNC(2);

    if constexpr (S == Long) {

        readExt<C>();
        disp = disp << 16 | queue.irc;
    }

    // Compute new PC
    u32 newpc = U32_ADD(oldpc, SEXT<S>(disp));
    checkEvenPC<C, AE_FETCH>(newpc);

    fullPrefetchAt<C, POLL>(newpc);

    changeOfFlow<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   (10, 10, 10,       10, 10, 10,       10, 10, 10)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBcc(u16 opcode)
{
    AVAILABILITY(S == Long ? Core::C68020 : Core::C68000)

    u32 oldpc = reg.pc;

    SYNC(2);

    if (cond<I>()) {

        u32 disp = S == Byte ? (u8)opcode : queue.irc;

        if constexpr (S == Long) {

            readExt<C>();
            disp = disp << 16 | queue.irc;
        }

        // Compute new PC
        u32 newpc = U32_ADD(oldpc, SEXT<S>(disp));
        checkEvenPC<C, AE_FETCH>(newpc);

        // Take branch
        fullPrefetchAt<C, POLL>(newpc);
        changeOfFlow<C>();

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_IP   (10, 10,  6,       10, 10,  6,        0,  0,  6)

    } else {

        // Fall through to next instruction
        if constexpr (C == Core::C68000) SYNC(2);
        if constexpr (S == Word) readExtLast<C>();
        if constexpr (S == Long) { readExt<C>(); readExtLast<C>(); }
        prefetch<C, POLL>();

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_IP   ( 8,  6,  4,       12, 10,  6,        0,  0,  6)
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBitDxDy(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    u8 b = readD(src) & 0b11111;
    [[maybe_unused]] auto c = cyclesBit<C, I>(b);

    u32 data = readD(dst);

    try { prefetchLate<C, POLL>(); } catch (BusError &e) {
        
        // MAME updates the flags after the prefetch
        if constexpr (C != Core::C68000 || compat == Compatibility::UAE) bit<C, I>(data, b);
        rethrow<C>(e);
    }
    data = bit<C, I>(data, b);

    SYNC(c);
    if constexpr (I != Instr::BTST) writeD(dst, data);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (4+c,4+c, 4,        0,  0,  0,        0,  0,  0)
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBitDxEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    u8 b = readD(src) & 0b111;

    u32 ea, data;

    if constexpr (C == Core::C68000) {

        readOp<C, M, Byte, STD_AE_FRAME>(dst, &ea, &data);

        try { fetchIrc<C, POLL, !(I == Instr::BTST && M != Mode::IM)>(); } catch (BusError &e) {

            // MAME updates the flags after the prefetch, except for BTST with a memory operand. UAE updates them before.
            constexpr bool before = compat == Compatibility::UAE || (I == Instr::BTST && !isRegMode(M) && !isImmMode(M));
            if constexpr (before) bit<C, I>(data, b);
            rethrow<C>(e);
        }
        data = bit<C, I>(data, b);

        // MAME and UAE idle after the prefetch
        if constexpr (I == Instr::BTST && M == Mode::IM) SYNC(2);

    } else {

        readOp<C, M, Byte>(dst, &ea, &data);
        if constexpr (I == Instr::BCLR && C == Core::C68010) SYNC(2);

        // The flags are updated before the prefetch
        try { fetchIrc<C, POLL, !(I == Instr::BTST && M != Mode::IM)>(); } catch (BusError &e) {
            bit<C, I>(data, b);
            rethrow<C>(e);
        }
        data = bit<C, I>(data, b);
    }

    if constexpr (I != Instr::BTST) writeM<C, M, Byte, COMMIT_IRD>(ea, data);
    else if constexpr (M == Mode::IM) commitIrd<C>();

    [[maybe_unused]] auto c = I == Instr::BTST ? 0 : I == Instr::BCLR && C == Core::C68010 ? 6 : 4;

    //             00    10    20        00  10  20        00  10  20
    //             .b    .b    .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   ( 8+c,  8+c,    8,        0,  0,  0,        0,  0,  0)
    CYCLES_PD   (10+c, 10+c,    9,        0,  0,  0,        0,  0,  0)
    CYCLES_PI   ( 8+c,  8+c,    8,        0,  0,  0,        0,  0,  0)
    CYCLES_DI   (12+c, 12+c,    9,        0,  0,  0,        0,  0,  0)
    CYCLES_IX   (14+c, 14+c,   11,        0,  0,  0,        0,  0,  0)
    CYCLES_AW   (12+c, 12+c,    8,        0,  0,  0,        0,  0,  0)
    CYCLES_AL   (16+c, 16+c,    8,        0,  0,  0,        0,  0,  0)
    CYCLES_DIPC (12+c, 12+c,    9,        0,  0,  0,        0,  0,  0)
    CYCLES_IXPC (14+c, 14+c,   11,        0,  0,  0,        0,  0,  0)
    CYCLES_IM   ( 8+c,  8+c,    6,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBitImDy(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u8  src = (u8)readI<C, S>();
    int dst = _____________xxx(opcode);

    src &= 0b11111;
    u32 data = readD(dst);

    try { prefetchLate<C, POLL>(); } catch (BusError &e) {
        
        // MAME updates the flags after the prefetch
        if constexpr (C != Core::C68000 || compat == Compatibility::UAE) bit<C, I>(data, src);
        rethrow<C>(e);
    }
    data = bit<C, I>(data, src);

    [[maybe_unused]] auto c = cyclesBit<C, I>(src);
    SYNC(c);
    if constexpr (I != Instr::BTST) writeD(dst, data);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (8+c,8+c, 4,        0,  0,  0,        0,  0,  0)
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBitImEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u8  src = (u8)readI<C, S>();
    int dst = _____________xxx(opcode);

    src &= 0b111;
    u32 ea, data;
    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);

    try { fetchIrc<C, POLL, (I != Instr::BTST)>(); } catch (BusError &e) {

        if constexpr (C == Core::C68000) {

            // MAME updates the flags after the prefetch, except for BTST with a memory operand. UAE updates them before.
            constexpr bool before = compat == Compatibility::UAE || (I == Instr::BTST && !isRegMode(M) && !isImmMode(M));
            if constexpr (before) bit<C, I>(data, src);

        } else {

            // The flags are updated before the prefetch
            bit<C, I>(data, src);
        }
        rethrow<C>(e);
    }
    data = bit<C, I>(data, src);
    if constexpr (I != Instr::BTST) writeM<C, M, S, COMMIT_IRD>(ea, data);

    [[maybe_unused]] auto c = I == Instr::BTST ? 0 : 4;

    //             00    10    20        00  10  20        00  10  20
    //             .b    .b    .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (12+c, 12+c,    8,        0,  0,  0,        0,  0,  0)
    CYCLES_PD   (14+c, 14+c,    9,        0,  0,  0,        0,  0,  0)
    CYCLES_PI   (12+c, 12+c,    8,        0,  0,  0,        0,  0,  0)
    CYCLES_DI   (16+c, 16+c,    9,        0,  0,  0,        0,  0,  0)
    CYCLES_IX   (18+c, 18+c,   11,        0,  0,  0,        0,  0,  0)
    CYCLES_AW   (16+c, 16+c,    8,        0,  0,  0,        0,  0,  0)
    CYCLES_AL   (20+c, 20+c,    8,        0,  0,  0,        0,  0,  0)
    CYCLES_DIPC (16+c, 16+c,    9,        0,  0,  0,        0,  0,  0)
    CYCLES_IXPC (18+c, 18+c,   11,        0,  0,  0,        0,  0,  0)
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBitFieldDn(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    u16 ext = (u16)readI<C, Word>();

    int dy     = _____________xxx (opcode);
    int dn     = _xxx____________ (ext);
    int of     = _____xxxxx______ (ext);
    int doBit  = ____x___________ (ext);
    int wi     = ___________xxxxx (ext);
    int dwBit  = __________x_____ (ext);

    // If do or dw is set, offset or width are taken from data registers
    int rawOffset = doBit ? reg.d[of & 0b111] : of;
    int rawWidth = dwBit ? reg.d[wi & 0b111] : wi;

    // Crop offset and map width to 32, 1 ... 31
    int offset = rawOffset & 0b11111;
    int width = int((u32(rawWidth) - 1) & 0b11111) + 1;

    // Create the bit mask
    u32 mask = std::rotr(u32(0xFFFFFFFF00000000 >> width), offset);

    u32 data = readD(dy);
    u32 result, insert;

    switch (I) {

        case Instr::BFCHG:
        case Instr::BFCLR:
        case Instr::BFSET:

            result = bitfield<I>(data, offset, width, mask);
            writeD(dy, result);

            //           00  10  20        00  10  20        00  10  20
            //           .b  .b  .b        .w  .w  .w        .l  .l  .l
            CYCLES_DN   ( 0,  0,  0,        0,  0,  0,        0,  0, 12)
            break;

        case Instr::BFEXTS:
        case Instr::BFEXTU:

            data = std::rotl(data, offset);
            result = bitfield<I>(data, offset, width, mask);
            writeD(dn, result);

            //           00  10  20        00  10  20        00  10  20
            //           .b  .b  .b        .w  .w  .w        .l  .l  .l
            CYCLES_DN   ( 0,  0,  0,        0,  0,  0,        0,  0,  8)
            break;

        case Instr::BFFFO:

            data = std::rotl(data, offset);
            result = bitfield<I>(data, rawOffset, width, mask);
            writeD(dn, result);

            //           00  10  20        00  10  20        00  10  20
            //           .b  .b  .b        .w  .w  .w        .l  .l  .l
            CYCLES_DN   ( 0,  0,  0,        0,  0,  0,        0,  0, 18)
            break;

        case Instr::BFINS:

            insert = readD(dn);
            insert = u32(insert << (32 - width));

            setMoveFlags<S>(insert);

            insert = std::rotr((u32)insert, offset);
            writeD(dy, (data & ~mask) | insert);

            //           00  10  20        00  10  20        00  10  20
            //           .b  .b  .b        .w  .w  .w        .l  .l  .l
            CYCLES_DN   ( 0,  0,  0,        0,  0,  0,        0,  0, 10)
            break;

        case Instr::BFTST:

            (void)bitfield<I>(data, offset, width, mask);

            //           00  10  20        00  10  20        00  10  20
            //           .b  .b  .b        .w  .w  .w        .l  .l  .l
            CYCLES_DN   ( 0,  0,  0,        0,  0,  0,        0,  0,  6)
            break;

        default:
            fatalError;
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBitFieldEa(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    u16 ext = (u16)readI<C, Word>();

    int dy     = _____________xxx (opcode);
    int dn     = _xxx____________ (ext);
    int of     = _____xxxxx______ (ext);
    int doBit  = ____x___________ (ext);
    int wi     = ___________xxxxx (ext);
    int dwBit  = __________x_____ (ext);

    // If do or dw is set, offset or width are taken from data registers
    int rawOffset = doBit ? reg.d[of & 0b111] : of;
    int rawWidth = dwBit ? reg.d[wi & 0b111] : wi;

    // Map width to 32, 1 ... 31
    int width = int((u32(rawWidth) - 1) & 0b11111) + 1;

    u32 result, insert;

    // Compute the effective address
    u32 ea = computeEA<C, M, S>(dy);

    // Split the offset into a byte displacement and a bit offset (0 ... 7)
    ea += rawOffset / 8;
    int offset = rawOffset % 8;
    if (offset < 0) {

        offset += 8;
        ea--;
    }

    // Create bit masks
    u64 baseMask = (0xFFFFFFFF00000000 << (32 - width)) >> offset;
    auto mask = u32(baseMask >> 32);
    auto mask8 = u8(baseMask >> 24);

    u32 data = readM<C, M, S>(ea);

    switch (I) {

        case Instr::BFCHG:
        case Instr::BFCLR:
        case Instr::BFSET:

            result = bitfield<I>(data, offset, width, mask);
            writeM<C, M, S>(ea, result);

            if ((width + offset) > 32) {

                data = readM<C, M, Byte>(ea + 4);

                if constexpr (I == Instr::BFCHG) writeM<C, M, Byte>(ea + 4, data ^ mask8);
                if constexpr (I == Instr::BFCLR) writeM<C, M, Byte>(ea + 4, data & ~mask8);
                if constexpr (I == Instr::BFSET) writeM<C, M, Byte>(ea + 4, data | mask8);

                reg.sr.z &= ZERO<Byte>(data & mask8);
            }

            //           00  10  20        00  10  20        00  10  20
            //           .b  .b  .b        .w  .w  .w        .l  .l  .l
            CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        0,  0, 24)
            CYCLES_DI   ( 0,  0,  0,        0,  0,  0,        0,  0, 25)
            CYCLES_IX   ( 0,  0,  0,        0,  0,  0,        0,  0, 27)
            CYCLES_AW   ( 0,  0,  0,        0,  0,  0,        0,  0, 24)
            CYCLES_AL   ( 0,  0,  0,        0,  0,  0,        0,  0, 24)
            break;

        case Instr::BFEXTS:
        case Instr::BFEXTU:
        case Instr::BFFFO:

            // Align the field to bit 31 and pull in the spill byte
            data = CLIP<Long>(data << offset);
            if ((offset + width) > 32) data |= (readM<C, M, Byte>(ea + 4) << offset) >> 8;

            // BFFFO reports the position relative to the unsplit offset
            result = bitfield<I>(data, I == Instr::BFFFO ? rawOffset : offset, width, mask);
            writeD(dn, result);

            if constexpr (I == Instr::BFFFO) {

                //           00  10  20        00  10  20        00  10  20
                //           .b  .b  .b        .w  .w  .w        .l  .l  .l
                CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        0,  0, 32)
                CYCLES_DI   ( 0,  0,  0,        0,  0,  0,        0,  0, 33)
                CYCLES_IX   ( 0,  0,  0,        0,  0,  0,        0,  0, 35)
                CYCLES_AW   ( 0,  0,  0,        0,  0,  0,        0,  0, 32)
                CYCLES_AL   ( 0,  0,  0,        0,  0,  0,        0,  0, 32)
                CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,        0,  0, 33)
                CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,        0,  0, 35)

            } else {

                //           00  10  20        00  10  20        00  10  20
                //           .b  .b  .b        .w  .w  .w        .l  .l  .l
                CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        0,  0, 19)
                CYCLES_DI   ( 0,  0,  0,        0,  0,  0,        0,  0, 20)
                CYCLES_IX   ( 0,  0,  0,        0,  0,  0,        0,  0, 22)
                CYCLES_AW   ( 0,  0,  0,        0,  0,  0,        0,  0, 19)
                CYCLES_AL   ( 0,  0,  0,        0,  0,  0,        0,  0, 19)
                CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,        0,  0, 20)
                CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,        0,  0, 22)
            }
            break;

        case Instr::BFINS:
        {
            insert = readD(dn);
            insert = u32(insert << (32 - width));

            setMoveFlags<S>(insert);

            writeM<C, M, S>(ea, (data & ~mask) | insert >> offset);

            if ((width + offset) > 32) {

                data = readM<C, M, Byte>(ea + 4);
                u8 insert8 = u8(readD(dn) << (8 - ((width + offset) - 32)));
                writeM<C, M, Byte>(ea + 4, (data & ~mask8) | insert8);
                reg.sr.z &= ZERO<Byte>(insert8);
            }

            //           00  10  20        00  10  20        00  10  20
            //           .b  .b  .b        .w  .w  .w        .l  .l  .l
            CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        0,  0, 21)
            CYCLES_DI   ( 0,  0,  0,        0,  0,  0,        0,  0, 22)
            CYCLES_IX   ( 0,  0,  0,        0,  0,  0,        0,  0, 24)
            CYCLES_AW   ( 0,  0,  0,        0,  0,  0,        0,  0, 21)
            CYCLES_AL   ( 0,  0,  0,        0,  0,  0,        0,  0, 21)
            break;
        }
        case Instr::BFTST:

            (void)bitfield<I>(data, offset, width, mask);

            if ((width + offset) > 32) {

                u8 data2 = u8(readM<C, M, Byte>(ea + 4));
                reg.sr.z &= ZERO<Byte>(data2 & mask8);
            }

            //           00  10  20        00  10  20        00  10  20
            //           .b  .b  .b        .w  .w  .w        .l  .l  .l
            CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        0,  0, 17)
            CYCLES_DI   ( 0,  0,  0,        0,  0,  0,        0,  0, 18)
            CYCLES_IX   ( 0,  0,  0,        0,  0,  0,        0,  0, 20)
            CYCLES_AW   ( 0,  0,  0,        0,  0,  0,        0,  0, 17)
            CYCLES_AL   ( 0,  0,  0,        0,  0,  0,        0,  0, 17)
            CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,        0,  0, 18)
            CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,        0,  0, 20)
            break;

        default:
            fatalError;
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBkpt(u16 opcode)
{
    AVAILABILITY(Core::C68010)

    // Acknowledges the breakpoint with a CPU space cycle
    willExecute(CpuSpaceType::BREAKPOINT, opcode & 7);
    didExecute(CpuSpaceType::BREAKPOINT, opcode & 7);

    execException<C>(M68kException::BKPT);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        0, 38, 20)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execBsr(u16 opcode)
{
    AVAILABILITY(S == Long ? Core::C68020 : Core::C68000)

    u32 oldpc = reg.pc;
    u32 disp = S == Byte ? (u8)opcode : queue.irc;

    if constexpr (S == Long) {

        readExt<C>();
        disp = disp << 16 | queue.irc;
    }

    u32 newpc = U32_ADD(oldpc, SEXT<S>(disp));
    u32 retpc = U32_ADD(reg.pc, S == Long || S == Word ? 2 : 0);

    SYNC(2);

    if constexpr (C == Core::C68000) {

        // Check for address errors
        if (misaligned<C>(reg.sp)) {
            reg.sp -= 4;
            throw earlyAddressError<C>(makeFrame<AE_WRITE|AE_DATA>(reg.sp));
        }
        if (misalignedPC<C>(newpc)) {

            // MAME pushes the return address before it detects the misaligned target
            push<C, Long>(retpc);
            reg.pc = newpc;
            throw earlyAddressError<C>(makeFrame<AE_PROG>(newpc));
        }

        // Save return address on stack
        push<C, Long, UAE_DEC_PC>(retpc);

        // Jump to new address
        reg.pc = newpc;
        fullPrefetch<C, POLL>();

    } else if constexpr (C == Core::C68010) {

        // Check for address errors
        if (misaligned<C>(reg.sp)) {

            writeBuffer = 0;
            throw earlyAddressError<C>(makeFrame<AE_WRITE|AE_DATA>(newpc));
        }
        checkEvenPC<C, AE_FETCH>(newpc);

        // Save return address on stack
        push<C, Long, POLL>(retpc);

        // Jump to new address
        reg.pc = newpc;
        fullPrefetch<C>();

    } else if constexpr (C == Core::C68020) {

        // Check for address error
        if (misalignedPC<C>(newpc)) {

            // The 68020 reserves the space for the return address (UAE does not write it)
            reg.sp -= 4;
            throw earlyAddressError<C>(makeFrame<AE_FETCH>(newpc));
        }

        // Save return address on stack
        push<C, Long, POLL>(retpc);

        // Jump to new address
        reg.pc = newpc;
        fullPrefetch<C>();
    }

    changeOfFlow<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   (18, 18,  7,       18, 18,  7,       18, 18,  7)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCallm(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    int dst = _____________xxx(opcode);
    u32 ea, data;

    readOp<C, M, Byte>(dst, &ea, &data);
    readExt<C>();
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        0,  0, 64)
    CYCLES_DI   ( 0,  0,  0,        0,  0,  0,        0,  0, 65)
    CYCLES_IX   ( 0,  0,  0,        0,  0,  0,        0,  0, 67)
    CYCLES_AW   ( 0,  0,  0,        0,  0,  0,        0,  0, 64)
    CYCLES_AL   ( 0,  0,  0,        0,  0,  0,        0,  0, 64)
    CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,        0,  0, 65)
    CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,        0,  0, 67)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCas(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    int dst = _____________xxx(opcode);
    int dc  = _____________xxx(queue.irc);
    int du  = _______xxx______(queue.irc);

    u32 ea, data;

    readExt<C>();
    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);

    auto compare = readD(dc);

    // Set flags
    cmp<C, S>(compare, data);

    if (!reg.sr.z) {

        writeD(dc, CLEAR<S>(compare) | data);

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_AI   ( 0,  0, 16,        0,  0, 16,        0,  0, 16)
        CYCLES_PI   ( 0,  0, 16,        0,  0, 16,        0,  0, 16)
        CYCLES_PD   ( 0,  0, 17,        0,  0, 17,        0,  0, 17)
        CYCLES_DI   ( 0,  0, 17,        0,  0, 17,        0,  0, 17)
        CYCLES_IX   ( 0,  0, 19,        0,  0, 19,        0,  0, 19)
        CYCLES_AW   ( 0,  0, 16,        0,  0, 16,        0,  0, 16)
        CYCLES_AL   ( 0,  0, 16,        0,  0, 16,        0,  0, 16)

    } else {

        writeM<C, M, S>(ea, readD<S>(du));

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_AI   ( 0,  0, 19,        0,  0, 19,        0,  0, 19)
        CYCLES_PI   ( 0,  0, 19,        0,  0, 19,        0,  0, 19)
        CYCLES_PD   ( 0,  0, 20,        0,  0, 20,        0,  0, 20)
        CYCLES_DI   ( 0,  0, 20,        0,  0, 20,        0,  0, 20)
        CYCLES_IX   ( 0,  0, 22,        0,  0, 22,        0,  0, 22)
        CYCLES_AW   ( 0,  0, 19,        0,  0, 19,        0,  0, 19)
        CYCLES_AL   ( 0,  0, 19,        0,  0, 19,        0,  0, 19)
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCas2(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    int dc1 = _____________xxx(queue.irc);
    int du1 = _______xxx______(queue.irc);
    int rn1 = xxxx____________(queue.irc);

    readExt<C>();

    int dc2 = _____________xxx(queue.irc);
    int du2 = _______xxx______(queue.irc);
    int rn2 = xxxx____________(queue.irc);

    readExt<C>();

    u32 ea1 = reg.r[rn1];
    u32 ea2 = reg.r[rn2];
    u32 data1 = readM<C, M, S>(ea1);
    u32 data2 = readM<C, M, S>(ea2);

    auto compare1 = readD(dc1);
    auto compare2 = readD(dc2);

    // Set flags (the second comparison is only performed if the first one matches)
    cmp<C, S>(compare1, data1);
    
    if (reg.sr.z) {
        
        cmp<C, S>(compare2, data2);
    }
    if (reg.sr.z) {

        writeM<C, M, S>(ea1, reg.d[du1]);
        writeM<C, M, S>(ea2, reg.d[du2]);

        prefetch<C, POLL>();
        CYCLES_68020(15)
        return;
    }

    writeD<S>(dc1, data1);
    writeD<S>(dc2, data2);
    prefetch<C, POLL>();

    CYCLES_68020(12)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execChk(u16 opcode)
{
    AVAILABILITY(S == Long ? Core::C68020 : Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);
    u32 ea = 0, data, dy;

    try { readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data); }
    catch (const AddressError &) {

        // Rectify the stack frame (68010 and above). The register has been advanced by readOp already.
        // The DIB still holds the last prefetch.
        if constexpr (C != Core::C68000) {

            constexpr bool noDec = isAbsMode(M) || M == Mode::AI || M == Mode::PI || M == Mode::PD;
            SYNC(2);
            throw AddressError(makeFrame<noDec ? 0 : AE_DEC_PC>(ea));
        }
        throw;
    }
    dy = readD<S>(dst);

    SYNC_68000(6);

    reg.sr.n = 0;
    setUndefinedCHK<C, S>(SEXT<S>(data), SEXT<S>(dy));

    bool above = SEXT<S>(dy) > SEXT<S>(data);
    bool below = SEXT<S>(dy) < 0;

    // The 68000 additionally takes the branch if the difference of both operands has a negative sign,
    // which includes the case where an arithmetic overflow occurs (vAmigaTS: Timing2/CHK/CHK1). UAE doesn't.
    constexpr bool overflowCheck = C == Core::C68000 ? compat != Compatibility::UAE : compat == Compatibility::MAME;
    if constexpr (overflowCheck) above |= S == Word && (i16)(data - dy) < 0;

    if (above || below) {

        if constexpr (C == Core::C68010) {

            // The 68010 prefetches before it traps. The stacked PC is the address of the next instruction.
            prefetch<C, POLL>();
            SYNC(above ? 4 : 6);
            reg.pc -= 2;

        } else {

            SYNC(above ? 2 : 4);
        }

        reg.sr.n = above ? NBIT<S>(dy) : 1;
        execException<C>(M68kException::CHK);

        CYCLES_68000(40)
        CYCLES_68010(44)
        CYCLES_68020(40)
        return;
    }

    prefetch<C, POLL>();
    SYNC_68010(4);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,       10,  8,  8,        0,  0,  8)
    CYCLES_AI   ( 0,  0,  0,       14, 12, 12,        0,  0, 12)
    CYCLES_PI   ( 0,  0,  0,       14, 12, 12,        0,  0, 12)
    CYCLES_PD   ( 0,  0,  0,       16, 14, 13,        0,  0, 13)
    CYCLES_DI   ( 0,  0,  0,       18, 16, 13,        0,  0, 13)
    CYCLES_IX   ( 0,  0,  0,       20, 18, 15,        0,  0, 15)
    CYCLES_AW   ( 0,  0,  0,       18, 16, 12,        0,  0, 12)
    CYCLES_AL   ( 0,  0,  0,       22, 20, 12,        0,  0, 12)
    CYCLES_DIPC ( 0,  0,  0,       18, 16, 13,        0,  0, 13)
    CYCLES_IXPC ( 0,  0,  0,       20, 18, 15,        0,  0, 15)
    CYCLES_IM   ( 0,  0,  0,       14, 12, 10,        0,  0, 12)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execChkCmp2(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    u32 ext = queue.irc;
    int src = _____________xxx(opcode);
    int dst = xxxx____________(ext);
    u32 ea, data1, data2;

    readExt<C>();

    readOp<C, M, S>(src, &ea, &data1);
    data2 = readM<C, M, S>(ea + S);

    i32 bound1 = SEXT<S>(data1);
    i32 bound2 = SEXT<S>(data2);
    i32 compare = dst < 8 ? SEXT<S>(readR(dst)) : readR(dst);

    if (bound1 <= bound2) {
        reg.sr.c = compare < bound1 || compare > bound2;
    } else {
        reg.sr.c = compare < bound1 && compare > bound2;
    }
    reg.sr.z = compare == bound1 || compare == bound2;
    setUndefinedCHK2<C, S>(bound1, bound2, compare);

    if ((ext & 0x800) && reg.sr.c) {

        execException<C>(M68kException::CHK);
        CYCLES_68020(40)

    } else {

        prefetch<C, POLL>();

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_AI   ( 0,  0, 22,        0,  0, 22,        0,  0, 22)
        CYCLES_DI   ( 0,  0, 23,        0,  0, 23,        0,  0, 23)
        CYCLES_IX   ( 0,  0, 25,        0,  0, 25,        0,  0, 25)
        CYCLES_AW   ( 0,  0, 22,        0,  0, 22,        0,  0, 22)
        CYCLES_AL   ( 0,  0, 22,        0,  0, 22,        0,  0, 22)
        CYCLES_DIPC ( 0,  0, 23,        0,  0, 23,        0,  0, 23)
        CYCLES_IXPC ( 0,  0, 23,        0,  0, 23,        0,  0, 23)
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execClr(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);

    if constexpr (C == Core::C68000) {

        u32 ea, data;
        readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);

        // MAME sets the flags first
        setMoveFlags<S>(0);

        try {

            fetchIrc<C, POLL, (!isRegMode(M) || S == Long)>();
            if constexpr (S == Long && isRegMode(M)) SYNC(2);
            writeOp<C, M, S, REVERSE | CLRC>(dst, ea, 0);

        } catch (BusError &) {

            if constexpr (isRegMode(M)) writeD<S == Long ? Word : S>(dst, 0);
            throw;
        }

    } else if constexpr (C == Core::C68010) {

        constexpr Flags incA = S == Long ? AE_INC_A : 0;
        u32 ea = computeEA<C, M, S>(dst);

        switch (M) {

            case Mode::AI:
            case Mode::PI:

                writeOp<C, M, S, POLL | AE_INC_PC>(dst, ea, 0);
                updateAnPI<M, S>(dst);
                if (looping<I>()) { noPrefetch<C>(); loopModeDelay = 4; } else { prefetch<C>(); }
                break;

            case Mode::PD:

                writeOp<C, M, S, REVERSE | POLL | AE_INC_PC | incA>(dst, ea, 0);
                updateAnPD<M, S>(dst);
                if (looping<I>()) { noPrefetch<C>(); loopModeDelay = 4; } else { prefetch<C>(); }
                break;

            case Mode::DI:
            case Mode::IX:

                setMoveFlags<S>(0);

                if constexpr (M == Mode::IX) SYNC(2);
                fetchIrc<C, POLL>();
                writeOp<C, M, S, REVERSE | AE_INC_PC | incA>(dst, ea, 0);
                break;

            case Mode::AW:
            case Mode::AL:

                // Real hardware has updated the flags and reports the second word of a long write
                setMoveFlags<S>(0);

                fetchIrc<C, POLL>();
                writeOp<C, M, S, REVERSE | incA>(dst, ea, 0);
                break;

            default:

                writeOp<C, M, S, REVERSE>(dst, ea, 0);
                prefetch<C, POLL>();
        }

        setMoveFlags<S>(0);

    } else if constexpr (C == Core::C68020) {

        // The 68020 does not read the operand before it clears it
        u32 ea = computeEA<C, M, S>(dst);
        updateAnPD<M, S>(dst);
        updateAnPI<M, S>(dst);

        // UAE sets the flags first
        setMoveFlags<S>(0);

        fetchIrc<C, POLL>();
        if constexpr (S == Long && isRegMode(M)) SYNC(2);
        writeOp<C, M, S>(dst, ea, 0);
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        6,  6,  2)
    CYCLES_AI   (12,  8,  8,       12,  8,  8,       20, 12,  8)
    CYCLES_PI   (12,  8,  8,       12,  8,  8,       20, 12,  8)
    CYCLES_PD   (14, 10,  9,       14, 10,  9,       22, 14,  9)
    CYCLES_DI   (16, 12,  9,       16, 12,  9,       24, 16,  9)
    CYCLES_IX   (18, 14, 11,       18, 14, 11,       26, 20, 11)
    CYCLES_AW   (16, 12,  8,       16, 12,  8,       24, 16,  8)
    CYCLES_AL   (20, 14,  8,       20, 14,  8,       28, 20,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCmp(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea, data;
    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    if constexpr (C == Core::C68000) {

        cmp<C, S>(data, readD<S>(dst));
        prefetchLate<C, POLL, (S == Long)>();

        if constexpr (S == Long) SYNC(2);

    } else {

        if constexpr (S == Long && (isRegMode(M) || isImmMode(M))) SYNC(2);

        cmp<C, S>(data, readD<S>(dst));
        POLL_IPL;
        looping<I>() ? noPrefetch<C>(S == Long ? 4 : 2) : prefetch<C>();
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        6,  6,  2)
    CYCLES_AN   ( 0,  0,  0,        4,  4,  2,        6,  6,  2)
    CYCLES_AI   ( 8,  8,  6,        8,  8,  6,       14, 14,  6)
    CYCLES_PI   ( 8,  8,  6,        8,  8,  6,       14, 14,  6)
    CYCLES_PD   (10, 10,  7,       10, 10,  7,       16, 16,  7)
    CYCLES_DI   (12, 12,  7,       12, 12,  7,       18, 18,  7)
    CYCLES_IX   (14, 14,  9,       14, 14,  9,       20, 20,  9)
    CYCLES_AW   (12, 12,  6,       12, 12,  6,       18, 18,  6)
    CYCLES_AL   (16, 16,  6,       16, 16,  6,       22, 22,  6)
    CYCLES_DIPC (12, 12,  7,       12, 12,  7,       18, 18,  7)
    CYCLES_IXPC (14, 14,  9,       14, 14,  9,       20, 20,  9)
    CYCLES_IM   ( 8,  8,  4,        8,  8,  4,       14, 14,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCmpa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea, data;
    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    data = SEXT<S>(data);
    cmp<C, Long>(data, readA(dst));

    if constexpr (C == Core::C68000) {

        looping<I>() ? noPrefetch<C>() : prefetchLate<C, POLL>();
        SYNC(2);

    } else {

        SYNC(2);
        POLL_IPL;
        looping<I>() ? noPrefetch<C>(2) : prefetch<C>();
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        6,  6,  4,        6,  6,  4)
    CYCLES_AN   ( 0,  0,  0,        6,  6,  4,        6,  6,  4)
    CYCLES_AI   ( 0,  0,  0,       10, 10,  8,       14, 14,  8)
    CYCLES_PI   ( 0,  0,  0,       10, 10,  8,       14, 14,  8)
    CYCLES_PD   ( 0,  0,  0,       12, 12,  9,       16, 16,  9)
    CYCLES_DI   ( 0,  0,  0,       14, 14,  9,       18, 18,  9)
    CYCLES_IX   ( 0,  0,  0,       16, 16, 11,       20, 20, 11)
    CYCLES_AW   ( 0,  0,  0,       14, 14,  8,       18, 18,  8)
    CYCLES_AL   ( 0,  0,  0,       18, 18,  8,       22, 22,  8)
    CYCLES_DIPC ( 0,  0,  0,       14, 14,  9,       18, 18,  9)
    CYCLES_IXPC ( 0,  0,  0,       16, 16, 11,       20, 20, 11)
    CYCLES_IM   ( 0,  0,  0,       10, 10,  6,       14, 14,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCmpiRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 src  = readI<C, S>();
    int dst  = _____________xxx(opcode);
    u32 data = readD<S>(dst);

    cmp<C, S>(src, data);
    prefetchLate<C, POLL, (S == Long)>();

    if constexpr (S == Long && C == Core::C68000) SYNC(2);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (8,   8,  2,        8,  8,  2,       14, 12,  2)
    CYCLES_AI   (12, 12,  6,       12, 12,  6,       20, 20,  6)
    CYCLES_PI   (12, 12,  6,       12, 12,  6,       20, 20,  6)
    CYCLES_PD   (14, 14,  7,       14, 14,  7,       22, 22,  7)
    CYCLES_DI   (16, 16,  7,       16, 16,  7,       24, 24,  7)
    CYCLES_IX   (18, 18,  9,       18, 18,  9,       26, 26,  9)
    CYCLES_AW   (16, 16,  6,       16, 16,  6,       24, 24,  6)
    CYCLES_AL   (20, 20,  6,       20, 20,  6,       28, 28,  6)
    CYCLES_DIPC ( 0,  0,  7,        0,  0,  7,        0,  0,  7)
    CYCLES_IXPC ( 0,  0,  9,        0,  0,  9,        0,  0,  9)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCmpiEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 src = readIFrame<C, S>();
    int dst = _____________xxx(opcode);
    u32 ea, data;
    
    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);
    cmp<C, S>(src, data);
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (8,   8,  2,        8,  8,  2,       14, 12,  2)
    CYCLES_AI   (12, 12,  6,       12, 12,  6,       20, 20,  6)
    CYCLES_PI   (12, 12,  6,       12, 12,  6,       20, 20,  6)
    CYCLES_PD   (14, 14,  7,       14, 14,  7,       22, 22,  7)
    CYCLES_DI   (16, 16,  7,       16, 16,  7,       24, 24,  7)
    CYCLES_IX   (18, 18,  9,       18, 18,  9,       26, 26,  9)
    CYCLES_AW   (16, 16,  6,       16, 16,  6,       24, 24,  6)
    CYCLES_AL   (20, 20,  6,       20, 20,  6,       28, 28,  6)
    CYCLES_DIPC ( 0, 16,  7,        0, 16,  7,        0, 24,  7)
    CYCLES_IXPC ( 0, 18,  9,        0, 18,  9,        0, 26,  9)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCmpm(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea1, ea2, data1, data2;

    if constexpr (C == Core::C68000) {

        if constexpr (compat == Compatibility::UAE) {

            // UAE advances the source register by the number of words read (including the failing one)
            try { readOp<C, M, S, AE_INC_PC>(src, &ea1, &data1); } catch (const BusError &e) {
                if constexpr (S == Byte) updateAnPI<M, S>(src);
                if constexpr (S == Long) U32_INC(reg.a[src], e.upperWord ? 2 : 4);
                throw;
            }
            POLL_IPL;
            readOp<C, M, S, AE_INC_PC>(dst, &ea2, &data2);

        } else {

            // MAME advances the source register by 2 if the upper word of a long operand fails
            try { readOp<C, M, S, AE_INC_PC>(src, &ea1, &data1); }
            catch (const AddressError &) { if constexpr (S == Long) U32_INC(reg.a[src], 2); throw; }
            catch (const BusError &e) { if (S == Long && e.upperWord) U32_INC(reg.a[src], 2); throw; }
            POLL_IPL;

            // MAME does not advance the destination register if the read fails
            u32 an = reg.a[dst];
            try { readOp<C, M, S, AE_INC_PC>(dst, &ea2, &data2); }
            catch (...) { reg.a[dst] = an; throw; }
        }

        cmp<C, S>(data1, data2);
        prefetch<C>();

    } else if constexpr (C == Core::C68010) {

        // The 68010 stacks the PC of the executing instruction. A failing long read advances the source register by 2.
        try { readOp<C, M, S, 0>(src, &ea1, &data1); }
        catch (const AddressError &) { if constexpr (S == Long) U32_INC(reg.a[src], 2); throw; }
        readOp<C, M, S, 0>(dst, &ea2, &data2);
        POLL_IPL;

        cmp<C, S>(data1, data2);
        looping<I>() ? noPrefetch<C>(S == Long ? 2 : 0) : prefetch<C>();

    } else {

        readOp<C, M, S, AE_INC_PC>(src, &ea1, &data1);
        readOp<C, M, S, AE_INC_PC>(dst, &ea2, &data2);
        POLL_IPL;

        cmp<C, S>(data1, data2);
        looping<I>() ? noPrefetch<C>(S == Long ? 2 : 0) : prefetch<C>();
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_PI   (12, 12,  9,       12, 12,  9,       20, 20,  9)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpBcc(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // First cycle of the coprocessor protocol
    notifyCPCycle(opcode, CIR::CONDITION);
    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpDbcc(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // First cycle of the coprocessor protocol
    notifyCPCycle(opcode, CIR::CONDITION);
    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpGen(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // First cycle of the coprocessor protocol
    notifyCPCycle(opcode, CIR::COMMAND);
    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpRestore(u16 opcode)
{
    AVAILABILITY(Core::C68020)
    SUPERVISOR_MODE_ONLY

    // First cycle of the coprocessor protocol
    notifyCPCycle(opcode, CIR::RESTORE);
    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpRestoreInvalid(u16 opcode)
{
    AVAILABILITY(Core::C68020)
    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpSave(u16 opcode)
{
    AVAILABILITY(Core::C68020)
    SUPERVISOR_MODE_ONLY

    // First cycle of the coprocessor protocol
    notifyCPCycle(opcode, CIR::SAVE);
    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpScc(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // First cycle of the coprocessor protocol
    notifyCPCycle(opcode, CIR::CONDITION);
    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpTrapcc(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // First cycle of the coprocessor protocol
    notifyCPCycle(opcode, CIR::CONDITION);
    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execDbcc(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    if constexpr (C == Core::C68000) {

        SYNC(2);
        if (!cond<I>()) {

            int dn = _____________xxx(opcode);
            u32 newpc = U32_ADD(reg.pc, (i16)queue.irc);
            bool takeBranch = readD<Word>(dn) != 0;

            if (misalignedPC<C>(newpc)) {

                if constexpr (compat == Compatibility::UAE) {

                    // UAE stacks the target address plus 2
                    reg.pc = newpc;
                    throw earlyAddressError<C>(makeFrame<AE_INC_PC|AE_FETCH>(newpc));

                } else if constexpr (compat == Compatibility::MAME) {

                    // MAME stacks the address behind the extension word
                    throw earlyAddressError<C>(makeFrame<AE_INC_PC|AE_FETCH>(newpc));

                } else {

                    // A real 68000 stacks the address of the extension word (vAmigaTS: addrerr40, addrerr41)
                    throw earlyAddressError<C>(makeFrame<AE_FETCH>(newpc));
                }
            }

            if (takeBranch) {

                // MAME decrements the counter after the first fetch at the branch target
                fetchFirstAt<C>(newpc, reg.pc + 2);
                writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));
                prefetch<C, POLL>();
                changeOfFlow<C>();
                CYCLES_68000(10);

            } else {

                writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));
                (void)read<C, AddrSpace::PROG, Word>(reg.pc + 2);
                CYCLES_68000(14);
                fullPrefetchAt<C, POLL>(reg.pc + 2);
            }

        } else {

            SYNC(2);
            CYCLES_68000(12);

            // Fall through to next instruction
            fullPrefetchAt<C, POLL>(reg.pc + 2);
        }

    } else if constexpr (C == Core::C68010) {

        if (looping<I>()) {

            if (!cond<I>()) {

                int dn = _____________xxx(opcode);
                u32 newpc = U32_ADD(reg.pc, -2);
                bool takeBranch = readD<Word>(dn) != 0;

                if (misalignedPC<C>(newpc)) {

                    reg.pc = newpc;
                    throw earlyAddressError<C>(makeFrame<AE_INC_PC|AE_FETCH>(newpc));
                }

                // Decrement loop counter
                writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));

                if (takeBranch) {

                    // Stay in loop mode
                    SYNC(6);
                    reg.pc = newpc;
                    queue.ird = queue.irc;
                    queue.irc = opcode;
                    POLL_IPL;

                } else {

                    // Leave loop mode and fall through to next instruction
                    if (loopModeDelay) SYNC(loopModeDelay);
                    loopModeDelay = 2;
                    reg.pc += 2;
                    fullPrefetch<C, POLL>();
                    flags &= ~State::LOOPING;
                }

            } else {

                // Leave loop mode and fall through to next instruction
                SYNC(4);
                reg.pc += 2;
                fullPrefetch<C, POLL>();
                flags &= ~State::LOOPING;
            }

        } else {

            if (!cond<I>()) {

                SYNC(2);

                int dn = _____________xxx(opcode);
                i16 disp = (i16)queue.irc;
                u32 newpc = U32_ADD(reg.pc, disp);
                bool takeBranch = readD<Word>(dn) != 0;

                if (misalignedPC<C>(newpc)) {

                    // A real 68010 stacks the address of the extension word, and the instruction
                    // input buffer of the frame holds $FFFF (vAmigaTS: addrerr40, addrerr41)
                    queue.irc = 0xFFFF;
                    throw earlyAddressError<C>(makeFrame<AE_FETCH>(newpc));
                }

                // Decrement loop counter
                writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));

                if (takeBranch) {

                    reg.pc = newpc;
                    fullPrefetch<C, POLL>();

                    // Enter loop mode if the loop body is a single loopable instruction
                    if (loop[queue.ird] && disp == -4) {

                        flags |= State::LOOPING;
                        queue.irc = opcode;
                    }

                    if constexpr (compat == Compatibility::MUSASHI) SYNC(2);
                    CYCLES_68010(12);

                } else {

                    (void)read<C, AddrSpace::PROG, Word>(reg.pc + 2);
                    SYNC((compat == Compatibility::MUSASHI) ? 4 : 2);
                    CYCLES_68010(8);

                    reg.pc += 2;
                    fullPrefetch<C, POLL>();
                    CYCLES_68010(10);
                }

            } else {

                SYNC(2);
                CYCLES_68010(2);

                // Fall through to next instruction
                reg.pc += 2;
                fullPrefetch<C, POLL>();
                CYCLES_68010(10);
            }
        }

    } else if constexpr (C == Core::C68020) {

        SYNC(2);
        if (!cond<I>()) {

            int dn = _____________xxx(opcode);
            u32 newpc = U32_ADD(reg.pc, (i16)queue.irc);
            bool takeBranch = readD<Word>(dn) != 0;

            if (misalignedPC<C>(newpc) && takeBranch) {

                // The 68020 has decremented the counter at this point
                writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));
                reg.pc = newpc;
                throw earlyAddressError<C>(makeFrame<AE_INC_PC|AE_FETCH>(newpc));
            }

            if (takeBranch) {

                // Like UAE, decrement the counter after the first fetch at the branch target
                fetchFirstAt<C>(newpc, reg.pc + 2);
                writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));
                prefetch<C, POLL>();
                changeOfFlow<C>();
                CYCLES_68020(6);

            } else {

                writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));
                (void)read<C, AddrSpace::PROG, Word>(reg.pc + 2);
                CYCLES_68020(10);
                fullPrefetchAt<C, POLL>(reg.pc + 2);
            }

        } else {

            SYNC(2);
            CYCLES_68020(6);

            // Fall through to next instruction
            fullPrefetchAt<C, POLL>(reg.pc + 2);
        }
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execExgDxDy(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    std::swap(reg.d[src], reg.d[dst]);
    prefetchLate<C, POLL>();
    SYNC(2);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        6,  6,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execExgAxDy(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    std::swap(reg.a[src], reg.d[dst]);
    prefetchLate<C, POLL>();
    SYNC(2);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        6,  6,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execExgAxAy(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    std::swap(reg.a[src], reg.a[dst]);
    prefetchLate<C, POLL>();
    SYNC(2);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        6,  6,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execExt(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int n = _____________xxx(opcode);

    u32 dn = readD(n);
    dn = (S == Long) ? SEXT<Word>(dn) : SEXT<Byte>(dn);

    writeD<S>(n, dn);
    setMoveFlags<S>(dn);
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        4,  4,  4,        4,  4,  4)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execExtb(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    int n = _____________xxx(opcode);

    u32 dn = readD(n);
    dn = SEXT<Byte>(dn);

    writeD(n, dn);
    setMoveFlags<S>(dn);
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        0,  0,  0,        0,  0,  4)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execJmp(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 oldpc = reg.pc;

    int src = _____________xxx(opcode);
    u32 ea  = computeEA<C, M, Long, SKIP_LAST_RD>(src);

    // Indexed modes take 4 extra cycles, the other modes with an extension word 2 (except absolute long)
    [[maybe_unused]] constexpr int delay =
    (M == Mode::IX || M == Mode::IXPC) ? 4 :
    (M == Mode::DI || M == Mode::AW || M == Mode::DIPC) ? 2 : 0;
    SYNC(delay);

    // Check for address error
    if (misalignedPC<C>(ea)) {
        throw earlyAddressError<C>(makeFrame<AE_FETCH>(ea, oldpc));
    }

    // Jump to new address
    // A fault on the first fetch stacks the address of the extension word
    fullPrefetchAt<C, POLL>(ea, reg.pc0 + 2);

    changeOfFlow<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        8,  8,  4)
    CYCLES_DI   ( 0,  0,  0,        0,  0,  0,       10, 10,  5)
    CYCLES_IX   ( 0,  0,  0,        0,  0,  0,       14, 14,  7)
    CYCLES_AW   ( 0,  0,  0,        0,  0,  0,       10, 10,  4)
    CYCLES_AL   ( 0,  0,  0,        0,  0,  0,       12, 12,  4)
    CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,       10, 10,  5)
    CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,       14, 14,  7)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execJsr(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    u32 ea  = computeEA<C, M, Long, SKIP_LAST_RD>(src);

    // Indexed modes take 4 extra cycles, the other modes with an extension word 2 (except absolute long)
    [[maybe_unused]] constexpr int delay =
    (M == Mode::IX || M == Mode::IXPC) ? 4 :
    (M == Mode::DI || M == Mode::AW || M == Mode::DIPC) ? 2 : 0;
    
    SYNC(delay);

    if constexpr (C == Core::C68000) {

        // MAME keeps the program counter unchanged, UAE decrements it in (d,An) and (d,An,Xi) mode
        constexpr Flags flags = isDspMode(M) && compat != Compatibility::MAME ? AE_DEC_PC : AE_FETCH;
        checkEven<C, Word, flags>(ea);

        // Fetch the first word of the subroutine before pushing
        u32 ret = reg.pc;
        reg.pc = ea;
        bool pushing = false;
        try {
            queue.irc = (u16)read<C, AddrSpace::PROG, Word>(ea);
            pushing = true;
            push<C, Long>(ret);

        } catch (BusError &e) {

            // MAME stacks the return address. UAE stacks the target address if the push fails.
            u32 target = ea + 2 + (M == Mode::AW ? 2 : M == Mode::AL ? 4 : 0);
            e.stackFrame.pc = (compat == Compatibility::UAE && pushing) ? target : ret;
            throw;

        } catch (AddressError &e) {

            // MAME stacks the return address of the subroutine call
            e.stackFrame.pc = ret;
            throw;
        }
        prefetch<C, POLL>();

    } else if constexpr (C == Core::C68010) {

        if (misaligned<C>(reg.sp)) {

            // Both addresses are misaligned
            if (misaligned<C>(ea)) {

                if constexpr (M == Mode::AI) {

                    reg.pc = ea & ~1;
                    queue.irc = (u16)read<C, AddrSpace::PROG, Word>(ea & ~1);
                    throw earlyAddressError<C>(makeFrame<AE_FETCH>(ea));

                } else if constexpr (isAbsMode(M)) {

                    auto frame = makeFrame<AE_FETCH>(ea);
                    frame.pc -= 4;
                    throw earlyAddressError<C>(frame);

                } else if constexpr (isDspMode(M)) {

                    throw earlyAddressError<C>(makeFrame<AE_DEC_PC|AE_FETCH>(ea));

                } else {

                    throw earlyAddressError<C>(makeFrame<AE_FETCH>(ea));
                }
            }

            // Only the stack pointer is misaligned
            prefetch<C>();
            reg.sp -= 4;
            writeBuffer = (u16)READ<Word, 1>(reg.pc);
            throw earlyAddressError<C>(makeFrame<AE_DATA>(reg.sp));
        }
        // A real 68010 stacks the address of the extension word in (d16,An) mode (vAmigaTS: addrerr42)
        checkEvenPC<C, isDspMode(M) ? AE_DEC_PC|AE_FETCH : AE_FETCH>(ea);

        // Save return address on stack
        push<C, Long, POLL>(reg.pc);

        // Jump to new address
        reg.pc = ea;
        fullPrefetch<C>();

    } else if constexpr (C == Core::C68020) {

        if (misalignedPC<C>(ea)) {

            // UAE makes room for the return address on the stack (but does not write it)
            reg.sp -= 4;
            throw earlyAddressError<C>(makeFrame<AE_FETCH>(ea));
        }

        // Save return address on stack
        push<C, Long, POLL>(reg.pc);

        // Jump to new address
        reg.pc = ea;
        fullPrefetch<C>();
    }

    changeOfFlow<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   ( 0,  0,  0,        0,  0,  0,       16, 16,  4)
    CYCLES_DI   ( 0,  0,  0,        0,  0,  0,       18, 18,  5)
    CYCLES_IX   ( 0,  0,  0,        0,  0,  0,       22, 22,  7)
    CYCLES_AW   ( 0,  0,  0,        0,  0,  0,       18, 18,  4)
    CYCLES_AL   ( 0,  0,  0,        0,  0,  0,       20, 20,  4)
    CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,       18, 18,  5)
    CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,       22, 22,  7)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execLea(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    if constexpr (C == Core::C68000 && M == Mode::AL) {

        // Load the high word into the register first, then the complete address
        reg.a[dst] = WRITE<Word, 1>(reg.a[dst], queue.irc);
        readExt<C>();
        reg.a[dst] = WRITE<Word, 0>(reg.a[dst], queue.irc);
        readExtLast<C>();

    } else if constexpr (C == Core::C68000 && (M == Mode::AW || M == Mode::DI || M == Mode::DIPC)) {

        // Load the address register before the extension word is refilled
        i16 d = (i16)queue.irc;
        reg.a[dst] = M == Mode::AW ? (u32)d : U32_ADD(M == Mode::DI ? reg.a[src] : reg.pc, d);
        readExt<C>();

    } else {

        reg.a[dst] = computeEA<C, M, S>(src);
    }
    
    if (isIdxMode(M)) SYNC(2);
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        0,  0,  0,        0,  0,  0)
    CYCLES_AN   ( 0,  0,  0,        0,  0,  0,        0,  0,  0)
    CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        4,  4,  6)
    CYCLES_PI   ( 0,  0,  0,        0,  0,  0,        0,  0,  0)
    CYCLES_PD   ( 0,  0,  0,        0,  0,  0,        0,  0,  0)
    CYCLES_DI   ( 0,  0,  0,        0,  0,  0,        8,  8,  7)
    CYCLES_IX   ( 0,  0,  0,        0,  0,  0,       12, 12,  9)
    CYCLES_AW   ( 0,  0,  0,        0,  0,  0,        8,  8,  6)
    CYCLES_AL   ( 0,  0,  0,        0,  0,  0,       12, 12,  6)
    CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,        8,  8,  7)
    CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,       12, 12,  9)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execLink(u16 opcode)
{
    AVAILABILITY(S == Long ? Core::C68020 : Core::C68000)

    u32 sp = getSP() - 4;
    int ax = _____________xxx(opcode);

    i32 disp;
    try { disp = SEXT<S>(readI<C, S>()); } catch (BusError &e) {
        
        if constexpr (C == Core::C68000) { e.stackFrame.pc += 2; }
        throw;
    }

    // Check for address error
    if (misaligned<C>(sp)) {

        writeBuffer = (u16)READ<Word, 1>(readA(ax));
        writeA(ax, sp);
        throw earlyAddressError<C>(makeFrame<AE_DATA|AE_WRITE|AE_INC_PC>(sp));
    }

    POLL_IPL;

    // LINK A7 pushes the decremented stack pointer on the 68040
    bool decremented = ax == 7 && cpuModel >= CPUModel::M68EC040;

    // Write to stack
    try { push<C, Long, AE_INC_PC>(readA(ax) - (decremented ? 4 : 0)); } catch (BusError &) {

        if constexpr (C == Core::C68000) { reg.sp += 4; writeA(ax, sp); }
        throw;
    }

    // Modify address register and stack pointer
    writeA(ax, sp);
    reg.sp = U32_ADD(reg.sp, disp);

    prefetch<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,       16, 16,  5,        0,  0,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove0(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    setMoveFlags<S>(data);
    writeOp<C, Mode::DN, S>(dst, data);
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        4,  4,  2)
    CYCLES_AN   ( 4,  4,  2,        4,  4,  2,        4,  4,  2)
    CYCLES_AI   ( 8,  8,  6,        8,  8,  6,       12, 12,  6)
    CYCLES_PI   ( 8,  8,  6,        8,  8,  6,       12, 12,  6)
    CYCLES_PD   (10, 10,  7,       10, 10,  7,       14, 14,  7)
    CYCLES_DI   (12, 12,  7,       12, 12,  7,       16, 16,  7)
    CYCLES_IX   (14, 14,  9,       14, 14,  9,       18, 18,  9)
    CYCLES_AW   (12, 12,  6,       12, 12,  6,       16, 16,  6)
    CYCLES_AL   (16, 16,  6,       16, 16,  6,       20, 20,  6)
    CYCLES_DIPC (12, 12,  7,       12, 12,  7,       16, 16,  7)
    CYCLES_IXPC (14, 14,  9,       14, 14,  9,       18, 18,  9)
    CYCLES_IM   ( 8,  8,  4,        8,  8,  4,       12, 12,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove2(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    if constexpr (C == Core::C68000) {

        // A failed write shows the flags set up in front of it
        constexpr Flags F = AE_INC_PC | ((S == Long && !isMemMode(M)) ? POLL : 0);
        writeBuffer = (u16)READ<Word, S == Long ? 1 : 0>(data);
        u32 ea2 = readA(dst);

        if constexpr (S == Long) {

            // First write: untouched flags (register source) or the flags of the low word (memory source)
            if constexpr (isMemMode(M)) {
                setMoveFlags<Word>(data);
            }
            writeM<C, Mode::AI, Word, AE_INC_PC | UAE_DEC_PC>(ea2, READ<Word, 1>(data));

            // Second write: the flags of the low word (register source) or the final flags (memory source)
            setMoveFlags<isMemMode(M) ? Long : Word>(data);
            writeM<C, Mode::AI, Word, F | UAE_DEC_PC>(ea2 + 2, READ<Word>(data));

        } else {

            setMoveFlags<S>(data);
            writeM<C, Mode::AI, S, F | UAE_DEC_PC>(ea2, data);
        }

        setMoveFlags<S>(data);

        // The interrupt level is polled in the second write if it is the last bus cycle (register source)
        if constexpr (S == Long && !isMemMode(M)) looping<I>() ? noPrefetch<C>() : prefetch<C>();
        else looping<I>() ? noPrefetch<C>() : prefetch<C, POLL>();

    } else {

        if constexpr (!isMemMode(M)) {

            POLL_IPL;

            writeOp<C, Mode::AI, S, AE_INC_PC>(dst, data);

            setMoveFlags<S>(data);

            looping<I>() ? noPrefetch<C>() : prefetch<C>();

        } else {

            setMoveFlags<S == Long ? Word : S>(data);

            writeOp<C, Mode::AI, S, AE_INC_PC>(dst, data);

            setMoveFlags<S>(data);

            looping<I>() ? noPrefetch<C>() : prefetch<C, POLL>();
            if (looping<I>() && S == Long) loopModeDelay = 0;
        }
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 8,  8,  4,        8,  8,  4,       12, 12,  4)
    CYCLES_AN   ( 8,  8,  4,        8,  8,  4,       12, 12,  4)
    CYCLES_AI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PD   (14, 14,  9,       14, 14,  9,       22, 22,  9)
    CYCLES_DI   (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_IX   (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_AW   (16, 16,  8,       16, 16,  8,       24, 24,  8)
    CYCLES_AL   (20, 20,  8,       20, 20,  8,       28, 28,  8)
    CYCLES_DIPC (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_IXPC (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_IM   (12, 12,  6,       12, 12,  6,       20, 20,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove3(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    if constexpr (C == Core::C68000) {

        // A failed write shows the flags set up in front of it
        constexpr Flags F = AE_INC_PC | (true ? POLL : 0);
        writeBuffer = (u16)READ<Word, S == Long ? 1 : 0>(data);
        u32 ea2 = readA(dst);

        if constexpr (S == Long) {

            // First write: untouched flags (register source) or the flags of the low word (memory source)
            if constexpr (isMemMode(M)) {
                setMoveFlags<Word>(data);
            }
            writeM<C, Mode::PI, Word, AE_INC_PC | UAE_DEC_PC>(ea2, READ<Word, 1>(data));

            // Second write: the flags of the low word (register source) or the final flags (memory source)
            setMoveFlags<isMemMode(M) ? Long : Word>(data);
            writeM<C, Mode::PI, Word, F | UAE_DEC_PC>(ea2 + 2, READ<Word>(data));

        } else {

            setMoveFlags<S>(data);
            writeM<C, Mode::PI, S, F | UAE_DEC_PC>(ea2, data);
        }
        updateAnPI<Mode::PI, S>(dst);
        setMoveFlags<S>(data);
        looping<I>() ? noPrefetch<C>() : prefetch<C>();

    } else {

        if constexpr (!isMemMode(M)) {

            POLL_IPL;
            writeOp<C, Mode::PI, S, AE_INC_PC>(dst, data);

            setMoveFlags<S>(data);

            looping<I>() ? noPrefetch<C>() : prefetch<C>();

        } else {

            setMoveFlags<S == Long ? Word : S>(data);

            writeOp<C, Mode::PI, S, AE_INC_PC|POLL>(dst, data);
            looping<I>() ? noPrefetch<C>() : prefetch<C>();
            if (looping<I>() && S == Long) loopModeDelay = 0;

            setMoveFlags<S>(data);
        }
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 8,  8,  4,        8,  8,  4,       12, 12,  4)
    CYCLES_AN   ( 8,  8,  4,        8,  8,  4,       12, 12,  4)
    CYCLES_AI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PD   (14, 14,  9,       14, 14,  9,       22, 22,  9)
    CYCLES_DI   (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_IX   (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_AW   (16, 16,  8,       16, 16,  8,       24, 24,  8)
    CYCLES_AL   (20, 20,  8,       20, 20,  8,       28, 28,  8)
    CYCLES_DIPC (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_IXPC (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_IM   (12, 12,  6,       12, 12,  6,       20, 20,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove4(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u16 ird = getIRD();
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    /* Source: http://pasti.fxatari.com/68kdocs/68kPrefetch.html
     *
     * "When the destination addressing mode is pre-decrement, steps 4 and 5
     *  above are inverted. So it behaves like a read modify instruction and it
     *  is a class 0 instruction. Note: The behavior is the same disregarding
     *  transfer size (byte, word or long), and disregarding the source
     *  addressing mode."
     */
    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    setMoveFlags<S>(data);

    // MAME shows the flags of the low word if the prefetch fails
    try { looping<I>() ? noPrefetch<C>(2) : fetchIrc<C, POLL>(); } catch (BusError &e) {
        
        if constexpr (S == Long) {
            rethrow<C>(e, CCR::N | CCR::Z, (NBIT<Word>(data) ? CCR::N : 0) | (ZERO<Word>(data) ? CCR::Z : 0));
        }
        throw;
    }
    if (looping<I>() && S == Long) loopModeDelay = 0;

    ea = computeEA<C, Mode::PD, S, IMPL_DEC>(dst);
    writeBuffer = (u16)READ<Word>(data);

    constexpr bool early = S != Long || C == Core::C68020;
    if constexpr (early) updateAn<Mode::PD, S>(dst);

    // Check for address error
    if (misaligned<C, S>(ea)) {

        // MAME has replaced the IRD when the address error is detected
        commitIrd<C>();

        if constexpr (compat == Compatibility::UAE) SYNC(2);
        if constexpr (S == Long) throw earlyAddressError<C>(makeFrame<AE_WRITE|AE_DATA>(ea + 2, reg.pc, getSR(), ird));

        SYNC(2);
        throw AddressError(makeFrame<AE_WRITE|AE_DATA>(ea));
    }

    if constexpr (S == Long && C == Core::C68000 && compat == Compatibility::UAE) {

        try {

            // The low word is written first. UAE decrements the register after both writes.
            writeM<C, Mode::PD, Word>(ea + 2, READ<Word>(data));
            writeM<C, Mode::PD, Word, COMMIT_IRD>(ea, READ<Word, 1>(data));

        } catch (BusError &e) {

            // UAE stacks the unmasked address
            e.stackFrame.addr = (e.stackFrame.addr & 0xFFFFFF) | (ea & 0xFF000000);
            throw;
        }
        updateAn<Mode::PD, S>(dst);

    } else if constexpr (S == Long && C == Core::C68000) {

        // The low word is written first. MAME decrements the register before the second write.
        writeM<C, Mode::PD, Word>(ea + 2, READ<Word>(data));
        updateAn<Mode::PD, S>(dst);
        writeM<C, Mode::PD, Word, COMMIT_IRD>(ea, READ<Word, 1>(data));

    } else {

        writeM<C, Mode::PD, S, REVERSE | COMMIT_IRD>(ea, data);
        if constexpr (!early) updateAn<Mode::PD, S>(dst);
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 8,  8,  5,        8,  8,  5,       12, 14,  5)
    CYCLES_AN   ( 0,  0,  0,        8,  8,  5,       12, 14,  5)
    CYCLES_AI   (12, 12,  9,       12, 12,  9,       20, 22,  9)
    CYCLES_PI   (12, 12,  9,       12, 12,  9,       20, 22,  9)
    CYCLES_PD   (14, 14, 10,       14, 14, 10,       22, 24, 10)
    CYCLES_DI   (16, 16, 10,       16, 16, 10,       24, 26, 10)
    CYCLES_IX   (18, 18, 12,       18, 18, 12,       26, 28, 12)
    CYCLES_AW   (16, 16,  9,       16, 16,  9,       24, 26,  9)
    CYCLES_AL   (20, 20,  9,       20, 20,  9,       28, 30,  9)
    CYCLES_DIPC (16, 16, 10,       16, 16, 10,       24, 26, 10)
    CYCLES_IXPC (18, 18, 12,       18, 18, 12,       26, 28, 12)
    CYCLES_IM   (12, 12,  7,       12, 12,  7,       20, 22,  9)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove5(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    execMoveDisp<C, I, M, S, Mode::DI>(opcode);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (12, 12,  5,       12, 12,  5,       16, 16,  5)
    CYCLES_AN   (12, 12,  5,       12, 12,  5,       16, 16,  5)
    CYCLES_AI   (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_PI   (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_PD   (18, 18, 10,       18, 18, 10,       26, 26, 10)
    CYCLES_DI   (20, 20, 10,       20, 20, 10,       28, 28, 10)
    CYCLES_IX   (22, 22, 12,       22, 22, 12,       30, 30, 12)
    CYCLES_AW   (20, 20,  9,       20, 20,  9,       28, 28,  9)
    CYCLES_AL   (24, 24,  9,       24, 24,  9,       32, 32,  9)
    CYCLES_DIPC (20, 20, 10,       20, 20, 10,       28, 28, 10)
    CYCLES_IXPC (22, 22, 12,       22, 22, 12,       30, 30, 12)
    CYCLES_IM   (16, 16,  7,       16, 16,  7,       24, 24,  9)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove6(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    execMoveDisp<C, I, M, S, Mode::IX>(opcode);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (14, 14,  7,       14, 14,  7,       18, 18,  7)
    CYCLES_AN   (14, 14,  7,       14, 14,  7,       18, 18,  7)
    CYCLES_AI   (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_PI   (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_PD   (20, 20, 12,       20, 20, 12,       28, 28, 12)
    CYCLES_DI   (22, 22, 12,       22, 22, 12,       30, 30, 12)
    CYCLES_IX   (24, 24, 14,       24, 24, 14,       32, 32, 14)
    CYCLES_AW   (22, 22, 11,       22, 22, 11,       30, 30, 11)
    CYCLES_AL   (26, 26, 11,       26, 26, 11,       34, 34, 11)
    CYCLES_DIPC (22, 22, 12,       22, 22, 12,       30, 30, 12)
    CYCLES_IXPC (24, 24, 14,       24, 24, 14,       32, 32, 14)
    CYCLES_IM   (18, 18,  9,       18, 18,  9,       26, 26, 11)

    FINALIZE
}

// MOVE to a destination with a displacement: (d16,An) or (d8,An,Xn)
template <Core C, Instr I, Mode M, Size S, Mode DM> void
Moira::execMoveDisp(u16 opcode)
{
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    if constexpr (C == Core::C68000) {

        writeBuffer = (u16)READ<Word, S == Long ? 1 : 0>(data);
        if constexpr (isMemMode(M)) setMoveFlags<S == Long ? Word : S>(data);
        
        u32 ea2 = computeEA<C, DM, S, NO_DEC_PC>(dst);

        if constexpr (S == Long && !isMemMode(M)) {

            // The first write shows the flags of the high word
            reg.sr.n = NBIT<Word>(READ<Word, 1>(data));
            reg.sr.z = ZERO<Word>(READ<Word, 1>(data)) && reg.sr.z;
            writeM<C, DM, Word>(ea2, READ<Word, 1>(data));
            setMoveFlags<Word>(data);

            // The second write shows the flags of the low word
            writeM<C, DM, Word, POLL>(ea2 + 2, READ<Word>(data));
            setMoveFlags<S>(data);
            prefetch<C>();

        } else {

            // A failed write shows the final flags
            setMoveFlags<S>(data);
            writeM<C, DM, S>(ea2, data);
            prefetch<C, POLL>();
        }

    } else {

        if constexpr (S == Long && !isMemMode(M)) {

            writeOp<C, DM, S, POLL>(dst, data);
            setMoveFlags<S>(data);
            prefetch<C>();

        } else {

            writeOp<C, DM, S>(dst, data);
            setMoveFlags<S>(data);
            prefetch<C, POLL>();
        }
    }
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove7(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    if constexpr (C == Core::C68000) {

        // A failed extension word fetch shows the flags of the (low word of the) operation
        setMoveFlags<S == Long ? Word : S>(data);

        writeBuffer = (u16)READ<Word, S == Long ? 1 : 0>(data);
        u32 ea2 = computeEA<C, Mode::AW, S>(dst);

        // A failed write shows the final flags
        setMoveFlags<S>(data);
        writeM<C, Mode::AW, S>(ea2, data);

    } else {

        writeOp<C, Mode::AW, S>(dst, data);
        setMoveFlags<S>(data);
    }
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (12, 12,  4,       12, 12,  4,       16, 16,  4)
    CYCLES_AN   (12, 12,  4,       12, 12,  4,       16, 16,  4)
    CYCLES_AI   (16, 16,  8,       16, 16,  8,       24, 24,  8)
    CYCLES_PI   (16, 16,  8,       16, 16,  8,       24, 24,  8)
    CYCLES_PD   (18, 18,  9,       18, 18,  9,       26, 26,  9)
    CYCLES_DI   (20, 20,  9,       20, 20,  9,       28, 28,  9)
    CYCLES_IX   (22, 22, 11,       22, 22, 11,       30, 30, 11)
    CYCLES_AW   (20, 20,  8,       20, 20,  8,       28, 28,  8)
    CYCLES_AL   (24, 24,  8,       24, 24,  8,       32, 32,  8)
    CYCLES_DIPC (20, 20,  9,       20, 20,  9,       28, 28,  9)
    CYCLES_IXPC (22, 22, 11,       22, 22, 11,       30, 30, 11)
    CYCLES_IM   (16, 16,  6,       16, 16,  6,       24, 24,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove8(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    /* Source: http://pasti.fxatari.com/68kdocs/68kPrefetch.html
     *
     * "When the destination addressing mode is long absolute and the source
     *  operand is any memory addr.mode, step 4 is interleaved in the middle of
     *  step 3. Step 3 only performs a single prefetch in this case. The other
     *  prefetch cycle that is normally performed at that step is deferred
     *  after the write cycles. So, two prefetch cycles are performed after the
     *  write ones. It is a class 2 instruction. Note: The behavior is the same
     *  disregarding transfer size (byte, word or long). But if the source
     *  operand is a data or address register, or immediate, then the behavior
     *  is the same as other MOVE variants (class 1 instruction)."
     */
    if constexpr (isMemMode(M)) {

        readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

        if constexpr (C == Core::C68000) {

            // A failed first extension word fetch shows the flags of the operation (untouched if long)
            if constexpr (S != Long) setMoveFlags<S>(data);
            u32 ea2 = queue.irc << 16;
            readExt<C, NO_DEC_PC>();
            ea2 |= queue.irc;

            // An address error shows the flags of the (low word of the) operation
            if constexpr (S == Long) setMoveFlags<Word>(data);
            checkEven<C, S, AE_WRITE|AE_DATA>(ea2);

            if constexpr (S == Long) {

                // The first write shows the flags of the low word, the second one the final flags
                writeM<C, Mode::AL, Word>(ea2, READ<Word, 1>(data));
                setMoveFlags<S>(data);
                writeM<C, Mode::AL, Word>(ea2 + 2, READ<Word>(data));

            } else {

                writeM<C, Mode::AL, S>(ea2, data);
            }

        } else {

            u32 ea2 = queue.irc << 16;
            readExt<C>();
            ea2 |= queue.irc;

            checkEven<C, S, AE_WRITE|AE_DATA>(ea2);
            writeM<C, Mode::AL, S>(ea2, data);
        }

        setMoveFlags<S>(data);
        readExt<C, NO_DEC_PC>();

    } else {

        readOp<C, M, S>(src, &ea, &data);

        if constexpr (C == Core::C68000) {

            // The first extension word is fetched with the flags untouched
            writeBuffer = (u16)READ<Word, S == Long ? 1 : 0>(data);
            u32 ea2 = queue.irc << 16;
            readExt<C>();
            ea2 |= queue.irc;
            readBuffer = queue.irc;

            // A failed fetch of the second extension word shows the flags of the (low word of the) operation
            setMoveFlags<S == Long ? Word : S>(data);
            readExtLast<C>();

            // A failed write shows the final flags
            setMoveFlags<S>(data);
            writeM<C, Mode::AL, S>(ea2, data);

        } else {

            writeOp<C, Mode::AL, S>(dst, data);
            setMoveFlags<S>(data);
        }
    }

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (16, 16,  6,       16, 16,  6,       20, 20,  6)
    CYCLES_AN   (16, 16,  6,       16, 16,  6,       20, 20,  6)
    CYCLES_AI   (20, 20, 10,       20, 20, 10,       28, 28, 10)
    CYCLES_PI   (20, 20, 10,       20, 20, 10,       28, 28, 10)
    CYCLES_PD   (22, 22, 11,       22, 22, 11,       30, 30, 11)
    CYCLES_DI   (24, 24, 11,       24, 24, 11,       32, 32, 11)
    CYCLES_IX   (26, 26, 13,       26, 26, 13,       34, 34, 13)
    CYCLES_AW   (24, 24, 10,       24, 24, 10,       32, 32, 10)
    CYCLES_AL   (28, 28, 10,       28, 28, 10,       36, 36, 10)
    CYCLES_DIPC (24, 24, 11,       24, 24, 11,       32, 32, 11)
    CYCLES_IXPC (26, 26, 13,       26, 26, 13,       34, 34, 13)
    CYCLES_IM   (20, 20,  8,       20, 20,  8,       28, 28, 10)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMovea(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea = 0, data;

    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    try { prefetch<C, POLL>(); } catch (BusError &e) {
        rethrowA<C>(e, dst, SEXT<S>(data));
    }
    writeA(dst, SEXT<S>(data));

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        4,  4,  2)
    CYCLES_AN   ( 4,  4,  2,        4,  4,  2,        4,  4,  2)
    CYCLES_AI   ( 8,  8,  6,        8,  8,  6,       12, 12,  6)
    CYCLES_PI   ( 8,  8,  6,        8,  8,  6,       12, 12,  6)
    CYCLES_PD   (10, 10,  7,       10, 10,  7,       14, 14,  7)
    CYCLES_DI   (12, 12,  7,       12, 12,  7,       16, 16,  7)
    CYCLES_IX   (14, 14,  9,       14, 14,  9,       18, 18,  9)
    CYCLES_AW   (12, 12,  6,       12, 12,  6,       16, 16,  6)
    CYCLES_AL   (16, 16,  6,       16, 16,  6,       20, 20,  6)
    CYCLES_DIPC (12, 12,  7,       12, 12,  7,       16, 16,  7)
    CYCLES_IXPC (14, 14,  9,       14, 14,  9,       18, 18,  9)
    CYCLES_IM   ( 8,  8,  4,        8,  8,  4,       12, 12,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMovecRcRx(u16 opcode)
{
    AVAILABILITY(Core::C68010)
    SUPERVISOR_MODE_ONLY

    SYNC(4);

    auto arg = readI<C, Word>();
    int dst = xxxx____________(arg);

    if (!isValidMovecReg(arg & 0xFFF)) {

        execIllegal<C, I, M, S>(opcode);
        return;
    }

    switch (arg & 0x0FFF) {

        case 0x000: reg.r[dst] = getSFC(); break;
        case 0x001: reg.r[dst] = getDFC(); break;
        case 0x800: reg.r[dst] = getUSP(); break;
        case 0x801: reg.r[dst] = getVBR(); break;
        case 0x002: reg.r[dst] = getCACR(); break;
        case 0x802: reg.r[dst] = getCAAR(); break;
        case 0x803: reg.r[dst] = reg.sr.m ? getSP() : getMSP(); break;
        case 0x804: reg.r[dst] = reg.sr.m ? getISP() : getSP(); break;
        case 0x003: reg.r[dst] = mmu.mmu040.tc; break;
        case 0x004: reg.r[dst] = mmu.mmu040.itt0; break;
        case 0x005: reg.r[dst] = mmu.mmu040.itt1; break;
        case 0x006: reg.r[dst] = mmu.mmu040.dtt0; break;
        case 0x007: reg.r[dst] = mmu.mmu040.dtt1; break;
        case 0x805: reg.r[dst] = mmu.mmu040.mmusr; break;
        case 0x806: reg.r[dst] = mmu.mmu040.urp; break;
        case 0x807: reg.r[dst] = mmu.mmu040.srp; break;
    }

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        0, 12,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMovecRxRc(u16 opcode)
{
    AVAILABILITY(Core::C68010)
    SUPERVISOR_MODE_ONLY

    SYNC(2);

    auto arg = readI<C, Word>();
    int  src = xxxx____________(arg);

    if (!isValidMovecReg(arg & 0xFFF)) {

        execIllegal<C, I, M, S>(opcode);
        return;
    }

    u32  val = readR(src);
    switch (arg & 0x0FFF) {

        case 0x000: setSFC(u8(val)); break;
        case 0x001: setDFC(u8(val)); break;
        case 0x800: setUSP(val); break;
        case 0x801: setVBR(val); break;
        case 0x002: setCACR(val); break;
        case 0x802: setCAAR(val); break;
        case 0x803: reg.sr.m ? setSP(val) : setMSP(val); break;
        case 0x804: reg.sr.m ? setISP(val) : setSP(val); break;
        case 0x003: if (cpuModel != CPUModel::M68EC040) mmu.mmu040.setTC(val); break;
        case 0x004: mmu.mmu040.setITT0(val); break;
        case 0x005: mmu.mmu040.setITT1(val); break;
        case 0x006: mmu.mmu040.setDTT0(val); break;
        case 0x007: mmu.mmu040.setDTT1(val); break;
        case 0x805: mmu.mmu040.mmusr = val; break;
        case 0x806: mmu.mmu040.urp = val; break;
        case 0x807: mmu.mmu040.srp = val; break;
    }

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        0, 10, 12)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMovemEaRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);

    u16 mask;
    u32 ea;

    if constexpr (C == Core::C68000) {

        try { mask = (u16)readI<C, Word>(); } catch (BusError &e) {

            // The stacked PC is not decremented if the mask refill fails
            if constexpr (M == Mode::AI || M == Mode::PI || M == Mode::PD) e.stackFrame.pc -= 2;
            throw;
        }

        try { ea = computeEA<C, M, S>(src); } catch (BusError &e) {

            // The stacked PC is the address of the consumed word
            if constexpr (M == Mode::AW) e.stackFrame.pc -= 2;
            throw;
        }

    } else {

        mask = (u16)readI<C, Word>();
        ea = computeEA<C, M, S>(src);
    }

    // Check for address error
    if (misaligned<C, S>(ea)) {

        // In indexed modes, the 68000 decrements the stacked PC (MAME does not). The 68010 does not
        // decrement it either, and the data input buffer holds the next prefetch (vAmigaTS: addrerr43).
        constexpr bool dec = C == Core::C68000 && compat != Compatibility::MAME;
        constexpr Flags flags = (M == Mode::IX || M == Mode::IXPC) && dec ? AE_DEC_PC : AE_INC_PC;
        if constexpr (C == Core::C68010) readBuffer = queue.irc;
        setFC<M>();
        throw earlyAddressError<C>(makeFrame<flags>(ea));
    }

    // MAME does not read ahead, it reads one word too many at the end. UAE neither reads ahead nor too far.
    constexpr bool readAhead = S == Long && compat != Compatibility::MAME && compat != Compatibility::UAE;
    if constexpr (readAhead) (void)read<C, AddrSpace::DATA, Word>(ea);

    if constexpr (C == Core::C68000) {

        for (int i = 0; i <= 15; i++) {

            // Only proceed if the mask bit is set
            if ((mask & (1 << i)) == 0) continue;

            if constexpr (S == Long) {

                // The upper word is loaded first
                u32 hi = readM<C, M, Word, AE_INC_PC>(ea);
                writeR(i, WRITE<Word, 1>(reg.r[i], hi));
                writeR(i, hi << 16 | readM<C, M, Word, AE_INC_PC>(ea + 2));

            } else {

                writeR(i, SEXT<S>(readM<C, M, S, AE_INC_PC>(ea)));
            }
            ea += S;
        }

        // The 68000 reads one word too many (unless it has read ahead)
        if constexpr (!readAhead) (void)read<C, isPrgMode(M) ? AddrSpace::PROG : AddrSpace::DATA, Word, AE_INC_PC>(ea);
        
        if constexpr (M == Mode::PI) writeA(src, ea);

    } else {

        for (int i = 0; i <= 15; i++) {

            // Only proceed if the mask bit is set
            if ((mask & (1 << i)) == 0) continue;

            writeR(i, SEXT<S>(readM<C, M, S, AE_INC_PC>(ea)));
            ea += S;
        }

        // The word operation reads one word too many after the address register has been updated
        if constexpr (M == Mode::PI) writeA(src, ea);
        if constexpr (S == Word) (void)read<C, AddrSpace::DATA, Word, AE_INC_PC>(ea);
    }

    prefetch<C, POLL>();

    // Each register in the list causes one transfer
    int cnt = std::popcount(mask);
    auto c = (C == Core::C68020 || S == Word) ? 4 * cnt : 8 * cnt;

    //           00  10  20        00    10    20        00    10    20
    //           .b  .b  .b        .w    .w    .w        .l    .l    .l
    CYCLES_AI   ( 0,  0,  0,     12+c, 12+c, 12+c,     12+c, 12+c, 12+c)
    CYCLES_PI   ( 0,  0,  0,     12+c, 12+c,  8+c,     12+c, 12+c,  8+c)
    CYCLES_DI   ( 0,  0,  0,     16+c, 16+c, 13+c,     16+c, 16+c, 13+c)
    CYCLES_IX   ( 0,  0,  0,     18+c, 18+c, 15+c,     18+c, 18+c, 15+c)
    CYCLES_AW   ( 0,  0,  0,     16+c, 16+c, 12+c,     16+c, 16+c, 12+c)
    CYCLES_AL   ( 0,  0,  0,     20+c, 20+c, 12+c,     20+c, 20+c, 12+c)
    CYCLES_DIPC ( 0,  0,  0,     16+c, 16+c,  9+c,     16+c, 16+c,  9+c)
    CYCLES_IXPC ( 0,  0,  0,     18+c, 18+c, 11+c,     18+c, 18+c, 11+c)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMovemRgEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);

    u16 mask;
    try { mask = (u16)readI<C, Word>(); } catch (BusError &e) {

        // MAME does not decrement the stacked PC if the mask refill fails
        if constexpr (C == Core::C68000) {
            e.stackFrame.pc += M == Mode::IX ? -2 : (M == Mode::AI || M == Mode::PI || M == Mode::PD) ? 2 : 0;
        }
        throw;
    }

    if constexpr (M == Mode::PD) {

        // The 68000 and the 68010 (like UAE) write the low word of a long word first
        constexpr Flags wflags = (C != Core::C68020 ? REVERSE : 0) | AE_INC_PC;

        u32 ea = readA(dst);

        for (int i = 15; i >= 0; i--) {

            // Only proceed if the mask bit is set
            if ((mask & (0x8000 >> i)) == 0) continue;

            // Check for address error
            if (misaligned<C, S>(ea)) {

                setFC<M>();
                readBuffer = mask;
                writeBuffer = (u16)READ<Word>(reg.r[i]);
                throw earlyAddressError<C>(makeFrame<AE_INC_PC|AE_WRITE>(U32_SUB(ea, 2)));
            }

            // Write register contents into memory
            ea -= S;

            // If the address register is part of the list, the 68000 and the 68010 store its initial value
            // and the 68020 stores the initial value minus the operand size (vAmigaTS, CPU/Misc/Movem/movem1).
            // The address register is updated after the loop, so it still holds the initial value here.
            u32 value = reg.r[i];
            if constexpr (C == Core::C68020) {
                if (i == 8 + dst) value -= S;
            }
            writeM<C, M, S, wflags>(ea, value);
        }
        writeA(dst, ea);

    } else {

        u32 ea;
        if constexpr (C == Core::C68000) {

            // MAME stacks the address behind the faulting fetch
            if constexpr (M == Mode::AL) {

                ea = (u32)queue.irc << 16;
                readExt<C>();
                ea |= queue.irc;
                try { readExtLast<C>(); } catch (BusError &e) { e.stackFrame.pc += 2; throw; }

            } else {

                try { ea = computeEA<C, M, S>(dst); } catch (BusError &e) { e.stackFrame.pc += (M == Mode::AW) ? 2 : 4; throw; }
            }

        } else {

            ea = computeEA<C, M, S>(dst);
        }

        for(int i = 0; i < 16; i++) {

            // Only proceed if the mask bit is set
            if ((mask & (1 << i)) == 0) continue;

            // Check for address error
            if (misaligned<C, S>(ea)) {

                setFC<M>();
                readBuffer = mask;
                writeBuffer = (u16)READ<Word, S == Long ? 1 : 0>(reg.r[i]);
                throw earlyAddressError<C>(makeFrame<AE_INC_PC|AE_WRITE>(ea));
            }

            // Write register contents into memory
            writeM<C, M, S, AE_INC_PC>(ea, reg.r[i]);
            ea += S;
        }
    }
    prefetch<C, POLL>();

    // Each register in the list causes one transfer
    int cnt = std::popcount(mask);
    auto c = (C == Core::C68020 || S == Word) ? 4 * cnt : 8 * cnt;

    //           00  10  20        00    10    20        00    10    20
    //           .b  .b  .b        .w    .w    .w        .l    .l    .l
    CYCLES_AI   ( 0,  0,  0,      8+c,  8+c,  8+c,      8+c,  8+c,  8+c)
    CYCLES_PD   ( 0,  0,  0,      8+c,  8+c,  4+c,      8+c,  8+c,  4+c)
    CYCLES_DI   ( 0,  0,  0,     12+c, 12+c,  9+c,     12+c, 12+c,  9+c)
    CYCLES_IX   ( 0,  0,  0,     14+c, 14+c, 11+c,     14+c, 14+c, 11+c)
    CYCLES_AW   ( 0,  0,  0,     12+c, 12+c,  8+c,     12+c, 12+c,  8+c)
    CYCLES_AL   ( 0,  0,  0,     16+c, 16+c,  8+c,     16+c, 16+c,  8+c)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMovepDxEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    u32 ea = computeEA<C, M, S>(dst);
    u32 dx = readD(src);

    switch (S) {

        case Long:
        {
            writeM<C, M, Byte, AE_DEC_PC>(ea, READ<Byte, 3>(dx)); ea += 2;
            writeM<C, M, Byte, AE_DEC_PC>(ea, READ<Byte, 2>(dx)); ea += 2;
            [[fallthrough]];
        }
        case Word:
        {
            writeM<C, M, Byte, AE_DEC_PC>(ea, READ<Byte, 1>(dx)); ea += 2;
            writeM<C, M, Byte, AE_DEC_PC>(ea, READ<Byte, 0>(dx));
        }
    }
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DI   ( 0,  0,  0,       16, 16, 11,       24, 24, 17)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMovepEaDx(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea = computeEA<C, M, S>(src);
    u32 dx = 0;

    switch (S) {

        case Long:
        {
            dx = WRITE<Byte, 3>(dx, read<C, AddrSpace::DATA, Byte, AE_DEC_PC>(ea)); ea += 2;
            dx = WRITE<Byte, 2>(dx, read<C, AddrSpace::DATA, Byte, AE_DEC_PC>(ea)); ea += 2;

            // MAME stores the upper word right away
            if constexpr (C == Core::C68000) writeD<Long>(dst, WRITE<Word, 1>(readD(dst), READ<Word, 1>(dx)));
            [[fallthrough]];
        }
        case Word:
        {
            dx = WRITE<Byte, 1>(dx, read<C, AddrSpace::DATA, Byte, AE_DEC_PC>(ea)); ea += 2;
            POLL_IPL;
            dx = WRITE<Byte, 0>(dx, read<C, AddrSpace::DATA, Byte, AE_DEC_PC>(ea));
        }

    }
    writeD<S>(dst, dx);
    prefetch<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DI   ( 0,  0,  0,       16, 16, 12,       24, 24, 18)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveq(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    i8  src = (i8)opcode;
    int dst = ____xxx_________(opcode);

    writeD <Long> (dst, (i32)src);

    setMoveFlags<Byte>(src);

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IM   ( 0,  0,  0,        0,  0,  0,        4,  4,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoves(u16 opcode)
{
    AVAILABILITY(Core::C68010)
    SUPERVISOR_MODE_ONLY

    // The extension word selects the direction and the register
    u16 arg = (u16)readI<C, Word>();

    // Both directions spend the same number of cycles before the access
    [[maybe_unused]] constexpr int delay =
    M == Mode::PI ? 8 :
    M == Mode::AI || M == Mode::PD || M == Mode::IX ? 6 :
    M == Mode::DI || M == Mode::AW || M == Mode::AL ? 4 : 0;

    if (arg & 0x800) {          // Rg -> Ea

        int src = xxxx____________(arg);
        int dst = _____________xxx(opcode);

        auto value = readR<S>(src);

        u32 ea = computeEA<C, M, S>(dst);
        updateAn<M, S>(dst);

        // MOVES An,(An)+ and MOVES An,-(An) store the modified register
        if constexpr (M == Mode::PI || M == Mode::PD) {

            if (src == (dst | 0x8)) {

                int delta = dst == 7 ? (S == Long ? 4 : 2) : S;
                value = M == Mode::PI ? value + delta : value - delta;
            }
        }

        SYNC(delay);

        // Make the DFC register visible on the FC pins
        fcSource = 2;

        // A failing access resets the FC pins after the stack frame has been created
        try { writeM<C, M, S, AE_INC_PC>(ea, value); } catch (const AddressError &) {

            writeBuffer = (u16)READ<Word, S == Long ? 1 : 0>(value);
            fcSource = 0;
            queue.irc = arg;
            throw;

        } catch (const BusError &) {

            fcSource = 0;
            throw;
        }

        // Switch back to the old FC pin values
        fcSource = 0;

        if constexpr (S == Long) {

            if (cpuModel == CPUModel::M68020 || cpuModel == CPUModel::M68EC020) budget.penalty += 2;
        }

    } else {                    // Ea -> Rg

        int src = _____________xxx(opcode);
        int dst = xxxx____________(arg);

        u32 ea;
        try {

            ea = computeEA<C, M, S>(src);
            updateAn<M, S>(src);

        } catch (AddressError &exc) {

            exc.stackFrame.ird = arg;
            throw;
        }

        // Make the SFC register visible on the FC pins
        fcSource = 1;

        SYNC(delay);

        // A failing access resets the FC pins after the stack frame has been created
        u32 data;
        try { data = readM<C, M, S>(ea); } catch (...) {

            fcSource = 0;
            throw;
        }

        if (dst < 8) {
            writeR<S>(dst, data);
        } else {
            writeR<Long>(dst, SEXT<S>(data));
        }

        // Switch back to the old FC pin values
        fcSource = 0;

        if (cpuModel == CPUModel::M68020 || cpuModel == CPUModel::M68EC020) budget.penalty += 2;
    }

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   ( 0, 18,  9,        0, 18,  9,        0, 22,  9)
    CYCLES_PI   ( 0, 18,  9,        0, 18,  9,        0, 22,  9)
    CYCLES_PD   ( 0, 20, 10,        0, 20, 10,        0, 28, 10)
    CYCLES_DI   ( 0, 26, 10,        0, 26, 10,        0, 32, 10)
    CYCLES_IX   ( 0, 30, 12,        0, 30, 12,        0, 36, 12)
    CYCLES_AW   ( 0, 26,  9,        0, 26,  9,        0, 32,  9)
    CYCLES_AL   ( 0, 30,  9,        0, 30,  9,        0, 36,  9)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveCcrRg(u16 opcode)
{
    AVAILABILITY(Core::C68010)

    int dst = _____________xxx(opcode);

    u32 ea, data;
    readOp<C, M, S>(dst, &ea, &data);
    prefetch<C, POLL>();

    writeD <S> (dst, getCCR());

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        0,  4,  4,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveCcrEa(u16 opcode)
{
    AVAILABILITY(Core::C68010)

    int dst = _____________xxx(opcode);

    constexpr int delay = M == Mode::PI ? 4 : (M == Mode::AI || M == Mode::PD) ? 2 : 0;
    if constexpr (delay) SYNC(delay);

    writeStatusEa<C, M, S, AE_WRITE|AE_INC_PC>(dst, getCCR());

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   ( 0,  0,  0,        0, 12,  8,        0,  0,  0)
    CYCLES_PI   ( 0,  0,  0,        0, 12,  8,        0,  0,  0)
    CYCLES_PD   ( 0,  0,  0,        0, 14,  9,        0,  0,  0)
    CYCLES_DI   ( 0,  0,  0,        0, 16,  9,        0,  0,  0)
    CYCLES_IX   ( 0,  0,  0,        0, 18, 11,        0,  0,  0)
    CYCLES_AW   ( 0,  0,  0,        0, 16,  8,        0,  0,  0)
    CYCLES_AL   ( 0,  0,  0,        0, 20,  8,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveToCcr(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);

    u32 ea, data;
    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    SYNC(4);
    setCCR((u8)data);

    readStatusRefill<C>();
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,       12, 12,  4,        0,  0,  0)
    CYCLES_AI   ( 0,  0,  0,       16, 16,  8,        0,  0,  0)
    CYCLES_PI   ( 0,  0,  0,       16, 16,  8,        0,  0,  0)
    CYCLES_PD   ( 0,  0,  0,       18, 18,  9,        0,  0,  0)
    CYCLES_DI   ( 0,  0,  0,       20, 20,  9,        0,  0,  0)
    CYCLES_IX   ( 0,  0,  0,       22, 22, 11,        0,  0,  0)
    CYCLES_AW   ( 0,  0,  0,       20, 20,  8,        0,  0,  0)
    CYCLES_AL   ( 0,  0,  0,       24, 24,  8,        0,  0,  0)
    CYCLES_DIPC ( 0,  0,  0,       20, 20,  9,        0,  0,  0)
    CYCLES_IXPC ( 0,  0,  0,       22, 22, 11,        0,  0,  0)
    CYCLES_IM   ( 0,  0,  0,       16, 16,  6,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveSrRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)
    if constexpr (C != Core::C68000) SUPERVISOR_MODE_ONLY

    int dst = _____________xxx(opcode);

    u32 ea, data;
    readOp<C, M, S>(dst, &ea, &data);
    try { prefetchLate<C, POLL>(); } catch (BusError &e) {
        rethrowD<C, S>(e, dst, getSR());
    }

    if constexpr (C == Core::C68000) SYNC(2);
    writeD<S>(dst, getSR());

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        6,  4,  8,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveSrEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)
    if constexpr (C != Core::C68000) SUPERVISOR_MODE_ONLY

    int dst = _____________xxx(opcode);

    if constexpr (C == Core::C68000) {

        u32 ea, data;
        readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);
        fetchIrc<C>();

        writeOp<C, M, S, POLL | COMMIT_IRD>(dst, ea, getSR());

    } else {

        constexpr int delay = M == Mode::PI ? 4 : (M == Mode::AI || M == Mode::PD) ? 2 : 0;
        if constexpr (delay) SYNC(delay);

        writeStatusEa<C, M, S, AE_WRITE>(dst, getSR());
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        6,  4,  8,        0,  0,  0)
    CYCLES_AI   ( 0,  0,  0,       12, 12, 12,        0,  0,  0)
    CYCLES_PI   ( 0,  0,  0,       12, 12, 12,        0,  0,  0)
    CYCLES_PD   ( 0,  0,  0,       14, 14, 13,        0,  0,  0)
    CYCLES_DI   ( 0,  0,  0,       16, 16, 13,        0,  0,  0)
    CYCLES_IX   ( 0,  0,  0,       18, 18, 15,        0,  0,  0)
    CYCLES_AW   ( 0,  0,  0,       16, 16, 12,        0,  0,  0)
    CYCLES_AL   ( 0,  0,  0,       20, 20, 12,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveToSr(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    SUPERVISOR_MODE_ONLY

    // Trace on change of flow: T0 is sampled before the status register changes
    changeOfFlow<C>();

    int src = _____________xxx(opcode);

    u32 ea, data;
    readOp<C, M, S, STD_AE_FRAME>(src, &ea, &data);

    SYNC(4);
    setSR((u16)data);

    readStatusRefill<C>();
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,       12, 12,  8,        0,  0,  0)
    CYCLES_AI   ( 0,  0,  0,       16, 16, 12,        0,  0,  0)
    CYCLES_PI   ( 0,  0,  0,       16, 16, 12,        0,  0,  0)
    CYCLES_PD   ( 0,  0,  0,       18, 18, 13,        0,  0,  0)
    CYCLES_DI   ( 0,  0,  0,       20, 20, 13,        0,  0,  0)
    CYCLES_IX   ( 0,  0,  0,       22, 22, 15,        0,  0,  0)
    CYCLES_AW   ( 0,  0,  0,       20, 20, 12,        0,  0,  0)
    CYCLES_AL   ( 0,  0,  0,       24, 24, 12,        0,  0,  0)
    CYCLES_DIPC ( 0,  0,  0,       20, 20, 13,        0,  0,  0)
    CYCLES_IXPC ( 0,  0,  0,       22, 22, 15,        0,  0,  0)
    CYCLES_IM   ( 0,  0,  0,       16, 16, 10,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveUspAn(u16 opcode)
{
    AVAILABILITY(Core::C68000)
    SUPERVISOR_MODE_ONLY

    int an = _____________xxx(opcode);

    if constexpr (C >= Core::C68010) SYNC(2);

    try { prefetch<C, POLL>(); } catch (BusError &e) {
        rethrowA<C>(e, an, getUSP());
    }
    writeA(an, getUSP());

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        4,  6,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMoveAnUsp(u16 opcode)
{
    AVAILABILITY(Core::C68000)
    SUPERVISOR_MODE_ONLY

    int an = _____________xxx(opcode);

    if constexpr (C >= Core::C68010) SYNC(2);

    setUSP(readA(an));
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        4,  6,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMuls(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    execMulWord<C, I, M, S>(opcode);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,       54,  32,  27,      0,  0,  0)
    CYCLES_AI   ( 0,  0,  0,       58,  36,  31,      0,  0,  0)
    CYCLES_PI   ( 0,  0,  0,       58,  36,  31,      0,  0,  0)
    CYCLES_PD   ( 0,  0,  0,       60,  38,  32,      0,  0,  0)
    CYCLES_DI   ( 0,  0,  0,       62,  40,  32,      0,  0,  0)
    CYCLES_IX   ( 0,  0,  0,       64,  42,  34,      0,  0,  0)
    CYCLES_AW   ( 0,  0,  0,       62,  40,  31,      0,  0,  0)
    CYCLES_AL   ( 0,  0,  0,       66,  44,  31,      0,  0,  0)
    CYCLES_DIPC ( 0,  0,  0,       62,  40,  32,      0,  0,  0)
    CYCLES_IXPC ( 0,  0,  0,       64,  42,  34,      0,  0,  0)
    CYCLES_IM   ( 0,  0,  0,       58,  36,  29,      0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMulu(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    execMulWord<C, I, M, S>(opcode);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,       54,  30,  27,      0,  0,  0)
    CYCLES_AI   ( 0,  0,  0,       58,  34,  31,      0,  0,  0)
    CYCLES_PI   ( 0,  0,  0,       58,  34,  31,      0,  0,  0)
    CYCLES_PD   ( 0,  0,  0,       60,  36,  32,      0,  0,  0)
    CYCLES_DI   ( 0,  0,  0,       62,  38,  32,      0,  0,  0)
    CYCLES_IX   ( 0,  0,  0,       64,  40,  34,      0,  0,  0)
    CYCLES_AW   ( 0,  0,  0,       62,  38,  31,      0,  0,  0)
    CYCLES_AL   ( 0,  0,  0,       66,  42,  31,      0,  0,  0)
    CYCLES_DIPC ( 0,  0,  0,       62,  38,  32,      0,  0,  0)
    CYCLES_IXPC ( 0,  0,  0,       64,  40,  34,      0,  0,  0)
    CYCLES_IM   ( 0,  0,  0,       58,  34,  29,      0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMulWord(u16 opcode)
{
    u32 ea, data, result;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    readOp<C, M, Word, STD_AE_FRAME>(src, &ea, &data);

    u32 factor = readD<Word>(dst);

    if constexpr (C == Core::C68000) {

        // The low word is cleared before the prefetch (which precedes the multiplication)
        result = I == Instr::MULS ? muls<C>(data, factor) : mulu<C>(data, factor);
        writeD<Word>(dst, 0);
        
        try { prefetchLate<C, POLL>(); } catch (BusError &e) {

            e.stackFrame.pc -=
            (M == Mode::PD) ? 0 :
            (M == Mode::DI || M == Mode::IX || M == Mode::DIPC || M == Mode::IXPC) ? 4 : 2;

            // MAME shows N=0, Z=1, V=0, C=0 in the frame of a failed prefetch
            setMoveFlags<S>(0);
            rethrow<C>(e);
        }

        SYNC((cyclesMul<C, I>(u16(data))));

    } else {

        result = I == Instr::MULS ? muls<C>(data, factor) : mulu<C>(data, factor);
        SYNC((cyclesMul<C, I>(u16(data))));
        prefetch<C, POLL>();
    }

    writeD(dst, result);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMull(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    u64 result;
    u32 ea, data;
    u16 ext = (u16)readI<C, Word>();

    int src = _____________xxx(opcode);
    int dh  = _____________xxx(ext);
    int dl  = _xxx____________(ext);

    readOp<C, M, S>(src, &ea, &data);

    prefetch<C, POLL>();

    switch (____xx__________(ext)) {

        case 0b00: { // Unsigned 32 bit

            result = mullu<C, Word>(data, readD(dl));

            writeD(dl, u32(result));
            break;
        }
        case 0b01: // Unsigned 64 bit

            result = mullu<C, Long>(data, readD(dl));

            // Note: 68040 switched the write-order
            writeD(dl, u32(result));
            writeD(dh, u32(result >> 32));
            break;

        case 0b10: // Signed 32 bit

            result = mulls<C, Word>(data, readD(dl));
            writeD(dl, u32(result));
            break;

        case 0b11: // Signed 64 bit

            result = mulls<C, Long>(data, readD(dl));

            // Note: 68040 switched the write-order
            writeD(dl, u32(result));
            writeD(dh, u32(result >> 32));
            break;
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        0,  0, 43,        0,  0, 43)
    CYCLES_AI   ( 0,  0,  0,        0,  0, 47,        0,  0, 47)
    CYCLES_PI   ( 0,  0,  0,        0,  0, 47,        0,  0, 47)
    CYCLES_PD   ( 0,  0,  0,        0,  0, 48,        0,  0, 48)
    CYCLES_DI   ( 0,  0,  0,        0,  0, 48,        0,  0, 48)
    CYCLES_IX   ( 0,  0,  0,        0,  0, 50,        0,  0, 50)
    CYCLES_AW   ( 0,  0,  0,        0,  0, 47,        0,  0, 47)
    CYCLES_AL   ( 0,  0,  0,        0,  0, 47,        0,  0, 47)
    CYCLES_DIPC ( 0,  0,  0,        0,  0, 48,        0,  0, 48)
    CYCLES_IXPC ( 0,  0,  0,        0,  0, 50,        0,  0, 50)
    CYCLES_IM   ( 0,  0,  0,        0,  0, 47,        0,  0, 47)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execDivs(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    bool divByZero = execDivWord<C, I, M, S>(opcode);

    if (divByZero) {

        CYCLES_68000(38)
        CYCLES_68010(44)
        CYCLES_68020(38)

    } else {

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_DN   ( 0,  0,  0,      158,122, 56,        0,  0,  0)
        CYCLES_AI   ( 0,  0,  0,      162,126, 60,        0,  0,  0)
        CYCLES_PI   ( 0,  0,  0,      162,126, 60,        0,  0,  0)
        CYCLES_PD   ( 0,  0,  0,      164,128, 61,        0,  0,  0)
        CYCLES_DI   ( 0,  0,  0,      166,130, 61,        0,  0,  0)
        CYCLES_IX   ( 0,  0,  0,      168,132, 63,        0,  0,  0)
        CYCLES_AW   ( 0,  0,  0,      166,130, 60,        0,  0,  0)
        CYCLES_AL   ( 0,  0,  0,      170,134, 60,        0,  0,  0)
        CYCLES_DIPC ( 0,  0,  0,      166,130, 61,        0,  0,  0)
        CYCLES_IXPC ( 0,  0,  0,      168,132, 63,        0,  0,  0)
        CYCLES_IM   ( 0,  0,  0,      162,126, 58,        0,  0,  0)
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execDivu(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    bool divByZero = execDivWord<C, I, M, S>(opcode);

    if (divByZero) {

        CYCLES_68000(38)
        CYCLES_68010(44)
        CYCLES_68020(38)

    } else {

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_DN   ( 0,  0,  0,      140,108, 44,       0,  0,  0)
        CYCLES_AI   ( 0,  0,  0,      144,112, 48,       0,  0,  0)
        CYCLES_PI   ( 0,  0,  0,      144,112, 48,       0,  0,  0)
        CYCLES_PD   ( 0,  0,  0,      146,114, 49,       0,  0,  0)
        CYCLES_DI   ( 0,  0,  0,      148,116, 49,       0,  0,  0)
        CYCLES_IX   ( 0,  0,  0,      150,118, 51,       0,  0,  0)
        CYCLES_AW   ( 0,  0,  0,      148,116, 48,       0,  0,  0)
        CYCLES_AL   ( 0,  0,  0,      152,120, 48,       0,  0,  0)
        CYCLES_DIPC ( 0,  0,  0,      148,116, 49,       0,  0,  0)
        CYCLES_IXPC ( 0,  0,  0,      150,118, 51,       0,  0,  0)
        CYCLES_IM   ( 0,  0,  0,      144,112, 46,       0,  0,  0)
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> bool
Moira::execDivWord(u16 opcode)
{
    constexpr bool sign = I == Instr::DIVS;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea = 0, divisor;

    try { readOp<C, M, Word, STD_AE_FRAME>(src, &ea, &divisor); }
    catch (const AddressError &) {

        // Rectify the stack frame (68010 and above). The register has been advanced by readOp already.
        // The DIB still holds the last prefetch.
        if constexpr (C != Core::C68000) {

            constexpr bool noDec = isAbsMode(M) || M == Mode::AI || M == Mode::PI || M == Mode::PD;
            SYNC(2);
            throw AddressError(makeFrame<noDec ? 0 : AE_DEC_PC>(ea));
        }
        throw;
    }

    u32 dividend = readD(dst);

    // Check for division by zero
    if (divisor == 0) {

        if (cpuModel >= CPUModel::M68EC040) {

            // The 68040 only clears the carry flag
            reg.sr.c = 0;

        } else if constexpr (sign) {

            setMoveFlags<S>(0);
            setDivZeroDIVS<C, S>(dividend);

        } else {

            reg.sr.n = NBIT<Long>(dividend);
            reg.sr.z = (dividend & 0xFFFF0000) == 0;
            reg.sr.v = 0;
            reg.sr.c = 0;
            setDivZeroDIVU<C, S>(dividend);
        }

        SYNC(8);
        execException<C>(M68kException::DIVIDE_BY_ZERO);
        return true;
    }

    if constexpr (C == Core::C68000) {

        // The 68000 spends the division cycles before it writes the result
        SYNC((cyclesDiv<C, I>(dividend, (u16)divisor) - 4));
        writeD(dst, sign ? divs<C>(dividend, divisor) : divu<C>(dividend, divisor));
        prefetch<C, POLL>();

    } else {

        // The 68010 spends the division cycles after the prefetch
        writeD(dst, sign ? divs<C>(dividend, divisor) : divu<C>(dividend, divisor));
        prefetch<C, POLL>();
        SYNC((cyclesDiv<C, I>(dividend, (u16)divisor) - 4));
    }

    return false;
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execDivl(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    bool divByZero = execDivLong<C, I, M, S>(opcode);

    if (!divByZero) {

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_DN   ( 0,  0,  0,        0,  0,  0,        0,  0, 84)
        CYCLES_AI   ( 0,  0,  0,        0,  0,  0,        0,  0, 88)
        CYCLES_PI   ( 0,  0,  0,        0,  0,  0,        0,  0, 88)
        CYCLES_PD   ( 0,  0,  0,        0,  0,  0,        0,  0, 89)
        CYCLES_DI   ( 0,  0,  0,        0,  0,  0,        0,  0, 89)
        CYCLES_IX   ( 0,  0,  0,        0,  0,  0,        0,  0, 91)
        CYCLES_AW   ( 0,  0,  0,        0,  0,  0,        0,  0, 88)
        CYCLES_AL   ( 0,  0,  0,        0,  0,  0,        0,  0, 88)
        CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,        0,  0, 89)
        CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,        0,  0, 91)
        CYCLES_IM   ( 0,  0,  0,        0,  0,  0,        0,  0, 88)

    } else {

        CYCLES_68000(38)
        CYCLES_68010(44)
        CYCLES_68020(38)
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> bool
Moira::execDivLong(u16 opcode)
{
    u64 dividend;
    u32 ea, divisor;
    u16 ext = (u16)readI<C, Word>();

    int src = _____________xxx(opcode);
    int dh  = _____________xxx(ext);
    int dl  = _xxx____________(ext);

    readOp<C, M, S>(src, &ea, &divisor);

    if (ext & 0x400) {
        dividend = (u64)readD(dh) << 32 | readD(dl);
    } else {
        dividend = readD(dl);
    }

    if (divisor == 0) {

        if (cpuModel >= CPUModel::M68EC040) {

            reg.sr.c = 0;

        } else if (ext & 0x800) {

            reg.sr.c = 0;
            setDivZeroDIVSL<C, S>(i64(dividend));

        } else {

            reg.sr.v = 1;
            reg.sr.c = 0;
            setDivZeroDIVUL<C, S>(i64(dividend));
        }

        execException<C>(M68kException::DIVIDE_BY_ZERO);
        return true;
    }

    prefetch<C, POLL>();

    std::pair<u32, u32> result;

    switch (____xx__________(ext)) {

        case 0b00: result = divlu<Word>(dividend, divisor); break;
        case 0b01: result = divlu<Long>(dividend, divisor); break;
        case 0b10: result = divls<Word>(dividend, divisor); break;
        case 0b11: result = divls<Long>(dividend, divisor); break;
    }

    if (!reg.sr.v) {

        writeD(dh, result.second);
        writeD(dl, result.first);

    } else {

        // The 32-bit unsigned division (0b00) cannot overflow
        switch (____xx__________(ext)) {

            case 0b01: setUndefinedDIVUL<C, S>(dividend, divisor); break;
            case 0b10: setUndefinedDIVSL<C, Word>(i64(i32(dividend)), divisor); break;
            case 0b11: setUndefinedDIVSL<C, S>(dividend, divisor); break;
        }
    }

    return false;
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execNbcdRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);

    u32 data = readD<Byte>(dst);
    u32 result = bcd<C, I, Byte>(data, 0);

    looping<I>() ? noPrefetch<C>() : prefetchLate<C, POLL>();

    SYNC(2);
    writeD<Byte>(dst, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 6,  6,  6,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execNbcdEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);

    u32 ea, data;
    readOp<C, M, Byte, STD_AE_FRAME>(dst, &ea, &data);

    u32 result = bcd<C, I, Byte>(data, 0);

    if (looping<I>()) {
        
        if constexpr (M == Mode::AI) noPrefetch<C>(4);
        else if constexpr (M == Mode::PI) noPrefetch<C>(4);
        else if constexpr (M == Mode::PD) noPrefetch<C>(4);
        else noPrefetch<C>(2);
    
    } else {
        
        fetchIrc<C, POLL>();
    }

    writeM<C, M, Byte, COMMIT_IRD>(ea, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (12, 12, 10,        0,  0,  0,        0,  0,  0)
    CYCLES_PI   (12, 12, 10,        0,  0,  0,        0,  0,  0)
    CYCLES_PD   (14, 14, 11,        0,  0,  0,        0,  0,  0)
    CYCLES_DI   (16, 16, 11,        0,  0,  0,        0,  0,  0)
    CYCLES_IX   (18, 18, 13,        0,  0,  0,        0,  0,  0)
    CYCLES_AW   (16, 16, 10,        0,  0,  0,        0,  0,  0)
    CYCLES_AL   (20, 20, 10,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execNegRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);
    u32 ea, data;

    readOp<C, M, S>(dst, &ea, &data);

    data = logic<C, I, S>(data);
    try { prefetchLate<C, POLL, (S == Long)>(); } catch (BusError &e) {
        rethrowD<C, S>(e, dst, data);
    }

    if constexpr (S == Long) SYNC(2);
    writeD<S>(dst, data);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        6,  6,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execNegEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);
    u32 ea, data;

    readOp<C, M, S, STD_AE_FRAME>(dst, &ea, &data);

    [[maybe_unused]] u8 ccr = getCCR();
    u32 result = logic<C, I, S>(data);

    // MAME shows the flags of the low word operation if the prefetch of a long operation fails
    try { looping<I>() ? noPrefetch<C>(2) : fetchIrc<C, POLL>(); } catch (BusError &e) {

        // NEGX reads the X flag, which the long operation has modified already
        if constexpr (S == Long) { setCCR(ccr); logic<C, I, Word>(data); }
        rethrow<C>(e);
    }

    writeOp<C, M, S, REVERSE | COMMIT_IRD>(dst, ea, result);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PI   (12, 12,  8,       12, 12,  8,       20, 20,  8)
    CYCLES_PD   (14, 14,  9,       14, 14,  9,       22, 22,  9)
    CYCLES_DI   (16, 16,  9,       16, 16,  9,       24, 24,  9)
    CYCLES_IX   (18, 18, 11,       18, 18, 11,       26, 26, 11)
    CYCLES_AW   (16, 16,  8,       16, 16,  8,       24, 24,  8)
    CYCLES_AL   (20, 20,  8,       20, 20,  8,       28, 28,  8)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execNop(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        4,  4,  2)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPackDn(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    int dx  = _____________xxx(opcode);
    int dy  = ____xxx_________(opcode);

    u16 adj = (u16)readI<C, Word>();
    u32 src = readD(dx) + adj;
    u32 dst = (src >> 4 & 0xF0) | (src & 0x0F);

    writeD<Byte>(dy, dst);
    prefetch<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        0,  0,  6,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPackPd(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    int ax  = _____________xxx(opcode);
    int ay  = ____xxx_________(opcode);

    reg.a[ax]--;
    auto data1 = read<C, AddrSpace::DATA, Byte>(readA(ax));

    u16 adj = (u16)readI<C, Word>();

    reg.a[ax]--;
    auto data2 = read<C, AddrSpace::DATA, Byte>(readA(ax));

    u32 src = (data2 << 8 | data1) + adj;
    u32 dst = (src >> 4 & 0xF0) | (src & 0x0F);

    writeOp<C, M, Byte>(ay, dst);

    prefetch<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_PD   ( 0,  0,  0,        0,  0, 13,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPea(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int src = _____________xxx(opcode);

    u32 ea = computeEA<C, M, Long>(src);

    if (isIdxMode(M)) SYNC(2);

    if constexpr (C == Core::C68000) {

        if (misaligned<C>(reg.sp)) {

            // MAME completes the prefetch before it detects the misaligned push
            U32_DEC(reg.sp, S);
            if constexpr (!isAbsMode(M)) fetchIrc<C>();
            throw earlyAddressError<C>(makeFrame<AE_WRITE|AE_DATA>(reg.sp));
        }

        if constexpr (isAbsMode(M)) {

            try { push<C, Long>(ea); } catch (BusError &e) {

                // UAE stacks the PC two bytes further if the push fails
                if constexpr (compat == Compatibility::UAE) e.stackFrame.pc += 2;
                throw;
            }
            prefetch<C, POLL>();

        } else if constexpr (isIdxMode(M)) {

            POLL_IPL;
            fetchIrc<C>();
            push<C, Long, COMMIT_IRD>(ea);

        } else {

            fetchIrc<C, POLL>();
            push<C, Long, COMMIT_IRD>(ea);
        }

    } else if constexpr (C == Core::C68010) {

        if (misaligned<C>(reg.sp)) {

            // The 68010 completes the prefetch in all modes (even the absolute ones) and
            // stacks the address of the instruction plus two (recorded on real hardware)
            U32_DEC(reg.sp, S);
            writeBuffer = (u16)READ<Word, 1>(ea);
            prefetch<C>();
            throw AddressError(makeFrame<AE_WRITE|AE_DATA>(reg.sp, reg.pc0 + 2));
        }

        if constexpr (isAbsMode(M)) {

            POLL_IPL;
            push<C, Long>(ea);
            prefetch<C>();

        } else {

            fetchIrc<C, POLL>();
            push<C, Long>(ea);
        }

    } else if constexpr (C == Core::C68020) {

        if constexpr (isAbsMode(M)) {

            POLL_IPL;
            push<C, Long>(ea);
            prefetch<C>();

        } else {

            fetchIrc<C, POLL>();
            push<C, Long>(ea);
        }
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   ( 0,  0,  0,        0,  0,  0,       12, 12,  9)
    CYCLES_DI   ( 0,  0,  0,        0,  0,  0,       16, 16, 10)
    CYCLES_IX   ( 0,  0,  0,        0,  0,  0,       20, 20, 12)
    CYCLES_AW   ( 0,  0,  0,        0,  0,  0,       16, 16,  9)
    CYCLES_AL   ( 0,  0,  0,        0,  0,  0,       20, 20,  9)
    CYCLES_DIPC ( 0,  0,  0,        0,  0,  0,       16, 16, 10)
    CYCLES_IXPC ( 0,  0,  0,        0,  0,  0,       20, 20, 12)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execReset(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    SUPERVISOR_MODE_ONLY

    // Disable the MMU
    mmu.mmu030.tc  &= ~(1 << 31);
    mmu.mmu030.tt0 &= ~(1 << 15);
    mmu.mmu030.tt1 &= ~(1 << 15);

    SYNC_68000(128);
    SYNC_68010(126);

    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,      132,130,518)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execRtd(u16 opcode)
{
    AVAILABILITY(Core::C68010)

    checkEvenSP<C, M>();

    u32 newpc = readM<C, M, Long>(reg.sp);
    reg.sp += 4 + i16(queue.irc);

    checkEvenPC<C, AE_PROG>(newpc);

    reg.pc = newpc;
    fullPrefetch<C, POLL>();

    changeOfFlow<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        0, 16, 10)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execRte(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    const bool t0AtStart = C == Core::C68020 && reg.sr.t0 && !reg.sr.t1;

    SUPERVISOR_MODE_ONLY

    u16 newsr = 0;
    u32 newpc = 0;

    if constexpr (C == Core::C68000) {

        readRteFrame68000<C>(newsr, newpc);

    } else if constexpr (C == Core::C68010) {

        if (!readRteFrame68010<C>(newsr, newpc)) return;

    } else {

        if (!readRteFrame68020<C>(newsr, newpc)) return;
    }

    setSR(newsr);

    checkEvenPC<C, AE_PROG>(newpc);

    // A fault on the first fetch stacks the address of the following word
    fullPrefetchAt<C, POLL>(newpc, reg.pc0 + 2);

    // The trace bits are sampled at the start of the instruction (not the ones that RTE loads)
    if (t0AtStart) flags |= State::TRACE_EXC;

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,       20, 24, 20)

    FINALIZE
}

template <Core C> void
Moira::readRteFrame68000(u16 &newsr, u32 &newpc)
{
    if constexpr (compat != Compatibility::UAE) {

        // MAME adjusts the stack pointer after the last read
        newsr = (u16)read<C, AddrSpace::DATA, Word>(reg.sp);
        newpc = (u32)read<C, AddrSpace::DATA, Word>(reg.sp + 2) << 16;
        reg.sp += 6;
        newpc |= read<C, AddrSpace::DATA, Word>(reg.sp - 2);

    } else {

        // UAE adjusts the stack pointer after the status register has been read
        newsr = (u16)read<C, AddrSpace::DATA, Word, UAE_DEC_PC>(reg.sp);
        reg.sp += 6;
        newpc = (u32)read<C, AddrSpace::DATA, Word, UAE_DEC_PC>(reg.sp - 4) << 16;
        newpc |= read<C, AddrSpace::DATA, Word, UAE_DEC_PC>(reg.sp - 2);
    }
}

template <Core C> bool
Moira::readRteFrame68010(u16 &newsr, u32 &newpc)
{
    // Sets the flags that the 68010 stacks with a format error
    auto formatError = [&](bool n) {

        reg.sr.n = n;
        reg.sr.z = 0;
        reg.sr.v = 0;
        reg.sr.c = 1;

        SYNC(4);
    };

    u16 format = (u16)read<C, AddrSpace::DATA, Word>(reg.sp + 6);

    // The 68010 knows the short format (0000) and the long format (1000)
    if (!supportsFrameFormat(cpuModel, format >> 12)) {

        // Recorded on a real 68010 (vAmigaTS, StackFrame/stackframe2): N=0, Z=0, V=0, C=1 for format 0010.
        // UAE sets N from the format word and leaves C alone.
        formatError(NBIT<Word>(format));
        (void)read<C, AddrSpace::DATA, Long>(reg.sp + 2);

        execException(M68kException::FORMAT_ERROR);
        return false;
    }

    newsr = (u16)read<C, AddrSpace::DATA, Word>(reg.sp + 0);
    newpc = read<C, AddrSpace::DATA, Long>(reg.sp + 2);

    if (format >> 12 == 0b0000) {

        reg.sp += 8;
        return true;
    }

    // The 68010 reads the rest of the frame (vAmigaTS: Timing2/RTE/RTE1). UAE skips it.
    (void)read<C, AddrSpace::DATA, Word>(reg.sp + 8); // Special status word
    (void)read<C, AddrSpace::DATA, Long>(reg.sp + 10); // Fault address

    /* The first word of the internal information holds the version number of the processor that
     * wrote the frame. The 68010 raises a format error if it differs from its own version number.
     * Recorded on a real 68010 (vAmigaTS, StackFrame/addrerr37 and addrerr38): The version number
     * is 2, and the stacked status register shows N=0, Z=0, V=0, C=1.
     */
    u16 version = (read<C, AddrSpace::DATA, Word>(reg.sp + 26) >> 10) & 0xF;
    if (version != VERSION_68010) {

        formatError(0);

        execException(M68kException::FORMAT_ERROR);
        return false;
    }

    (void)read<C, AddrSpace::DATA, Word>(reg.sp + 28); // Internal information

    /* Unused, reserved (14), data output buffer (16), unused, reserved (18),
     * data input buffer (20), unused, reserved (22), instruction input buffer (24)
     */
    for (u32 offset : { 14, 16, 18, 20, 22, 24 }) {
        (void)read<C, AddrSpace::DATA, Word>(reg.sp + offset);
    }

    // Internal information (note: the first three long word reads overlap)
    for (u32 offset : { 34, 36, 38, 42, 46, 50, 54 }) {
        (void)read<C, AddrSpace::DATA, Long>(reg.sp + offset);
    }

    reg.sp += 58;
    return true;
}

template <Core C> bool
Moira::readRteFrame68020(u16 &newsr, u32 &newpc)
{
    bool rollback = false;

    while (1) {

        u32 frameBase = reg.sp;

        // Like UAE, read the status register, the program counter and the format word, in this order
        newsr = (u16)read<C, AddrSpace::DATA, Word>(frameBase);
        newpc = read<C, AddrSpace::DATA, Long>(frameBase + 2);
        u16 format = (u16)(read<C, AddrSpace::DATA, Word>(frameBase + 6) >> 12);

        if (!supportsFrameFormat(cpuModel, format)) {

            execException(M68kException::FORMAT_ERROR);
            CYCLES_68020(4)
            return false;
        }

        // Determine the frame size
        u32 size = 0;

        switch (format) {

            case 0b0000: size = 8; break;   // Standard frame
            case 0b0001: size = 8; break;   // Throwaway frame
            case 0b0010: size = 12; break;  // Trap frame
            case 0b0011: size = 12; break;  // Floating-point post-instruction frame
            case 0b0100: size = 16; break;  // Unimplemented floating-point instruction frame
            case 0b0111: size = 60; break;  // Access error frame
            case 0b1001: size = 20; break;  // Coprocessor mid-instruction frame
            case 0b1010: size = 32; break;  // Short bus fault frame
            case 0b1011: size = 92; break;  // Long bus fault frame
        }

        // Like UAE, skip the rest of the frame without reading it
        reg.sp = frameBase + size;

        if (format == 0b1011) {

            // If the handler has not touched the frame, restart the instruction
            if (restart.valid && frameBase == restart.frameSp && newsr == restart.frameSr && newpc == restart.framePC) {

                newpc = restart.pc;
                newsr = restart.preSr;
                rollback = true;
            }
            restart.valid = false;
        }

        // A throwaway frame is followed by another frame
        if (format != 0b0001) break;
        setSR(newsr);
    }

    // Roll back the registers that the restarted instruction has modified
    if (rollback) {

        for (int i = 0; i < 15; i++) {
            if (reg.r[i] == restart.fault.r[i]) reg.r[i] = restart.pre.r[i];
        }

        // The stack pointers (the instruction may have switched modes)
        u32 preUsp = restart.pre.sr.s ? restart.pre.usp : restart.pre.sp;
        u32 preIsp = restart.pre.sr.s ? restart.pre.sp : restart.pre.isp;
        if (reg.sp == restart.entrySsp) reg.sp = preIsp;
        if (reg.usp == restart.entryUsp) reg.usp = preUsp;
    }

    return true;
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execRtm(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    fullPrefetch<C, POLL>();

    changeOfFlow<C>();

    CYCLES_68020(19)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execRtr(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    checkEvenSP<C, M>();

    u16 newccr;
    u32 newpc;

    // The 68010 and 68020 follow UAE in all modes
    if constexpr (C == Core::C68000 && compat != Compatibility::UAE) {

        // Adjust the stack pointer right before the last word is read
        u32 sp = reg.sp;
        newccr = (u16)readM<C, M, Word>(sp);
        u32 hi = read<C, AddrSpace::DATA, Word>(sp + 2);
        
        // Load the condition codes before the last read
        setCCR((u8)newccr);
        reg.sp = sp + 6;
        newpc = hi << 16 | read<C, AddrSpace::DATA, Word>(sp + 4);

    } else {

        newccr = (u16)readM<C, M, Word>(reg.sp);
        reg.sp += 2;
        newpc = read<C, AddrSpace::DATA, Long>(reg.sp);
        reg.sp += 4;
        setCCR((u8)newccr);
    }

    checkEvenPC<C, AE_PROG>(newpc);

    // MAME stacks the old PC if the first fetch at the return address fails
    fullPrefetchAt<C, POLL>(newpc);

    changeOfFlow<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,       20, 20, 14)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execRts(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    checkEvenSP<C, M>();

    u32 newpc = readM<C, M, Long, UAE_DEC_PC>(reg.sp);
    reg.sp += 4;

    checkEvenPC<C, AE_PROG>(newpc);

    // MAME stacks the old PC if the first fetch at the return address fails
    fullPrefetchAt<C, POLL>(newpc);

    changeOfFlow<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,       16, 16, 10)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execSccRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);
    u32 ea, data;

    readOp<C, M, S>(dst, &ea, &data);
    data = cond<I>() ? 0xFF : 0;

    if constexpr (C == Core::C68000) {

        try {
            // If the condition is met, write the register at the end and keep the IRD
            if (data) prefetchLate<C, POLL>(); else prefetch<C, POLL>();
        }
        catch (BusError &e) {

            // Clear the register before the prefetch
            if (!data) writeD<S>(dst, 0);
            rethrow<C>(e);
        }
        if (data) SYNC(2);

    } else if constexpr (C == Core::C68010) {

        try { prefetch<C, POLL>(); } catch (BusError &e) {

            // Clear the register before the prefetch
            if (!data) writeD<S>(dst, 0);
            rethrow<C>(e);
        }

    } else {

        // Write the register after the prefetch
        prefetch<C, POLL>();
    }

    writeD<S>(dst, data);

    //              00     10  20        00  10  20        00  10  20
    //              .b     .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   (data?6:4,  4,  4,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execSccEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);
    u32 ea, data;

    if constexpr (C == Core::C68000) {

        readOp<C, M, Byte, STD_AE_FRAME>(dst, &ea, &data);

        fetchIrc<C, POLL>();
        data = cond<I>() ? 0xFF : 0;
        writeOp<C, M, Byte, COMMIT_IRD>(dst, ea, data);

    } else {

        ea = computeEA<C, M, S>(dst);
        updateAn<M, S>(dst);

        constexpr int delay = M == Mode::PI ? 4 : (M == Mode::AI || M == Mode::PD || M == Mode::IX) ? 2 : 0;
        if constexpr (delay) SYNC(delay);

        fetchIrc<C, POLL>();
        data = cond<I>() ? 0xFF : 0;
        writeOp<C, M, S, COMMIT_IRD>(dst, ea, data);
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (12, 12, 10,        0,  0,  0,        0,  0,  0)
    CYCLES_PI   (12, 12, 10,        0,  0,  0,        0,  0,  0)
    CYCLES_PD   (14, 14, 11,        0,  0,  0,        0,  0,  0)
    CYCLES_DI   (16, 16, 11,        0,  0,  0,        0,  0,  0)
    CYCLES_IX   (18, 18, 13,        0,  0,  0,        0,  0,  0)
    CYCLES_AW   (16, 16, 10,        0,  0,  0,        0,  0,  0)
    CYCLES_AL   (20, 20, 10,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execStop(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    SUPERVISOR_MODE_ONLY

    u16 src;

    if constexpr (C == Core::C68000) {

        src = queue.irc;
        reg.pc += 2;
        SYNC(4);

    } else {

        src = (u16)readI<C, Word>();
    }

    setSR(src);
    flags |= State::STOPPED;
    POLL_IPL;

    reg.pc += 2;

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        4,  4,  8,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execSwap(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int rg  = _____________xxx(opcode);
    u32 dat = readD(rg);

    dat = std::rotl(dat, 16);

    writeD(rg, dat);
    setMoveFlags<Long>(dat);
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        4,  4,  4,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execTasRg(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);

    u32 ea, data;
    readOp<C, M, Byte>(dst, &ea, &data);

    setMoveFlags<Byte>(data);

    data |= 0x80;
    writeD<S>(dst, data);
    prefetch<C, POLL>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  4,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execTasEa(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int dst = _____________xxx(opcode);

    u32 ea, data;

    if constexpr (C == Core::C68000) {

        constexpr Flags tas = AE_INC_PC;

        try { readOp<C, M, Byte, tas>(dst, &ea, &data); } catch (BusError &e) {

            // Only a failing read cycle is special (not a failing extension word fetch)
            if ((e.stackFrame.fc & 3) != 2) {

                if constexpr (compat != Compatibility::UAE) {

                    // MAME completes a failed read like a failed write
                    e.stackFrame.code &= ~0x10;
                    
                    // Set N=0, Z=1, V=0, C=0
                    e.stackFrame.sr = (e.stackFrame.sr & ~0x0F) | 0x04;
                    
                    setMoveFlags<S>(0);
                }
                if constexpr (compat == Compatibility::UAE && M != Mode::PD) {

                    // UAE does not advance the stacked PC for a bus error in the read cycle
                    e.stackFrame.pc -= (M == Mode::DI || M == Mode::IX) ? 4 : 2;
                }
            }
            throw;
        }

        setMoveFlags<Byte>(data);
        data |= 0x80;

        if (!isRegMode(M)) SYNC(2);
        writeOp<C, M, S, tas>(dst, ea, data);
        prefetch<C, POLL>();

    } else {

        constexpr int delay = M == Mode::PI ? 4 : (M == Mode::AI || M == Mode::PD || M == Mode::IX) ? 2 : 0;
        if constexpr (delay) SYNC(delay);

        readOp<C, M, Byte>(dst, &ea, &data);

        setMoveFlags<Byte>(data);
        data |= 0x80;

        writeOp<C, M, S>(dst, ea, data);
        prefetch<C, POLL>();
        SYNC(2);
    }

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_AI   (18, 18, 18,        0,  0,  0,        0,  0,  0)
    CYCLES_PI   (18, 18, 18,        0,  0,  0,        0,  0,  0)
    CYCLES_PD   (20, 20, 20,        0,  0,  0,        0,  0,  0)
    CYCLES_DI   (22, 22, 22,        0,  0,  0,        0,  0,  0)
    CYCLES_IX   (24, 24, 24,        0,  0,  0,        0,  0,  0)
    CYCLES_AW   (22, 22, 22,        0,  0,  0,        0,  0,  0)
    CYCLES_AL   (26, 26, 26,        0,  0,  0,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execTrap(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int nr = ____________xxxx(opcode);

    SYNC(4);
    execException<C>(M68kException::TRAP, nr);

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,        0,  0,  0,       34, 38, 20)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execTrapv(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    if (reg.sr.v) {

        if constexpr (C == Core::C68000 && compat == Compatibility::UAE) {

            try { (void)read<C, AddrSpace::PROG, Word>(reg.pc + 2); } catch (BusError &e) {

                // A bus error at this point is raised after the exception has already been entered
                setSupervisorMode(true);
                clearTraceFlags();
                e.stackFrame.sr = (e.stackFrame.sr | 0x2000) & ~0x8000; // Stacked with S set and T cleared
                e.stackFrame.code = (e.stackFrame.code & ~7) | 6;       // Supervisor program space
                throw;
            }

        } else if constexpr (C == Core::C68000) {

            // Read the word in supervisor mode
            bool s = reg.sr.s; reg.sr.s = true;

            try { (void)read<C, AddrSpace::PROG, Word>(reg.pc + 2); } catch (BusError &e) {

                // A bus error at this point is raised after the exception has already been entered
                reg.sr.s = s;
                setSupervisorMode(true);
                clearTraceFlags();
                e.stackFrame.sr = (e.stackFrame.sr | 0x2000) & ~0x8000; // Stacked with S set and T cleared
                throw;
            }
            reg.sr.s = s;

        } else if constexpr (C == Core::C68010 && compat == Compatibility::UAE) {

            // UAE's 68010 doesn't read the next word. It spends two internal cycles instead.
            SYNC(2);

        } else {

            (void)read<C, AddrSpace::PROG, Word>(reg.pc + 2);
            SYNC(2);
        }

        execException<C>(M68kException::TRAPV);

        // The 68010 takes 40 cycles, as in precise timing mode: It stacks an additional
        // format word (4 cycles) and waits 2 more cycles after reading the next word.
        // Musashi counts 34 cycles, UAE 36.
        [[maybe_unused]] constexpr int c10 =
        compat == Compatibility::MUSASHI ? 34 : compat == Compatibility::UAE ? 36 : 40;

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_IP   ( 0,  0,  0,        0,  0,  0,       34, c10, 20)

    } else {

        prefetch<C, POLL>();

        //           00  10  20        00  10  20        00  10  20
        //           .b  .b  .b        .w  .w  .w        .l  .l  .l
        CYCLES_IP   ( 0,  0,  0,        0,  0,  0,        4,  4,  4)
    }

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execTrapcc(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    switch (opcode & 0b111) {

        case 0b010: (void)readI<C, Word>(); break;
        case 0b011: (void)readI<C, Long>(); break;
    }

    if (cond<I>()) {

        execException<C>(M68kException::TRAPV);
        CYCLES_68020(20);
        return;
    }

    switch (opcode & 0b111) {

        case 0b100: CYCLES_68020(4); break;
        case 0b010: CYCLES_68020(6); break;
        case 0b011: CYCLES_68020(8); break;
    }

    prefetch<C, POLL>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execTst(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int rg = _____________xxx(opcode);

    u32 ea, data;
    readOp<C, M, S, STD_AE_FRAME>(rg, &ea, &data);

    setMoveFlags<S>(data);

    looping<I>() ? noPrefetch<C>(S == Long ? 4 : 2) : prefetch<C, POLL>();
    if (looping<I>() && S == Long) loopModeDelay = 0;

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 4,  4,  2,        4,  4,  2,        4,  4,  2)
    CYCLES_AN   ( 0,  0,  0,        0,  0,  2,        4,  4,  2)
    CYCLES_AI   ( 8,  8,  6,        8,  8,  6,       12, 12,  6)
    CYCLES_PI   ( 8,  8,  6,        8,  8,  6,       12, 12,  6)
    CYCLES_PD   (10, 10,  7,       10, 10,  7,       14, 14,  7)
    CYCLES_DI   (12, 12,  7,       12, 12,  7,       16, 16,  7)
    CYCLES_IX   (14, 14,  9,       14, 14,  9,       18, 18,  9)
    CYCLES_AW   (12, 12,  6,       12, 12,  6,       16, 16,  6)
    CYCLES_AL   (16, 16,  6,       16, 16,  6,       20, 20,  6)
    CYCLES_DIPC ( 0,  0,  7,        0,  0,  7,        0,  0,  7)
    CYCLES_IXPC ( 0,  0,  9,        0,  0,  9,        0,  0,  9)
    CYCLES_IM   ( 0,  0,  6,        0,  0,  6,        0,  0,  6)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execUnlk(u16 opcode)
{
    AVAILABILITY(Core::C68000)

    int an = _____________xxx(opcode);

    checkEven<C, Word, AE_DATA|AE_INC_PC>(readA(an));

    // Move address register to stack pointer
    u32 oldsp = reg.sp;
    reg.sp = readA(an);

    // Update address register
    u32 ea, data;
    try { readOp<C, Mode::AI, Long, AE_DATA|AE_INC_PC|POLL>(7, &ea, &data); } catch (BusError &) {

        reg.sp = oldsp;
        throw;
    }
    writeA(an, data);

    if (an != 7) reg.sp += 4;
    prefetch<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_IP   ( 0,  0,  0,       12, 12,  6,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execUnpkDn(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    int dx  = _____________xxx(opcode);
    int dy  = ____xxx_________(opcode);

    u16 adj = (u16)readI<C, Word>();
    u32 src = readD(dx);

    u32 dst = ((src << 4 & 0x0F00) | (src & 0x000F)) + adj;
    writeD<Word>(dy, dst);

    prefetch<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_DN   ( 0,  0,  0,        0,  0,  8,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execUnpkPd(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    int ax = _____________xxx(opcode);
    int ay = ____xxx_________(opcode);

    u32 ea, data;
    readOp<C, M, Byte>(ax, &ea, &data);

    u16 adj = (u16)readI<C, Word>();
    u32 dst = ((data << 4 & 0x0F00) | (data & 0x000F)) + adj;

    // The result is written as a single (possibly misaligned) word (vAmigaTS, CPU/68020/Misc/unpk1)
    reg.a[ay] -= 2;
    write<C, AddrSpace::DATA, Word>(readA(ay), dst);

    prefetch<C>();

    //           00  10  20        00  10  20        00  10  20
    //           .b  .b  .b        .w  .w  .w        .l  .l  .l
    CYCLES_PD   ( 0,  0,  0,        0,  0, 13,        0,  0,  0)

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCinv(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // Scope 0 is reserved
    if (___________xx___(opcode) == 0) { execLineF<C, I, M, S>(opcode); return; }

    SUPERVISOR_MODE_ONLY

    // The caches of the 68040 are not emulated. Invalidating them has no effect.
    // CINV triggers a trace on change of flow.
    changeOfFlow<C>();

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execCpush(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // Scope 0 is reserved
    if (___________xx___(opcode) == 0) { execLineF<C, I, M, S>(opcode); return; }

    SUPERVISOR_MODE_ONLY

    // The caches of the 68040 are not emulated. Pushing them has no effect.
    // CPUSH triggers a trace on change of flow.
    changeOfFlow<C>();

    prefetch<C, POLL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C> void
Moira::move16(u32 src, u32 dst)
{
    // MOVE16 transfers a complete line. The lower four address bits are ignored.
    src &= ~15;
    dst &= ~15;

    u32 line[4];
    for (int i = 0; i < 4; i++) line[i] = read<C, AddrSpace::DATA, Long>(src + 4 * i);
    for (int i = 0; i < 4; i++) write<C, AddrSpace::DATA, Long>(dst + 4 * i, line[i]);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove16PiPi(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // MOVE16 (Ax)+,(Ay)+
    int ax = _____________xxx(opcode);
    int ay = xxxx____________(queue.irc) & 7;
    readExt<C>();

    move16<C>(readA(ax), readA(ay));

    // If Ax and Ay are the same register, it is incremented only once
    if (ax != ay) writeA(ax, readA(ax) + 16);
    writeA(ay, readA(ay) + 16);

    prefetch<C, POLL>();

    CYCLES_68020(18);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove16PiAl(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // MOVE16 (Ay)+,(xxx).L
    int ay = _____________xxx(opcode);
    u32 dst = readI<C, Long>();

    move16<C>(readA(ay), dst);
    writeA(ay, readA(ay) + 16);

    prefetch<C, POLL>();

    CYCLES_68020(18);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove16AlPi(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // MOVE16 (xxx).L,(Ay)+
    int ay = _____________xxx(opcode);
    u32 src = readI<C, Long>();

    move16<C>(src, readA(ay));
    writeA(ay, readA(ay) + 16);

    prefetch<C, POLL>();

    CYCLES_68020(18);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove16AiAl(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // MOVE16 (Ay),(xxx).L
    int ay = _____________xxx(opcode);
    u32 dst = readI<C, Long>();

    move16<C>(readA(ay), dst);

    prefetch<C, POLL>();

    CYCLES_68020(18);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execMove16AlAi(u16 opcode)
{
    AVAILABILITY(Core::C68020)

    // MOVE16 (xxx).L,(Ay)
    int ay = _____________xxx(opcode);
    u32 src = readI<C, Long>();

    move16<C>(src, readA(ay));

    prefetch<C, POLL>();

    CYCLES_68020(18);
    FINALIZE
}

#include "MoiraExecMMU_cpp.h"
#include "MoiraExecFPU_cpp.h"

#undef AVAILABILITY
#undef SUPERVISOR_MODE_ONLY
#undef FINALIZE
