// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

template<Mode M, Size S, Flags F> bool
Moira::readOp(int n, u32 &ea, u32 &result)
{
    switch (M) {
            
        // Handle non-memory modes
        case MODE_DN: result = readD<S>(n); return true;
        case MODE_AN: result = readA<S>(n); return true;
        case MODE_IM: result = readI<S>();  return true;
            
        default:
            
            // Compute effective address
            ea = computeEA<M,S,F>(n);

            // Read from effective address
            bool error; result = readM<M,S,F>(ea, error);

            // Emulate -(An) register modification
            updateAnPD<M,S>(n);

            // Exit if an address error has occurred
            if (error) return false;

            // Emulate (An)+ register modification
            updateAnPI<M,S>(n);
            
            return !error;
    }
}

template<Mode M, Size S, Flags F> bool
Moira::writeOp(int n, u32 val)
{
    switch (M) {
            
        // Handle non-memory modes
        case MODE_DN: writeD<S>(n, val); return true;
        case MODE_AN: writeA<S>(n, val); return true;
        case MODE_IM: fatalError;

        default:
            
            // Compute effective address
            u32 ea = computeEA<M,S>(n);
            
            // Write to effective address
            bool error; writeM <M,S,F> (ea, val, error);
            
            // Emulate -(An) register modification
            updateAnPD<M,S>(n);
            
            // Early exit in case of an address error
            if (error) return false;
            
            // Emulate (An)+ register modification
            updateAnPI<M,S>(n);
            
            return !error;
    }
}

template<Mode M, Size S, Flags F> void
Moira::writeOp(int n, u32 ea, u32 val)
{
    switch (M) {
            
        // Handle non-memory modes
        case MODE_DN: writeD <S> (n, val); return;
        case MODE_AN: writeA <S> (n, val); return;
        case MODE_IM: fatalError;

        default:
            writeM <M,S,F> (ea, val);
    }
}

template<Mode M, Size S, Flags F> u32
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
            if ((F & IMPLICIT_DECR) == 0) sync(2);
            result = readA(n) - ((n == 7 && S == Byte) ? 2 : S);
            break;
        }
        case 5: // (d,An)
        {
            u32 an = readA(n);
            i16  d = (i16)queue.irc;
            
            result = U32_ADD(an, d);
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 6: // (d,An,Xi)
        {
            i8   d = (i8)queue.irc;
            u32 an = readA(n);
            u32 xi = readR((queue.irc >> 12) & 0b1111);

            result = U32_ADD3(an, d, ((queue.irc & 0x800) ? xi : SEXT<Word>(xi)));

            sync(2);
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 7: // ABS.W
        {
            result = (i16)queue.irc;
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 8: // ABS.L
        {
            result = queue.irc << 16;
            readExt();
            result |= queue.irc;
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 9: // (d,PC)
        {
            i16  d = (i16)queue.irc;

            result = U32_ADD(reg.pc, d);
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 10: // (d,PC,Xi)
        {
            i8   d = (i8)queue.irc;
            u32 xi = readR((queue.irc >> 12) & 0b1111);
            
            result = U32_ADD3(reg.pc, d, ((queue.irc & 0x800) ? xi : SEXT<Word>(xi)));
            sync(2);
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 11: // Im
        {
            result = readI<S>();
            break;
        }
        default:
        {
            fatalError;
        }
    }
    return result;
}

template<Mode M, Size S> void
Moira::updateAnPD(int n)
{
    // -(An)
    if constexpr (M == 4) reg.a[n] -= (n == 7 && S == Byte) ? 2 : S;
}

template<Mode M, Size S> void
Moira::undoAnPD(int n)
{
    // -(An)
    if constexpr (M == 4) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;
}

template<Mode M, Size S> void
Moira::updateAnPI(int n)
{
    // (An)+
    if constexpr (M == 3) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;

}

template<Mode M, Size S> void
Moira::updateAn(int n)
{
    // (An)+
    if constexpr (M == 3) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;

    // -(An)
    if constexpr (M == 4) reg.a[n] -= (n == 7 && S == Byte) ? 2 : S;
}

template<Mode M, Size S, Flags F> u32
Moira::readM(u32 addr, bool &error)
{
    if (isPrgMode(M)) {
        return readMS <MEM_PROG, S, F> (addr, error);
    } else {
        return readMS <MEM_DATA, S, F> (addr, error);
    }
}

template<Mode M, Size S, Flags F> u32
Moira::readM(u32 addr)
{
    if (isPrgMode(M)) {
        return readMS <MEM_PROG, S, F> (addr);
    } else {
        return readMS <MEM_DATA, S, F> (addr);
    }
}

template<MemSpace MS, Size S, Flags F> u32
Moira::readMS(u32 addr, bool &error)
{
    // Check for address errors
    if ((error = misaligned<S>(addr)) == true) {
        
        setFC(MS == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
        execAddressError(makeFrame<F>(addr), 2);
        return 0;
    }
    
    return readMS <MS,S,F> (addr);
}

template<MemSpace MS, Size S, Flags F> u32
Moira::readMS(u32 addr)
{
    u32 result;
        
    if constexpr (S == Long) {

        // Break down the long word access into two word accesses
        result = readMS <MS, Word> (addr) << 16;
        result |= readMS <MS, Word, F> (addr + 2);

    } else {
        
        // Update function code pins
        setFC(MS == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
        
        // Check if a watchpoint is being accessed
        if ((flags & CPU_CHECK_WP) && debugger.watchpointMatches(addr, S)) {
            watchpointReached(addr);
        }
        
        // Perform the read operation
        sync(2);
        if (F & POLLIPL) pollIpl();
        result = (S == Byte) ? read8(addr & 0xFFFFFF) : read16(addr & 0xFFFFFF);
        sync(2);
    }
    
    return result;
}

template<Mode M, Size S, Flags F> void
Moira::writeM(u32 addr, u32 val, bool &error)
{
    if (isPrgMode(M)) {
        writeMS <MEM_PROG, S, F> (addr, val, error);
    } else {
        writeMS <MEM_DATA, S, F> (addr, val, error);
    }
}

template<Mode M, Size S, Flags F> void
Moira::writeM(u32 addr, u32 val)
{
    if (isPrgMode(M)) {
        writeMS <MEM_PROG, S, F> (addr, val);
    } else {
        writeMS <MEM_DATA, S, F> (addr, val);
    }
}

template<MemSpace MS, Size S, Flags F> void
Moira::writeMS(u32 addr, u32 val, bool &error)
{
    // Check for address errors
    if ((error = misaligned<S>(addr)) == true) {
        setFC(MS == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
        execAddressError(makeFrame <F|AE_WRITE> (addr), 2);
        return;
    }
    
    writeMS <MS,S,F> (addr, val);
}

template<MemSpace MS, Size S, Flags F> void
Moira::writeMS(u32 addr, u32 val)
{
    if constexpr (S == Long) {

        // Break down the long word access into two word accesses
        if (F & REVERSE) {
            writeMS <MS, Word>    (addr + 2, val & 0xFFFF);
            writeMS <MS, Word, F> (addr,     val >> 16   );
        } else {
            writeMS <MS, Word>    (addr,     val >> 16   );
            writeMS <MS, Word, F> (addr + 2, val & 0xFFFF);
        }

    } else {
        
        // Update function code pins
        setFC(MS == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
        
        // Check if a watchpoint is being accessed
        if ((flags & CPU_CHECK_WP) && debugger.watchpointMatches(addr, S)) {
            watchpointReached(addr);
        }
        
        // Perform the write operation
        sync(2);
        if (F & POLLIPL) pollIpl();
        S == Byte ? write8(addr & 0xFFFFFF, (u8)val) : write16(addr & 0xFFFFFF, (u16)val);
        sync(2);
    }
}

template<Size S> u32
Moira::readI()
{
    u32 result;

    switch (S) {
            
        case Byte:
            
            result = (u8)queue.irc;
            readExt();
            break;
            
        case Word:
            
            result = queue.irc;
            readExt();
            break;
            
        case Long:
            
            result = queue.irc << 16;
            readExt();
            result |= queue.irc;
            readExt();
            break;
            
        default:
            fatalError;
    }

    return result;
}

template<Size S, Flags F> void
Moira::push(u32 val)
{
    reg.sp -= S;
    writeMS <MEM_DATA,S,F> (reg.sp, val);
}

template<Size S, Flags F> void
Moira::push(u32 val, bool &error)
{
    reg.sp -= S;
    writeMS <MEM_DATA,S,F> (reg.sp, val, error);
}

template<Size S> bool
Moira::misaligned(u32 addr)
{
    return EMULATE_ADDRESS_ERROR ? ((addr & 1) && S != Byte) : false;
}

template <Flags F> AEStackFrame
Moira::makeFrame(u32 addr, u32 pc, u16 sr, u16 ird)
{
    AEStackFrame frame;
    u16 read = 0x10;
    
    // Prepare
    if (F & AE_WRITE) read = 0;
    if (F & AE_PROG) setFC(FC_USER_PROG);
    if (F & AE_DATA) setFC(FC_USER_DATA);

    // Create
    frame.code = (ird & 0xFFE0) | (u16)readFC() | read;
    frame.addr = addr;
    frame.ird = ird;
    frame.sr = sr;
    frame.pc = pc;

    // Adjust
    if (F & AE_INC_PC) frame.pc += 2;
    if (F & AE_DEC_PC) frame.pc -= 2;
    if (F & AE_INC_ADDR) frame.addr += 2;
    if (F & AE_DEC_ADDR) frame.addr -= 2;
    if (F & AE_SET_CB3) frame.code |= (1 << 3);
        
    return frame;
}

template <Flags F> AEStackFrame
Moira::makeFrame(u32 addr, u32 pc)
{
    return makeFrame <F> (addr, pc, getSR(), getIRD());
}

template <Flags F> AEStackFrame
Moira::makeFrame(u32 addr)
{
    return makeFrame <F> (addr, getPC(), getSR(), getIRD());
}

template<Flags F> void
Moira::prefetch()
{
    /* Whereas pc is a moving target (it moves forward while an instruction is
     * being processed, pc0 stays stable throughout the entire execution of
     * an instruction. It always points to the start address of the currently
     * executed instruction.
     */
    reg.pc0 = reg.pc;
    
    queue.ird = queue.irc;
    queue.irc = (u16)readMS <MEM_PROG, Word, F> (reg.pc + 2);
}

template<Flags F, int delay> void
Moira::fullPrefetch()
{    
    // Check for address error
    if (misaligned(reg.pc)) {
        execAddressError(makeFrame(reg.pc), 2);
        return;
    }

    queue.irc = (u16)readMS <MEM_PROG, Word> (reg.pc);
    if (delay) sync(delay);
    prefetch<F>();
}

void
Moira::readExt()
{
    reg.pc += 2;
    
    // Check for address error
    if (misaligned<Word>(reg.pc)) {
        execAddressError(makeFrame(reg.pc));
        return;
    }
    
    queue.irc = (u16)readMS <MEM_PROG, Word> (reg.pc);
}

template<Flags F> void
Moira::jumpToVector(int nr)
{
    u32 vectorAddr = 4 * nr;

    exception = nr;
    
    // Update the program counter
    reg.pc = readMS <MEM_DATA, Long> (vectorAddr);
    
    // Check for address error
    if (misaligned(reg.pc)) {
        if (nr != 3) {
            execAddressError(makeFrame<F|AE_PROG>(reg.pc, vectorAddr));
        } else {
            halt(); // Double fault
        }
        return;
    }
    
    // Update the prefetch queue
    queue.irc = (u16)readMS <MEM_PROG, Word> (reg.pc);
    sync(2);
    prefetch<POLLIPL>();
    
    // Stop emulation if the exception should be catched
    if (debugger.catchpointMatches(nr)) catchpointReached(u8(nr));

    signalJumpToVector(nr, reg.pc);
}
