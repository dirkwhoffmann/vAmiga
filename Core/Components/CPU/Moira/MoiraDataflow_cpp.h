// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

/* Register accessors and the address bus mask.
 *
 * These live here rather than in Moira.cpp so that they stay visible to the
 * per-core translation units, which need to inline them into the instruction
 * handlers. addrMask is evaluated on every memory access.
 */
template <Core C> u32
Moira::addrMask() const
{
    if constexpr (C == Core::C68020) {

        return cpuModel == CPUModel::M68EC020 ? 0x00FFFFFF : 0xFFFFFFFF;

    } else {

        return 0x00FFFFFF;
    }
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

template <Mode M> void
Moira::setFC()
{
    if constexpr (MOIRA_EMULATE_FC) {
        
        fcl = (M == Mode::DIPC || M == Mode::IXPC) ? FC::USER_PROG : FC::USER_DATA;
    }
}

template <Core C, Mode M, Size S, Flags F> u32
Moira::computeEA(u32 n) {

    assert(n < 8);

    u32 result;

    switch ((int)M) {

        case 0:  // Dn
        case 1:  // An
        {
            result = n;
            break;
        }
        case 2:  // (An)
        {
            result = readA(n);
            break;
        }
        case 3:  // (An)+
        {
            result = readA(n);
            break;
        }
        case 4:  // -(An)
        {
            if constexpr ((F & IMPL_DEC) == 0) SYNC(2);
            result = readA(n) - ((n == 7 && S == Byte) ? 2 : S);
            break;
        }
        case 5: // (d,An)
        {
            u32 an = readA(n);
            i16  d = (i16)queue.irc;

            result = U32_ADD(an, d);
            if constexpr ((F & SKIP_LAST_RD) == 0) { readExt<C, F & NO_DEC_PC>(); } else { reg.pc += 2; }
            break;
        }
        case 6: // (d,An,Xi)
        {
            if constexpr (C == Core::C68020) {

                if (queue.irc & 0x100) {
                    result = computeEAfull<C, M, S, F>(readA(n));
                } else {
                    result = computeEAbrief<C, M, S, F>(readA(n));
                }

            } else {

                i8   d = (i8)queue.irc;
                u32 an = readA(n);
                u32 xi = readR((queue.irc >> 12) & 0b1111);

                result = U32_ADD3(an, d, ((queue.irc & 0x800) ? xi : SEXT<Word>(xi)));

                SYNC(2);
                if constexpr ((F & SKIP_LAST_RD) == 0) { readExt<C, F & NO_DEC_PC>(); } else { reg.pc += 2; }
            }
            break;
        }
        case 7: // ABS.W
        {
            result = (i16)queue.irc;
            readBuffer = queue.irc;

            if constexpr ((F & SKIP_LAST_RD) == 0) { readExtLast<C>(); } else { reg.pc += 2; }
            break;
        }
        case 8: // ABS.L
        {
            result = queue.irc << 16;
            readExt<C>();
            result |= queue.irc;
            readBuffer = queue.irc;

            if constexpr ((F & SKIP_LAST_RD) == 0) { readExtLast<C>(); } else { reg.pc += 2; }
            break;
        }
        case 9: // (d,PC)
        {
            i16  d = (i16)queue.irc;

            result = U32_ADD(reg.pc, d);
            if constexpr ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }
            break;
        }
        case 10: // (d,PC,Xi)
        {
            if constexpr (C == Core::C68020) {

                if (queue.irc & 0x100) {
                    result = computeEAfull<C, M, S, F>(reg.pc);
                } else {
                    result = computeEAbrief<C, M, S, F>(reg.pc);
                }

            } else {

                i8   d = (i8)queue.irc;
                u32 xi = readR((queue.irc >> 12) & 0b1111);

                result = U32_ADD3(reg.pc, d, ((queue.irc & 0x800) ? xi : SEXT<Word>(xi)));
                SYNC(2);
                if constexpr ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }
            }
            break;
        }
        case 11: // Im
        {
            result = readI<C, S>();
            break;
        }
        default:
        {
            fatalError;
        }
    }
    return result;
}

template <Core C, Mode M, Size S, Flags F> u32
Moira::computeEAbrief(u32 an)
{
    u32 result;

    //   15 - 12    11   10   09   08   07   06   05   04   03   02   01   00
    // -----------------------------------------------------------------------
    // | REGISTER | LW | SCALE   | 0  | DISPLACEMENT                         |
    // -----------------------------------------------------------------------

    u16 ext   = queue.irc;
    u16 rn    = xxxx____________ (ext);
    u16 lw    = ____x___________ (ext);
    u16 scale = _____xx_________ (ext);
    u16 disp  = ________xxxxxxxx (ext);

    u32 xn = u32(u64(lw ? readR(rn) : SEXT<Word>(readR(rn))) << scale);
    result = U32_ADD3(an, i8(disp), xn);

    SYNC(2);
    if constexpr ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }

    return result;
}

template <Core C, Mode M, Size S, Flags F> u32
Moira::computeEAfull(u32 an)
{
    u32 result;

    //   15 - 12    11   10   09   08   07   06   05   04   03   02   01   00
    // -----------------------------------------------------------------------
    // | REGISTER | LW | SCALE   | 1  | BS | IS | BD SIZE  | 0  | IIS        |
    // -----------------------------------------------------------------------

    u16  ext   = queue.irc;
    u16  rn    = xxxx____________ (ext);
    u16  lw    = ____x___________ (ext);
    u16  scale = _____xx_________ (ext);
    u16  bs    = ________x_______ (ext);
    u16  is    = _________x______ (ext);
    u16  iis   = _____________xxx (ext);

    u32 xn = 0, bd = 0, od = 0;

    // Read extension words
    readExt<C>();
    auto dw = baseDispWords(ext);
    if (dw == 1) bd = SEXT<Word>(readI<C, Word>());
    if (dw == 2) bd = readI<C, Long>();
    auto ow = outerDispWords(ext);
    if (ow == 1) od = SEXT<Word>(readI<C, Word>());
    if (ow == 2) od = readI<C, Long>();

    // Wipe out an if base register is present
    if (bs) an = 0;

    // Check if index is present
    if (!is) xn = (lw ? readR(rn) : SEXT<Word>(readR(rn))) << scale;

    // Compute effective address
    if (iis & 0b100) {
        result = readM<C, M, Long>(an + bd) + xn + od;
    } else if (iis & 0b011) {
        result = readM<C, M, Long>(an + bd + xn) + od;
    } else {
        result = an + bd + xn;
    }

    // Add the number of extra cycles consumed in this addressing mode
    budget.penalty += penaltyCycles<C, M, S>(ext);

    return result;
}

template <Core C, Mode M, Size S, Flags F> void
Moira::readOp(int n, u32 *ea, u32 *result)
{
    switch (M) {

        case Mode::DN: *result = readD<S>(n);   break;
        case Mode::AN: *result = readA<S>(n);   break;
        case Mode::IM: *result = readI<C, S>(); break;

        default:

            // Compute effective address
            *ea = computeEA<C, M, S, F>(n);

            // Emulate -(An) register modification
            updateAnPD<M, S>(n);

            // Read from effective address
            if constexpr (C == Core::C68010) {

                // Behavior of the 68010 as observed on real hardware (see Tests/Unit/StackFrameTests.cpp):
                // A failing word read advances the (An)+ register, a failing long read leaves the -(An)
                // register untouched, and a failing -(An) word read stacks a PC that is 2 lower than on
                // the 68000.
                if constexpr (M == Mode::PI && S == Word) {

                    try { *result = readM<C, M, S, F>(*ea); } catch (const AddressError &) { updateAnPI<M, S>(n); throw; }

                } else if constexpr (M == Mode::PD && S == Long) {

                    try { *result = readM<C, M, S, F>(*ea); } catch (const AddressError &) { undoAnPD<M, S>(n); throw; }

                } else if constexpr (M == Mode::PD && S == Word) {

                    *result = readM<C, M, S, F & ~Flags(AE_INC_PC)>(*ea);

                } else {

                    *result = readM<C, M, S, F>(*ea);
                }

            } else if constexpr (C == Core::C68000) {

                if constexpr ((F & DEC_LATE) && M == Mode::PD && S == Long) {

                    // MAME decrements the register between the two word reads, UAE after both. Hence, a failing
                    // first access (the lower word) leaves the register untouched, and so does any failure in UAE.
                    try { *result = readM<C, M, S, F>(*ea); }
                    catch (const AddressError &) { undoAnPD<M, S>(n); throw; }
                    catch (const BusError &e) { if (compat == Compatibility::UAE || !e.upperWord) undoAnPD<M, S>(n); throw; }

                } else if constexpr (M == Mode::PI && S == Word) {

                    // The register is advanced before a word read. A bus error leaves it advanced (MAME, UAE).
                    // An address error leaves it untouched on a real 68000 (recorded on an A500), but not in MAME.
                    try { *result = readM<C, M, S, F>(*ea); }
                    catch (const AddressError &) { if constexpr (compat == Compatibility::MAME) updateAnPI<M, S>(n); throw; }
                    catch (const BusError &) { updateAnPI<M, S>(n); throw; }

                } else if constexpr (M == Mode::PI && compat != Compatibility::UAE) {

                    // MAME advances the register before a bus error is reported. A long operand
                    // is advanced between the two word accesses.
                    try { *result = readM<C, M, S, F>(*ea); }
                    catch (const BusError &e) { if (S != Long || !e.upperWord) updateAnPI<M, S>(n); throw; }

                } else {

                    *result = readM<C, M, S, F>(*ea);
                }

            } else {

                *result = readM<C, M, S, F>(*ea);
            }

            // Emulate (An)+ register modification
            updateAnPI<M, S>(n);
    }
}

template <Core C, Mode M, Size S, Flags F> void
Moira::writeOp(int n, u32 val)
{
    switch (M) {

        case Mode::DN: if constexpr (F & COMMIT_IRD) commitIrd<C>(); writeD<S>(n, val); break;
        case Mode::AN: if constexpr (F & COMMIT_IRD) commitIrd<C>(); writeA<S>(n, val); break;
        case Mode::IM: fatalError;

        default:

            writeBuffer = u16(READ<Word, S == Long ? 1 : 0>(val));

            // Compute effective address
            u32 ea = computeEA<C, M, S>(n);

            // Emulate -(An) register modification
            updateAnPD<M, S>(n);

            // Emulate (An)+ register modification. The 68020 advances the register before it writes.
            if constexpr (C == Core::C68020) updateAnPI<M, S>(n);

            // Write to effective address
            writeM<C, M, S, F>(ea, val);

            if constexpr (C != Core::C68020) updateAnPI<M, S>(n);
    }
}

template <Core C, Mode M, Size S, Flags F> void
Moira::writeOp(int n, u32 ea, u32 val)
{
    switch (M) {

        case Mode::DN: if constexpr (F & COMMIT_IRD) commitIrd<C>(); writeD<S>(n, val); break;
        case Mode::AN: if constexpr (F & COMMIT_IRD) commitIrd<C>(); writeA<S>(n, val); break;
        case Mode::IM: fatalError;

        default:

            writeBuffer = u16(READ<Word, S == Long ? 1 : 0>(val));

            // Write to effective address
            writeM<C, M, S, F>(ea, val);
    }
}

template <Core C, Mode M, Flags F> void
Moira::readOp64(int n, u32 *ea, u64 *result)
{
    assert(M != Mode::DN && M != Mode::AN && M != Mode::IM);

    // Compute effective address
    *ea = computeEA<C, M, Long, F>(n);

    // Read the first long word
    updateAnPD<M, Long>(n);
    u64 hi = readM<C, M, Long, F>(*ea);
    updateAnPI<M, Long>(n);

    // Read the second long word
    u64 lo = readM<C, M, Long, F>(*ea + 4);
    updateAnPD<M, Long>(n);
    updateAnPI<M, Long>(n);

    *result = hi << 32 | lo;
}

template <Core C, Mode M, Size S, Flags F> void
Moira::writeStatusEa(int n, u16 val)
{
    // Compute effective address
    u32 ea = computeEA<C, M, S>(n);

    // Emulate -(An) register modification
    updateAnPD<M, S>(n);

    prefetch<C>();

    if (misaligned<C>(ea)) {

        writeBuffer = (u16)READ<Word>(val);
        updateAnPI<M, S>(n);
        setFC<M>();
        throw AddressError(makeFrame<F>(ea));
    }

    // Emulate (An)+ register modification (68020)
    if constexpr (C == Core::C68020) updateAnPI<M, S>(n);

    // Write to effective address
    writeM<C, M, S, POLL>(ea, val);

    // Emulate (An)+ register modification (68000, 68010)
    if constexpr (C != Core::C68020) updateAnPI<M, S>(n);
}

template <Core C, Mode M, Flags F> void
Moira::writeOp64(int n, u64 val)
{
    assert(M != Mode::DN && M != Mode::AN && M != Mode::IM);

    // Compute effective address
    u32 ea = computeEA<C, M, Long>(n);

    // Write the first long word
    updateAnPD<M, Long>(n);
    writeM<C, M, Long, F>(ea, u32(val >> 32));
    updateAnPI<M, Long>(n);

    // Write the second long word
    writeM<C, M, Long, F>(ea + 4, u32(val));
    updateAnPD<M, Long>(n);
    updateAnPI<M, Long>(n);
}

// The following definitions are not templated. They are compiled into the
// main translation unit only (see MoiraCore_cpp.h).
#ifdef MOIRA_MAIN_TU

void
Moira::updateAn(Mode M, Size S, int n)
{
    if ((int)M == 3) U32_INC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
    if ((int)M == 4) U32_DEC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

void
Moira::updateAnPI(Mode M, Size S, int n)
{
    if ((int)M == 3) U32_INC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

void
Moira::updateAnPD(Mode M, Size S, int n)
{
    if ((int)M == 4) U32_DEC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

void
Moira::undoAn(Mode M, Size S, int n)
{
    if ((int)M == 3) U32_DEC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
    if ((int)M == 4) U32_INC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

void
Moira::undoAnPI(Mode M, Size S, int n)
{
    if ((int)M == 3) U32_DEC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

void
Moira::undoAnPD(Mode M, Size S, int n)
{
    if ((int)M == 4) U32_INC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

#endif

template <Mode M, Size S> void
Moira::updateAn(int n)
{
    updateAnPI<M, S>(n);
    updateAnPD<M, S>(n);
}

template <Mode M, Size S> void
Moira::updateAnPI(int n)
{
    if constexpr ((int)M == 3) U32_INC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

template <Mode M, Size S> void
Moira::updateAnPD(int n)
{
    if constexpr ((int)M == 4) U32_DEC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

template <Mode M, Size S> void
Moira::undoAn(int n)
{
    undoAnPI<M, S>(n);
    undoAnPD<M, S>(n);
}

template <Mode M, Size S> void
Moira::undoAnPI(int n)
{
    if constexpr ((int)M == 3) U32_DEC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

template <Mode M, Size S> void
Moira::undoAnPD(int n)
{
    if constexpr ((int)M == 4) U32_INC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

template <Core C, Mode M, Size S, Flags F> u32
Moira::readM(u32 addr)
{
    if constexpr (isPrgMode(M)) {
        return read<C, AddrSpace::PROG, S, F>(addr);
    } else {
        return read<C, AddrSpace::DATA, S, F>(addr);
    }
}

template <Core C, Flags F> u32
Moira::read2x16(u32 lo, u32 hi)
{
    if constexpr ((F & REVERSE) && C == Core::C68000) {

        // MAME reads the lower word first (the read buffer keeps it in case the upper word fails)
        u32 result;
        try { result = readBuffer = read16(hi); } catch (BusError &e) { e.upperWord = false; throw; }
        SYNC(4);
        if constexpr (F & POLL) POLL_IPL;
        try { result |= read16(lo) << 16; } catch (BusError &e) { e.upperWord = true; throw; }
        SYNC(2);
        return result;
    }

    u32 result;
    try { result = read16(lo) << 16; } catch (BusError &e) { e.upperWord = true; throw; }
    SYNC(4);
    if constexpr (F & POLL) POLL_IPL;
    try { result |= read16(hi); } catch (BusError &e) { e.upperWord = false; throw; }
    SYNC(2);
    return result;
}

template <Core C, Flags F> void
Moira::write2x16(u32 lo, u32 hi, u32 valHi, u32 valLo)
{
    if constexpr ((F & REVERSE) && C != Core::C68020) {

        try { write16(hi, u16(valLo)); } catch (BusError &e) { e.upperWord = false; throw; }
        SYNC(4);
        if constexpr (F & POLL) POLL_IPL;
        if constexpr (F & COMMIT_IRD) commitIrd<C>();
        try { write16(lo, u16(valHi)); } catch (BusError &e) { e.upperWord = true; throw; }
        SYNC(2);

    } else {

        try { write16(lo, u16(valHi)); } catch (BusError &e) { e.upperWord = true; throw; }
        SYNC(4);
        if constexpr (F & POLL) POLL_IPL;
        if constexpr (F & COMMIT_IRD) commitIrd<C>();
        try { write16(hi, u16(valLo)); } catch (BusError &e) { e.upperWord = false; throw; }
        SYNC(2);
    }
}

template <Core C, Flags F> u32
Moira::read2x8(u32 addr)
{
    u32 result = u32(read8(addr)) << 8;
    SYNC(4);
    if constexpr (F & POLL) POLL_IPL;
    result |= u32(read8(addr + 1));
    SYNC(2);
    return result;
}

template <Core C, Flags F> void
Moira::write2x8(u32 addr, u32 val)
{
    if constexpr ((F & REVERSE) && C != Core::C68020) {

        write8(addr + 1, u8(val));
        SYNC(4);
        if constexpr (F & POLL) POLL_IPL;
        write8(addr, u8(val >> 8));
        SYNC(2);

    } else {

        write8(addr, u8(val >> 8));
        SYNC(4);
        if constexpr (F & POLL) POLL_IPL;
        write8(addr + 1, u8(val));
        SYNC(2);
    }
}

template <Core C, Flags F> u32
Moira::read4x8(u32 addr)
{
    u32 result;

    try {
        result = u32(read8(addr)) << 24;
        SYNC(4);
        result |= u32(read8(addr + 1)) << 16;
        SYNC(4);
    } catch (BusError &e) { e.upperWord = true; throw; }

    try {
        result |= u32(read8(addr + 2)) << 8;
        SYNC(4);
        if constexpr (F & POLL) POLL_IPL;
        result |= u32(read8(addr + 3));
        SYNC(2);
    } catch (BusError &e) { e.upperWord = false; throw; }

    return result;
}

template <Core C, Flags F> void
Moira::write4x8(u32 addr, u32 val)
{
    if constexpr ((F & REVERSE) && C != Core::C68020) {

        write8(addr + 3, u8(val));
        SYNC(4);
        write8(addr + 2, u8(val >> 8));
        SYNC(4);
        write8(addr + 1, u8(val >> 16));
        SYNC(4);
        if constexpr (F & POLL) POLL_IPL;
        write8(addr, u8(val >> 24));
        SYNC(2);

    } else {

        write8(addr, u8(val >> 24));
        SYNC(4);
        write8(addr + 1, u8(val >> 16));
        SYNC(4);
        write8(addr + 2, u8(val >> 8));
        SYNC(4);
        if constexpr (F & POLL) POLL_IPL;
        write8(addr + 3, u8(val));
        SYNC(2);
    }
}

template <Core C, Flags F> u32
Moira::readLongSplit(u32 addr)
{
    /* The addressed port decides how the transfer is carried out. A 32 bit
     * port delivers the longword in a single bus cycle, a 16 bit port needs
     * two and an 8 bit port four (see dsack).
     */
    switch (portSize(dsack(addr))) {

        case 4:
        {
            // A failing single cycle access is reported as an upper word failure (the fault address is ea)
            u32 result;
            try { result = read32(addr); } catch (BusError &e) { e.upperWord = true; throw; }
            if constexpr (F & POLL) POLL_IPL;
            SYNC(2);
            return result;
        }
        case 2:

            return read2x16<C,F>(addr, addr + 2);

        default:

            return read4x8<C,F>(addr);
    }
}

template <Core C, Flags F> void
Moira::writeLongSplit(u32 addr, u32 val)
{
    // See readLongSplit(): the port width decides how the transfer is split
    switch (portSize(dsack(addr))) {

        case 4:

            write32(addr, val);
            if constexpr (F & POLL) POLL_IPL;
            SYNC(2);
            return;

        case 2:

            write2x16<C,F>(addr, addr + 2, READ<Word, 1>(val), READ<Word>(val));
            return;

        default:

            write4x8<C,F>(addr, val);
            return;
    }
}

template <Core C, AddrSpace AS, Size S, Flags F> u32
Moira::read(u32 addr)
{
    // Update function code pins
    u8 fc = AS == AddrSpace::DATA ? FC::USER_DATA : FC::USER_PROG;
    setFC(fc);
    SYNC(2);

    // Check for address errors
    if (misaligned<C, S>(addr)) {
        throw AddressError(makeFrame<F>(addr));
    }

    // Route data accesses through the data cache (68030)
    if constexpr (C == Core::C68020 && AS == AddrSpace::DATA) {
        if (dCache.active) return readDataCache<C, S, F>(addr);
    }

    if constexpr (C == Core::C68000) {

        // Let Moira complete the stack frame if the host signals a bus error
        try { return readBus<C, S, F>(addr); }
        catch (BusError &e) { if (!e.complete) { e.stackFrame = makeFrame<beFrameFlags<F>()>(e.stackFrame.addr); e.complete = true; } throw; }

    } else {

        u32 result = readBus<C, S, F>(addr);

        // The data input buffer of the 68010 holds the last word read by a data access
        if constexpr (C == Core::C68010 && AS == AddrSpace::DATA && S != Byte) readBuffer = u16(result);

        return result;
    }
}

template <Core C, Size S, Flags F> u32
Moira::readBus(u32 addr)
try
{
    if constexpr (MOIRA_REPORT_BUS_ACCESS) willAccessBus(addr, S, false);

    u32 result;

    // Derive the physical address
    addr = translate<C, false, S>(addr, getFC());

    // Check if a watchpoint has been reached
    if ((flags & State::CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        didReachWatchpoint(addr);
    }

    if constexpr (S == Byte) {

        if constexpr (F & POLL) POLL_IPL;
        result = read8(addr & addrMask<C>());
        SYNC(2);
    }

    if constexpr (S == Word) {

        auto a = addr & addrMask<C>();

        // An 8 bit port needs a second bus cycle to deliver a word
        if constexpr (C >= Core::C68020) {

            if (portSize(dsack(a)) == 1) {
                return read2x8<C,F>(a);
            }
        }

        if constexpr (F & POLL) POLL_IPL;
        result = read16(a);
        SYNC(2);
    }

    if constexpr (S == Long) {

        if constexpr (C >= Core::C68020) {

            if ((addr & 3) == 0) {
                result = readLongSplit<C,F>(addr & addrMask<C>());
            } else {
                result = read2x16<C,F>(addr & addrMask<C>(), (addr + 2) & addrMask<C>());
            }

        } else {

            result = read2x16<C,F>(addr & addrMask<C>(), (addr + 2) & addrMask<C>());
        }
    }

    return result;
}
catch (BusError &e)
{
    // Tell the fault handler whether the access was a read or a write (68030 and UAE bus error frames)
    if constexpr (C == Core::C68020) { e.stackFrame.write = false; completeBusError<S, false>(e); }
    throw;
}


template <Core C> void
Moira::readStatusRefill()
{
    u32 addr = C == Core::C68000 ? reg.pc : reg.pc + 2;
    if constexpr (C == Core::C68020) {
        if (iCache.emulated) {
            (void)readInstr<C>(addr);
            return;
        }
    }
    (void)read<C, AddrSpace::PROG, Word>(addr);
}

template <Core C, Flags F> u16
Moira::readInstr(u32 addr)
{
    // Update function code pins
    setFC(FC::USER_PROG);
    SYNC(2);

    // Check for address errors
    if (misaligned<C, Word>(addr)) {
        throw AddressError(makeFrame<F>(addr));
    }

    if constexpr (C == Core::C68020) {

        // The caches are addressed logically. The MMU is only involved on a miss.
        if ((flags & State::CHECK_WP) && debugger.watchpointMatches(addr, Word)) {
            didReachWatchpoint(addr);
        }
        if constexpr (F & POLL) POLL_IPL;

        bool busAccess = true;
        bool fill;
        u32 phys;
        u16 result;

        try {

            if (iCache.emulated) {

                result = iCache.fetch(addr & addrMask<C>(), busAccess, phys);
                fill = busAccess;

            } else {

                // Without a cache, every fetch is a bus access. The 68020 and 68030 fetch longwords.
                phys = translate<C, false, Word>(addr & addrMask<C>(), getFC()) & addrMask<C>();
                result = read16(phys);
                fill = cpuModel < CPUModel::M68EC040 && iCache.trackFetch(addr & addrMask<C>());
            }

        } catch (BusError &e) {

            e.stackFrame.write = false;
            completeBusError<Word, false>(e, true);
            throw;
        }
        // The 020/030 instruction timings already include a cached fetch.
        // Charge the fill of a longword that misses the cache, plus the narrow-port penalty.
        // Without cache emulation, every new longword is a miss.
        if (fill) budget.penalty += 2;
        if (busAccess && portSize(dsack(phys)) != 4) budget.penalty += 4;

        return result;

    } else {

        // Derive the physical address
        addr = translate<C, false, Word>(addr, getFC());

        // Check if a watchpoint has been reached
        if ((flags & State::CHECK_WP) && debugger.watchpointMatches(addr, Word)) {
            didReachWatchpoint(addr);
        }

        if constexpr (F & POLL) POLL_IPL;

        if constexpr (C == Core::C68000) {

            // Let Moira complete the stack frame if the host signals a bus error
            try {
                u16 result = read16(addr & addrMask<C>());
                SYNC(2);
                return result;
            } catch (BusError &e) {
                if (!e.complete) { e.stackFrame = makeFrame<beFrameFlags<F>()>(e.stackFrame.addr); e.complete = true; }
                throw;
            }

        } else {

            u16 result = read16(addr & addrMask<C>());
            SYNC(2);

            return result;
        }
    }
}

template <Core C, Mode M, Size S, Flags F> void
Moira::writeM(u32 addr, u32 val)
{
    if constexpr (isPrgMode(M)) {
        write<C, AddrSpace::PROG, S, F>(addr, val);
    } else {
        write<C, AddrSpace::DATA, S, F>(addr, val);
    }
}

template <Core C, AddrSpace AS, Size S, Flags F> void
Moira::write(u32 addr, u32 val)
{
    // Update function code pins
    u8 fc = AS == AddrSpace::DATA ? FC::USER_DATA : FC::USER_PROG;
    setFC(fc);
    SYNC(2);

    // Check for address errors
    if (misaligned<C, S>(addr)) {
        throw AddressError(makeFrame<F|AE_WRITE>(addr));
    }

    // Keep the data cache coherent (68030)
    if constexpr (C == Core::C68020 && AS == AddrSpace::DATA) {
        if (dCache.active) { writeDataCache<C, S, F>(addr, val); return; }
    }

    if constexpr (C == Core::C68000) {

        // Let Moira complete the stack frame if the host signals a bus error
        try { writeBus<C, S, F>(addr, val); }
        catch (BusError &e) { if (!e.complete) { e.stackFrame = makeFrame<beFrameFlags<F>() | AE_WRITE>(e.stackFrame.addr); e.complete = true; } throw; }

    } else {

        writeBus<C, S, F>(addr, val);
    }
}

template <Core C, Size S, Flags F> void
Moira::writeBus(u32 addr, u32 val, bool *ci)
try
{
    if constexpr (MOIRA_REPORT_BUS_ACCESS) willAccessBus(addr, S, true);

    // Derive the physical address
    addr = translate<C, true, S>(addr, getFC(), ci);

    // Check if a watchpoint has been reached
    if ((flags & State::CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        didReachWatchpoint(addr);
    }

    if constexpr (S == Byte) {

        if constexpr (F & POLL) POLL_IPL;
        if constexpr (F & COMMIT_IRD) commitIrd<C>();
        write8(addr & addrMask<C>(), (u8)val);
        SYNC(2);
    }

    if constexpr (S == Word) {

        auto a = addr & addrMask<C>();

        // An 8 bit port needs a second bus cycle to accept a word
        if constexpr (C >= Core::C68020) {

            if (portSize(dsack(a)) == 1) {
                write2x8<C,F>(a, val);
                return;
            }
        }

        if constexpr (F & POLL) POLL_IPL;
        if constexpr (F & COMMIT_IRD) commitIrd<C>();
        write16(a, (u16)val);
        SYNC(2);
    }

    if constexpr (S == Long) {

        if constexpr (C >= Core::C68020) {

            if ((addr & 3) == 0) {
                writeLongSplit<C,F>(addr & addrMask<C>(), val);
            } else {
                write2x16<C,F>(addr & addrMask<C>(), (addr + 2) & addrMask<C>(),
                                  READ<Word, 1>(val), READ<Word>(val));
            }

        } else {

            write2x16<C,F>(addr & addrMask<C>(), (addr + 2) & addrMask<C>(),
                              READ<Word, 1>(val), READ<Word>(val));
        }
    }
}
catch (BusError &e)
{
    // Tell the fault handler whether the access was a read or a write and what data it carried
    if constexpr (C == Core::C68020) { e.stackFrame.write = true; e.stackFrame.data = val; completeBusError<S, true>(e); }
    throw;
}


//
// Data cache (68030)
//

template <Core C, Size S, Flags F> u32
Moira::readDataCache(u32 addr)
{
    u8 fc = getFC();

    u32 offset = addr & 3;
    bool spans = offset + S > 4;
    u32 v1, v2 = 0;

    // Read the longword (or the two longwords if the access crosses a boundary)
    bool cached;
    try {
        cached = dCache.read(addr & ~3, fc, v1) && (!spans || dCache.read((addr & ~3) + 4, fc, v2));
    } catch (BusError &e) {
        // The line fill has failed
        e.stackFrame.write = false;
        completeBusError<S, false>(e);
        throw;
    }

    // The page is cache-inhibited
    if (!cached) return readBus<C, S, F>(addr);

    // Check if a watchpoint has been reached
    if ((flags & State::CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        didReachWatchpoint(addr);
    }

    if constexpr (F & POLL) POLL_IPL;

    u64 window = u64(v1) << 32 | v2;
    u32 mask = S == Long ? 0xFFFFFFFF : S == Word ? 0xFFFF : 0xFF;
    return u32(window >> (64 - 8 * offset - 8 * S)) & mask;
}

template <Core C, Size S, Flags F> void
Moira::writeDataCache(u32 addr, u32 val)
{
    u8 fc = getFC();
    bool supervisor = fc & 4;

    // The cache is write-through. The bus write also reveals the cache-inhibit attribute.
    bool ci;
    writeBus<C, S, F>(addr, val, &ci);

    // Place the value in a 64 bit window that covers the two longwords an access can touch
    u32 offset = addr & 3;
    u64 mask = (S == Long ? 0xFFFFFFFFULL : S == Word ? 0xFFFFULL : 0xFFULL) << (64 - 8 * offset - 8 * S);
    u64 data = (u64(val) << (64 - 8 * offset - 8 * S)) & mask;

    for (int i = 0; i < 2; i++) {

        u32 m = i == 0 ? u32(mask >> 32) : u32(mask);
        u32 d = i == 0 ? u32(data >> 32) : u32(data);
        if (m) dCache.write((addr & ~3) + 4 * i, supervisor, m, d, ci);
    }
}

template <Core C, Size S> u32
Moira::readIFrame()
{
    // (MAME mode: readI no longer decrements the stacked PC for byte and word operands)
    return readI<C, S>();
}

template <Core C, Size S> u32
Moira::readI()
{
    u32 result;

    switch (S) {

        case Byte:

            result = (u8)queue.irc;
            readExtLast<C>();
            break;

        case Word:

            result = queue.irc;
            readExtLast<C>();
            break;

        case Long:

            result = queue.irc << 16;
            readExt<C>();
            result |= queue.irc;
            readExtLast<C>();
            break;

        default:
            fatalError;
    }
    readBuffer = queue.irc;

    return result;
}

template <Core C, Size S, Flags F> void
Moira::push(u32 val)
{
    reg.sp -= S;
    write<C, AddrSpace::DATA, S, F>(reg.sp, val);
}

template <Core C, Size S, Flags F> u32
Moira::pop()
{
    u32 result = read<C, AddrSpace::DATA, S, F>(reg.sp);
    reg.sp += S;
    return result;
}

// Instruction fetches from odd addresses cause an address error on all CPUs (even the 68020)
template <Core C> bool
Moira::misalignedPC(u32 addr)
{
    if constexpr (MOIRA_EMULATE_ADDRESS_ERROR) {
        return addr & 1;
    } else {
        return false;
    }
}

template <Core C, Size S> bool
Moira::misaligned(u32 addr)
{
    if constexpr (MOIRA_EMULATE_ADDRESS_ERROR && C != Core::C68020 && S != Byte) {
        return addr & 1;
    } else {
        return false;
    }
}

template <Size S, bool W> void
Moira::completeBusError(BusError &e, bool instructionFetch)
{
    /* A host that throws BusError(addr) only provides the fault address. The
     * 68020 and later processors report the cause of the fault in the special status
     * word, which is built here like for a fault detected by the MMU. Faults
     * detected by the MMU and frames supplied by the host are left untouched.
     */
    if (e.complete || cpuModel < CPUModel::M68EC020) return;

    u8 fc = getFC();

    if (cpuModel >= CPUModel::M68EC040) {

        // The 68040 reports the transfer type and the size
        u16 size = S == Byte ? SSW040::SIZ_B : S == Word ? SSW040::SIZ_W : SSW040::SIZ_L;

        e.stackFrame.ssw = size | (fc & 7);
        if (!W) e.stackFrame.ssw |= SSW040::RW;

    } else {

        // A 68020 PC-relative operand uses program space but is a data cycle
        // (MC68020UM 6.2.1). Only an instruction fetch marks the pipe.
        u16 size = S == Byte ? SSW030::SIZ_B : S == Word ? SSW030::SIZ_W : SSW030::SIZ_L;
        bool prog = cpuModel < CPUModel::M68EC030 ? instructionFetch : (fc & 3) == 2;

        e.stackFrame.ssw = (prog ? SSW030::FB | SSW030::RB : SSW030::DF) | size | (fc & 7);
        if (!W) e.stackFrame.ssw |= SSW030::RW;
    }

    e.stackFrame.fc = fc;
    e.complete = true;
}

template <Flags F> StackFrame
Moira::makeFrame(u32 addr, u32 pc, u16 sr, u16 ird)
{
    StackFrame frame;
    u16 read = 0x10;

    // Prepare
    if constexpr (F & AE_WRITE) read = 0;
    if constexpr (F & AE_PROG) setFC(FC::USER_PROG);
    if constexpr (F & AE_DATA) setFC(FC::USER_DATA);

    // Create
    frame.code = (ird & 0xFFE0) | (u16)getFC() | read;
    frame.addr = addr;
    frame.ird = ird;
    frame.sr = sr;
    frame.pc = pc;
    frame.fc = getFC();
    frame.ssw = frame.fc;

    // Special status word of the 68010: R/W (bit 8), DF (bit 12) and IF (bit 13).
    // Real hardware sets DF only if a data read fails. A failing data write sets neither DF nor IF.
    if constexpr (!(F & AE_WRITE)) frame.ssw |= (1 << 8);
    if constexpr (F & (AE_PROG | AE_FETCH)) frame.ssw |= (1 << 13);
    else if constexpr (!(F & AE_WRITE)) frame.ssw |= (1 << 12);

    // Adjust
    if constexpr (F & AE_INC_PC) frame.pc += 2;
    if constexpr (F & AE_DEC_PC) frame.pc -= 2;
    if constexpr (F & AE_INC_A) frame.addr += 2;
    if constexpr (F & AE_DEC_A) frame.addr -= 2;
    if constexpr (F & AE_SET_CB3) frame.code |= (1 << 3);

    return frame;
}

template <Flags F> StackFrame
Moira::makeFrame(u32 addr, u32 pc)
{
    return makeFrame<F>(addr, pc, getSR(), getIRD());
}

template <Flags F> StackFrame
Moira::makeFrame(u32 addr)
{
    return makeFrame<F>(addr, reg.pc, getSR(), getIRD());
}

template <Core C> AddressError
Moira::earlyAddressError(const StackFrame &frame)
{
    // MAME spends two extra cycles if an address error is detected without a bus access
    if constexpr (C == Core::C68000) sync(2);

    return AddressError(frame);
}

template <Core C, Size S, Flags F> void
Moira::checkEven(u32 addr)
{
    if (misaligned<C, S>(addr)) throw earlyAddressError<C>(makeFrame<F>(addr));
}

template <Core C, Flags F> void
Moira::checkEvenPC(u32 addr)
{
    if (misalignedPC<C>(addr)) throw earlyAddressError<C>(makeFrame<F>(addr));
}

template <Core C, Mode M> void
Moira::checkEvenSP()
{
    if (misaligned<C>(reg.sp)) {

        setFC<M>();

        // MAME and UAE (68000) abort the access without a bus cycle
        if constexpr (C != Core::C68000) readBuffer = u16(readM<C, M, Word>(reg.sp & ~1));
        throw earlyAddressError<C>(makeFrame<0>(reg.sp));
    }
}

template <Core C, Flags F> void
Moira::prefetch()
{
    reg.pc += 2;

    queue.ird = queue.irc;
    queue.irc = readInstr<C, F>(reg.pc);
    readBuffer = queue.irc;
}

template <Core C, Flags F, bool ON> void
Moira::fetchIrc()
{
    if constexpr (C == Core::C68000 && ON) {

        // The word waits in the EDB latch (as in the real CPU) until the IRD is replaced
        reg.pc += 2;
        queue.edb = readInstr<C, F>(reg.pc);
        readBuffer = queue.edb;

    } else {

        prefetch<C, F>();
    }
}

template <Core C> void
Moira::commitIrd()
{
    if constexpr (C == Core::C68000) {

        queue.ird = queue.irc;
        queue.irc = queue.edb;
    }
}

template <Core C, Flags F, bool ON> void
Moira::prefetchLate()
{
    fetchIrc<C, F, ON>();
    if constexpr (ON) commitIrd<C>();
}

template <Core C, Flags F> void
Moira::fullPrefetchAt(u32 newpc)
{
    // MAME stacks the old program counter if the first fetch fails
    fullPrefetchAt<C, F>(newpc, reg.pc);
}

template <Core C> void
Moira::fetchFirstAt(u32 newpc, u32 framePc)
{
    reg.pc = newpc;
    iCache.flushLatch();

    try { queue.irc = readInstr<C>(reg.pc); }
    catch (BusError &e) { if constexpr (C == Core::C68000) e.stackFrame.pc = framePc; throw; }
}

template <Core C, Flags F> void
Moira::fullPrefetchAt(u32 newpc, u32 framePc)
{
    fetchFirstAt<C>(newpc, framePc);
    prefetch<C, F>();
}

template <Core C, Flags F, int delay> void
Moira::fullPrefetch()
{
    assert(!misaligned<C>(reg.pc));

    iCache.flushLatch();
    queue.irc = readInstr<C>(reg.pc);
    if (delay) SYNC(delay);
    prefetch<C, F>();
}

template <Core C> void
Moira::noPrefetch(int delay)
{
    assert(flags & State::LOOPING);

    reg.pc += 2;
    std::swap(queue.irc, queue.ird);
    if (delay) SYNC(delay);
}

template <Core C, Flags F> void
Moira::readExt()
{
    assert(!misaligned<C>(reg.pc));

    reg.pc += 2;

    // MAME stacks the address of the consumed extension word if the refill fails (unless NO_DEC_PC is set)
    if constexpr (C == Core::C68000) {
        queue.irc = readInstr<C, (F & NO_DEC_PC) ? 0 : AE_DEC_PC>(reg.pc);
    } else {
        queue.irc = readInstr<C>(reg.pc);
    }
}

template <Core C> void
Moira::readExtLast()
{
    // MAME does not decrement the stacked PC if the refill after the last word of a long operand fails
    if constexpr (C == Core::C68000) {

        assert(!misaligned<C>(reg.pc));
        reg.pc += 2;
       
        queue.irc = readInstr<C>(reg.pc);

    } else {

        readExt<C>();
    }
}


template <Core C, Flags F> void
Moira::jumpToVector(int nr)
{
    u32 vbr = C == Core::C68000 ? 0 : reg.vbr;
    u32 vectorAddr = (vbr & ~0x1) + 4 * nr;
    u32 oldpc = reg.pc;

    // Update the program counter
    if constexpr (C == Core::C68000) {

        // MAME stacks the vector address if a bus error occurs while the vector is fetched
        try { reg.pc = read<C, AddrSpace::DATA, Long>(vectorAddr); }
        catch (BusError &e) { e.stackFrame.pc = vectorAddr; throw; }

    } else {

        reg.pc = read<C, AddrSpace::DATA, Long>(vectorAddr);
    }

    // Check for address error
    if (misaligned<C>(reg.pc)) {

        if (nr == 3) {
            
            throw DoubleFault();
            
        } else if constexpr (C == Core::C68000) {

            u32 target = reg.pc;
            reg.pc = vectorAddr;
            throw AddressError(makeFrame<F|AE_PROG>(target));

        } else {

            queue.irc = readBuffer = u16(reg.pc);
            writeBuffer = u16(4 * nr);
            
            switch (M68kException(nr)) {
                    
                case M68kException::ILLEGAL:
                case M68kException::LINEA:
                case M68kException::LINEF:
                case M68kException::PRIVILEGE:
                    
                    throw AddressError(makeFrame<F|AE_DEC_PC|AE_PROG>(reg.pc, oldpc));
                    
                default:
                    
                    throw AddressError(makeFrame<F|AE_PROG>(reg.pc, oldpc));
            }
            /*
            if (nr == ILLEGAL || nr == LINEA || nr == LINEF || nr == EXC_PRIVILEGE) {
                throw AddressError(makeFrame<F|AE_DEC_PC|AE_PROG>(reg.pc, oldpc));
            } else {
                throw AddressError(makeFrame<F|AE_PROG>(reg.pc, oldpc));
            }
            */
        }
        return;
    }

    // Update the prefetch queue
    if constexpr (C == Core::C68000) {

        // The first fetch also stacks the vector address, the second one the handler address
        try { queue.irc = (u16)read<C, AddrSpace::PROG, Word>(reg.pc); }
        catch (BusError &e) { e.stackFrame.pc = vectorAddr; throw; }
        SYNC(2);
        prefetch<C, POLL>();

    } else {

        queue.irc = (u16)read<C, AddrSpace::PROG, Word>(reg.pc);
        SYNC(2);
        prefetch<C, POLL>();
    }

    // Stop emulation if the exception should be catched
    if (debugger.catchpointMatches(nr)) didReachCatchpoint(u8(nr));

    didJumpToVector(nr, reg.pc - 2);
}

// The following definitions are not templated. They are compiled into the
// main translation unit only (see MoiraCore_cpp.h).
#ifdef MOIRA_MAIN_TU

int
Moira::baseDispWords(u16 ext) const
{
    u16 xx = __________xx____ (ext);

    bool base_disp      = (xx >= 2);
    bool base_disp_long = (xx == 3);

    return base_disp ? (base_disp_long ? 2 : 1) : 0;
}

int
Moira::outerDispWords(u16 ext) const
{
    u16 xx = ______________xx (ext);

    bool outer_disp      = (xx >= 2);
    bool outer_disp_long = (xx == 3);

    return outer_disp ? (outer_disp_long ? 2 : 1) : 0;
}

#endif

template <Core C, Mode M, Size S> int
Moira::penaltyCycles(u16 ext) const
{
    constexpr u8 delay[64] = {

        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  5,  7,  7,  0,  5,  7,  7,  0,  5,  7,  7,  0,  5,  7,  7,
        2,  7,  9,  9,  0,  7,  9,  9,  0,  7,  9,  9,  0,  7,  9,  9,
        6, 11, 13, 13,  0, 11, 13, 13,  0, 11, 13, 13,  0, 11, 13, 13
    };

    if constexpr (C == Core::C68020 && (M == Mode::IX || M == Mode::IXPC)) {

        if (ext & 0x100) return delay[ext & 0x3F];
    }

    return 0;
}

// Explicit template instantiations
template void Moira::fullPrefetch<Core::C68000, POLL>();
