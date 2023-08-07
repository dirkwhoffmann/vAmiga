// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Core C, Mode M, Size S, Flags F> u32
Moira::computeEA(u32 n) {

    assert(n < 8);

    u32 result;

    switch (M) {

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
            if ((F & IMPL_DEC) == 0) SYNC(2);
            result = readA(n) - ((n == 7 && S == Byte) ? 2 : S);
            break;
        }
        case 5: // (d,An)
        {
            u32 an = readA(n);
            i16  d = (i16)queue.irc;

            result = U32_ADD(an, d);
            if ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }
            break;
        }
        case 6: // (d,An,Xi)
        {
            if constexpr (C == C68020) {

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
                if ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }
            }
            break;
        }
        case 7: // ABS.W
        {
            result = (i16)queue.irc;
            readBuffer = queue.irc;

            if ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }
            break;
        }
        case 8: // ABS.L
        {
            result = queue.irc << 16;
            readExt<C>();
            result |= queue.irc;
            readBuffer = queue.irc;

            if ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }
            break;
        }
        case 9: // (d,PC)
        {
            i16  d = (i16)queue.irc;

            result = U32_ADD(reg.pc, d);
            if ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }
            break;
        }
        case 10: // (d,PC,Xi)
        {
            if constexpr (C == C68020) {

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
                if ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }
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
    if ((F & SKIP_LAST_RD) == 0) { readExt<C>(); } else { reg.pc += 2; }

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
    if (dw == 1) bd = SEXT<Word>(readExt<C, Word>());
    if (dw == 2) bd = readExt<C, Long>();
    auto ow = outerDispWords(ext);
    if (ow == 1) od = SEXT<Word>(readExt<C, Word>());
    if (ow == 2) od = readExt<C, Long>();

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
    cp += penaltyCycles<C, M, S>(ext);

    return result;
}

template <Core C, Mode M, Size S, Flags F> void
Moira::readOp(int n, u32 *ea, u32 *result)
{
    switch (M) {

        case MODE_DN: *result = readD<S>(n);   break;
        case MODE_AN: *result = readA<S>(n);   break;
        case MODE_IM: *result = readI<C, S>(); break;

        default:

            // Compute effective address
            *ea = computeEA<C, M, S, F>(n);

            // Emulate -(An) register modification
            updateAnPD<M, S>(n);

            // Read from effective address
            *result = readM<C, M, S, F>(*ea);

            // Emulate (An)+ register modification
            updateAnPI<M, S>(n);
    }
}

template <Mode M, Flags F> Float80
Moira::readFpuOp(int n, FltFormat fmt)
{
    Float80 result;

    if constexpr (M == MODE_DN) return readFpuOpRg<M, F>(n, fmt);
    if constexpr (M == MODE_IM) return readFpuOpIm<M, F>(fmt);

    return readFpuOpEa<M>(n, fmt);
}

template <Mode M, Flags F> Float80
Moira::readFpuOpRg(int n, FltFormat fmt)
{
    // TODO
    return Float80();
}

template <Mode M, Flags F> Float80
Moira::readFpuOpEa(int n, FltFormat fmt)
{
    Float80 result;

    switch (fmt) {

        case FLT_BYTE:
        {
            auto ea = computeEA<C68020, M, Byte>(n);
            auto data = i8(readM<C68020, M, Byte>(ea));
            updateAn<M, Byte>(n);

            result.raw = softfloat::int32_to_floatx80(data);
            break;
        }
        case FLT_WORD:
        {
            auto ea = computeEA<C68020, M, Word>(n);
            auto data = i16(readM<C68020, M, Word>(ea));
            updateAn<M, Word>(n);

            result.raw = softfloat::int32_to_floatx80(data);
            break;
        }
        case FLT_LONG:
        {
            auto ea = computeEA<C68020, M, Long>(n);
            auto data = i32(readM<C68020, M, Long>(ea));
            updateAn<M, Long>(n);

            result.raw = softfloat::int32_to_floatx80(data);
            break;
        }
        case FLT_SINGLE:
        {
            auto ea = computeEA<C68020, M, Long>(n);
            auto data = readM<C68020, M, Long>(ea);
            updateAn<M, Long>(n);

            result.raw = softfloat::float32_to_floatx80(data);
            break;
        }
        case FLT_DOUBLE:
        {
            auto ea = computeEA<C68020, M, Quad>(n);
            u64 data = u64(readM<C68020, M, Long>(ea)) << 32;
            data |= readM<C68020, M, Long>(U32_ADD(ea, 4));
            updateAn<M, Quad>(n);

            result.raw = softfloat::float64_to_floatx80(data);
            break;
        }
        case FLT_EXTENDED:
        {
            auto ea = computeEA<C68020, M, Extended>(n);
            u16 data1 = u16(readM<C68020, M, Word>(ea));
            u32 data2 = readM<C68020, M, Long>(U32_ADD(ea, 4));
            u32 data3 = readM<C68020, M, Long>(U32_ADD(ea, 8));
            updateAn<M, Extended>(n);

            result.raw.high = data1;
            result.raw.low = ((u64)data2 << 32) | (data3 & 0xFFFFFFFF);
            result.normalize();
            break;
        }
        case FLT_PACKED:
        {

            auto ea = computeEA<C68020, M, Quad>(n);
            u16 data1 = u16(readM<C68020, M, Long>(ea));
            u32 data2 = readM<C68020, M, Long>(U32_ADD(ea, 4));
            u32 data3 = readM<C68020, M, Long>(U32_ADD(ea, 8));
            updateAn<M, Extended>(n);

            fpu.unpack(data1, data2, data3, result);
            break;
        }
        default:
            fatalError;
    }
    return result;
}

template <Mode M, Flags F> Float80
Moira::readFpuOpIm(FltFormat fmt)
{
    Float80 result;

    switch (fmt) {

        case FLT_LONG:
        {
            auto ext = readExt<C68020, Long>();
            result.raw = softfloat::int32_to_floatx80(ext);
            break;
        }
        case FLT_SINGLE:
        {
            u32 data = readExt<C68020, Long>();
            result.raw = softfloat::float32_to_floatx80(data);
            break;
        }
        case FLT_EXTENDED:
        {
            u32 high = readExt<C68020, Word>();
            (void)readExt<C68020, Word>();
            u64 low = (u64)readExt<C68020, Long>() << 32;
            low |= readExt<C68020, Long>();
            result.raw.high = u16(high);
            result.raw.low = low;
            result.normalize();
            break;
        }
        case FLT_PACKED:
        {
            u32 dw1 = readExt<C68020, Long>();
            u32 dw2 = readExt<C68020, Long>();
            u32 dw3 = readExt<C68020, Long>();
            printf("            %x %x %x\n", dw1, dw2, dw3);
            fpu.unpack(dw1, dw2, dw3, result);
            break;
        }
        case FLT_WORD:
        {
            auto ext = readExt<C68020, Word>();
            printf("FLT_WORD ext = %x\n", ext);
            result = Float80(ext);
            break;
        }
        case FLT_DOUBLE:
        {
            u64 data = (u64)readExt<C68020, Long>() << 32;
            data |= readExt<C68020, Long>();

            softfloat::float_exception_flags = 0;
            result.raw = softfloat::float64_to_floatx80(data);
            printf("Loaded FLT_DOUBLE: %llx -> %x,%llx (flags = %x)\n", data, result.raw.high, result.raw.low, softfloat::float_exception_flags);
            break;
        }
        case FLT_BYTE:
        {
            auto ext = readExt<C68020, Byte>();
            result = Float80(ext & 0xFF);
            printf("readFpuOpIm.B: ext = %x %f\n", ext, result.asDouble());
            break;
        }
        default:
            fatalError;
    }

    return result;
}

template <Core C, Mode M, Size S, Flags F> void
Moira::writeOp(int n, u32 val)
{
    switch (M) {

        case MODE_DN: writeD<S>(n, val); break;
        case MODE_AN: writeA<S>(n, val); break;
        case MODE_IM: fatalError;

        default:

            writeBuffer = (S == Long) ? u16(val >> 16) : u16(val & 0xFFFF);

            // Compute effective address
            u32 ea = computeEA<C, M, S>(n);

            // Emulate -(An) register modification
            updateAnPD<M, S>(n);

            // Write to effective address
            writeM<C, M, S, F>(ea, val);

            // Emulate (An)+ register modification
            updateAnPI<M, S>(n);
    }
}

template <Mode M, Flags F> void
Moira::writeFpuOp(int n, u32 ea, FPUReg &reg, FltFormat fmt, int k)
{
    switch (fmt) {

        case FLT_BYTE:
        {
            u8 data = reg.asByte();
            writeM<C68020, M, Byte>(ea, data);
            updateAn<M, Byte>(n);
            break;
        }
        case FLT_WORD:
        {
            u16 data = reg.asWord();
            writeM<C68020, M, Word>(ea, data);
            updateAn<M, Word>(n);
            break;
        }
        case FLT_LONG:
        {
            u32 data = reg.asLong();
            writeM<C68020, M, Long>(ea, data);
            updateAn<M, Long>(n);
            break;
        }
        case FLT_SINGLE:
        {
            auto data = reg.asSingle();
            writeM<C68020, M, Long>(ea, data);
            updateAn<M, Long>(n);
            break;
        }
        case FLT_DOUBLE:
        {
            u64 data = reg.asDouble();
            writeM<C68020, M, Long>(ea, u32(data >> 32));
            writeM<C68020, M, Long>(U32_ADD(ea, Long), u32(data));
            updateAn<M, Quad>(n);
            break;
        }
        case FLT_EXTENDED:
        {
            Float80 data;

            if constexpr (F & FPU_FMOVEM) {
                data = reg.val;
            } else {
                data = reg.asExtended();
            }
            
            writeM<C68020, M, Word>(ea, u32(data.raw.high));
            writeM<C68020, M, Word>(U32_ADD(ea, 2), u32(0));
            writeM<C68020, M, Long>(U32_ADD(ea, 4), u32(data.raw.low >> 32));
            writeM<C68020, M, Long>(U32_ADD(ea, 8), u32(data.raw.low));
            updateAn<M, Extended>(n);
            break;
        }
        case FLT_PACKED:
        {
            Packed data = reg.asPacked(k);
            printf("Packed = %x,%x,%x\n", data.data[0], data.data[1], data.data[2]);
            auto ea = computeEA<C68020, M, Extended>(n);
            writeM<C68020, M, Long>(ea, data.data[0]);
            writeM<C68020, M, Long>(U32_ADD(ea, 4), data.data[1]);
            writeM<C68020, M, Long>(U32_ADD(ea, 8), data.data[2]);
            updateAn<M, Extended>(n);
            break;
        }

        default:
            fatalError;
    }
}

template <Core C, Mode M, Size S, Flags F> void
Moira::writeOp(int n, u32 ea, u32 val)
{
    switch (M) {

        case MODE_DN: writeD<S>(n, val); break;
        case MODE_AN: writeA<S>(n, val); break;
        case MODE_IM: fatalError;

        default:

            writeBuffer = (S == Long) ? u16(val >> 16) : u16(val & 0xFFFF);

            // Write to effective address
            writeM<C, M, S, F>(ea, val);
    }
}

template <Mode M, Size S> void
Moira::updateAn(int n)
{
    updateAnPI<M, S>(n);
    updateAnPD<M, S>(n);
}

template <Mode M, Size S> void
Moira::updateAnPI(int n)
{
    if constexpr (M == 3) U32_INC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

template <Mode M, Size S> void
Moira::updateAnPD(int n)
{
    if constexpr (M == 4) U32_DEC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
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
    if constexpr (M == 3) U32_DEC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

template <Mode M, Size S> void
Moira::undoAnPD(int n)
{
    if constexpr (M == 4) U32_INC(reg.a[n], (n == 7 && S == Byte) ? 2 : S);
}

template <Core C, Mode M, Size S, Flags F> u32
Moira::readM(u32 addr)
{
    if constexpr (isPrgMode(M)) {
        return read<C, MEM_PROG, S, F>(addr);
    } else {
        return read<C, MEM_DATA, S, F>(addr);
    }
}

template <Core C, MemSpace MS, Size S, Flags F> u32
Moira::read(u32 addr)
{
    u32 result;

    // Update function code pins
    setFC(MS == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
    SYNC(2);

    // Check for address errors
    if (misaligned<C, S>(addr)) {
        throw AddressError(makeFrame<F>(addr));
    }

    // Check if a watchpoint has been reached
    if ((flags & CPU_CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        watchpointReached(addr);
    }

    if constexpr (S == Byte) {

        if (F & POLL) POLL_IPL;
        result = read8(addr & addrMask<C>());
        SYNC(2);
    }

    if constexpr (S == Word) {

        if (F & POLL) POLL_IPL;
        result = read16(addr & addrMask<C>());
        SYNC(2);
    }

    if constexpr (S == Long) {

        result = read16(addr & addrMask<C>()) << 16;
        SYNC(4);
        if (F & POLL) POLL_IPL;
        result |= read16((addr + 2) & addrMask<C>());
        SYNC(2);
    }

    return result;
}

template <Core C, Mode M, Size S, Flags F> void
Moira::writeM(u32 addr, u32 val)
{
    if constexpr (isPrgMode(M)) {
        write<C, MEM_PROG, S, F>(addr, val);
    } else {
        write<C, MEM_DATA, S, F>(addr, val);
    }
}

template <Core C, MemSpace MS, Size S, Flags F> void
Moira::write(u32 addr, u32 val)
{
    // Update function code pins
    setFC(MS == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
    SYNC(2);

    // Check for address errors
    if (misaligned<C, S>(addr)) {
        throw AddressError(makeFrame<F|AE_WRITE>(addr));
    }

    // Check if a watchpoint has been reached
    if ((flags & CPU_CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        watchpointReached(addr);
    }

    if constexpr (S == Byte) {

        if (F & POLL) POLL_IPL;
        write8(addr & addrMask<C>(), (u8)val);
        SYNC(2);
    }

    if constexpr (S == Word) {

        if (F & POLL) POLL_IPL;
        write16(addr & addrMask<C>(), (u16)val);
        SYNC(2);
    }

    if constexpr (S == Long) {

        if (F & REVERSE) {

            write16((addr + 2) & addrMask<C>(), u16(val & 0xFFFF));
            SYNC(4);
            if (F & POLL) POLL_IPL;
            write16(addr & addrMask<C>(), u16(val >> 16));
            SYNC(2);

        } else {

            write16(addr & addrMask<C>(), u16(val >> 16));
            SYNC(4);
            if (F & POLL) POLL_IPL;
            write16((addr + 2) & addrMask<C>(), u16(val & 0xFFFF));
            SYNC(2);
        }
    }
}

template <Core C, Size S> u32
Moira::readI()
{
    u32 result;

    switch (S) {

        case Byte:

            result = (u8)queue.irc;
            readExt<C>();
            break;

        case Word:

            result = queue.irc;
            readExt<C>();
            break;

        case Long:

            result = queue.irc << 16;
            readExt<C>();
            result |= queue.irc;
            readExt<C>();
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
    write<C, MEM_DATA, S, F>(reg.sp, val);
}

template <Core C, Size S, Flags F> u32
Moira::pop()
{
    u32 result = read<C, MEM_DATA, S, F>(reg.sp);
    reg.sp += S;
    return result;
}

template <Core C, Size S> bool
Moira::misaligned(u32 addr)
{
    if constexpr (EMULATE_ADDRESS_ERROR && C != C68020 && S != Byte) {
        return addr & 1;
    } else {
        return false;
    }
}

template <Flags F> StackFrame
Moira::makeFrame(u32 addr, u32 pc, u16 sr, u16 ird)
{
    StackFrame frame;
    u16 read = 0x10;

    // Prepare
    if constexpr (F & AE_WRITE) read = 0;
    if constexpr (F & AE_PROG) setFC(FC_USER_PROG);
    if constexpr (F & AE_DATA) setFC(FC_USER_DATA);

    // Create
    frame.code = (ird & 0xFFE0) | (u16)readFC() | read;
    frame.addr = addr;
    frame.ird = ird;
    frame.sr = sr;
    frame.pc = pc;
    frame.fc = readFC();
    frame.ssw = frame.fc;

    // Adjust
    if constexpr (F & AE_INC_PC) frame.pc += 2;
    if constexpr (F & AE_DEC_PC) frame.pc -= 2;
    if constexpr (F & AE_INC_A) frame.addr += 2;
    if constexpr (F & AE_DEC_A) frame.addr -= 2;
    if constexpr (F & AE_SET_CB3) frame.code |= (1 << 3);
    if constexpr (F & AE_SET_RW) frame.ssw |= (1 << 8);
    if constexpr (F & AE_SET_DF) frame.ssw |= (1 << 12);
    if constexpr (F & AE_SET_IF) frame.ssw |= (1 << 13);

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
    return makeFrame<F>(addr, getPC(), getSR(), getIRD());
}

template <Core C, Flags F> void
Moira::prefetch()
{
    /* Whereas pc is a moving target (it moves forward while an instruction is
     * being processed, pc0 stays stable throughout the entire execution of
     * an instruction. It always points to the start address of the currently
     * executed instruction.
     */
    reg.pc0 = reg.pc;

    queue.ird = queue.irc;
    queue.irc = (u16)read<C, MEM_PROG, Word, F>(reg.pc + 2);
    readBuffer = queue.irc;
}

template <Core C, Flags F, int delay> void
Moira::fullPrefetch()
{
    assert(!misaligned<C>(reg.pc));

    queue.irc = (u16)read<C, MEM_PROG, Word>(reg.pc);
    if (delay) SYNC(delay);
    prefetch<C, F>();
}

template <Core C> void
Moira::noPrefetch(int delay)
{
    assert(flags & CPU_IS_LOOPING);

    reg.pc0 = reg.pc;
    std::swap(queue.irc, queue.ird);
    if (delay) SYNC(delay);
}

template <Core C> void
Moira::readExt()
{
    assert(!misaligned<C>(reg.pc));

    reg.pc += 2;
    queue.irc = (u16)read<C, MEM_PROG, Word>(reg.pc);
}

template <Core C, Size S> u32
Moira::readExt()
{
    u32 result = queue.irc;
    readExt<C>();

    if constexpr (S == Long) {

        result = result << 16 | queue.irc;
        readExt<C>();
    }

    return result;
}

template <Core C, Flags F> void
Moira::jumpToVector(int nr)
{
    u32 vbr = C == C68000 ? 0 : reg.vbr;
    u32 vectorAddr = (vbr & ~0x1) + 4 * nr;
    u32 oldpc = reg.pc;

    // Update the program counter
    reg.pc = read<C, MEM_DATA, Long>(vectorAddr);

    // Check for address error
    if (misaligned<C>(reg.pc)) {

        if (nr == 3) {
            
            throw DoubleFault();
            
        } else if (C == C68000) {

            throw AddressError(makeFrame<F|AE_PROG>(reg.pc, vectorAddr));

        } else {

            queue.irc = readBuffer = u16(reg.pc);
            writeBuffer = u16(4 * nr);
            if (nr == EXC_ILLEGAL || nr == EXC_LINEA || nr == EXC_LINEF || nr == EXC_PRIVILEGE) {
                throw AddressError(makeFrame<F|AE_DEC_PC|AE_PROG|AE_SET_RW|AE_SET_IF>(reg.pc, oldpc));
            } else {
                throw AddressError(makeFrame<F|AE_PROG|AE_SET_RW|AE_SET_IF>(reg.pc, oldpc));
            }
        }
        return;
    }

    // Update the prefetch queue
    queue.irc = (u16)read<C, MEM_PROG, Word>(reg.pc);
    SYNC(2);
    prefetch<C, POLL>();

    // Stop emulation if the exception should be catched
    if (debugger.catchpointMatches(nr)) catchpointReached(u8(nr));

    didJumpToVector(nr, reg.pc);
}

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

template <Core C, Mode M, Size S> int
Moira::penaltyCycles(u16 ext) const
{
    constexpr u8 delay[64] = {

        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  5,  7,  7,  0,  5,  7,  7,  0,  5,  7,  7,  0,  5,  7,  7,
        2,  7,  9,  9,  0,  7,  9,  9,  0,  7,  9,  9,  0,  7,  9,  9,
        6, 11, 13, 13,  0, 11, 13, 13,  0, 11, 13, 13,  0, 11, 13, 13
    };

    if constexpr (C == C68020 && (M == MODE_IX || M == MODE_IXPC)) {

        if (ext & 0x100) return delay[ext & 0x3F];
    }

    return 0;
}

// Explicit template instantiations
template void Moira::fullPrefetch<C68000, POLL>();
