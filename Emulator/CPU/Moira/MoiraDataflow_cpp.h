// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#define LAST_BUS_CYCLE true

template<Mode M, Size S> bool
Moira::readOp(int n, u32 &ea, u32 &result)
{
    // Handle non-memory modes
    if (M == MODE_DN) { result = readD<S>(n); return true; }
    if (M == MODE_AN) { result = readA<S>(n); return true; }
    if (M == MODE_IM) { result = readI<S>();  return true; }

    // Compute effective address
    ea = computeEA<M,S>(n);

    // Update the function code pins
    setFC<M>();

    // Read from effective address
    bool error; result = readM<S>(ea, error);

    // Emulate -(An) register modification
    updateAnPD<M,S>(n);

    // Exit if an address error has occurred
    if (error) return false;

    // Emulate (An)+ register modification
    updateAnPI<M,S>(n);

    return !error;
}

template<Mode M, Size S, bool last> bool
Moira::writeOp(int n, u32 val)
{
    // Handle non-memory modes
    if (M == MODE_DN) { writeD<S>(n, val); return true;  }
    if (M == MODE_AN) { writeA<S>(n, val); return true;  }
    if (M == MODE_IM) { assert(false);     return false; }

    // Compute effective address
    u32 ea = computeEA<M,S>(n);

    // Update the function code pins
    setFC<M>();

    // Write to effective address
    bool error; writeM<S,last>(ea, val, error);

    // Emulate -(An) register modification
    updateAnPD<M,S>(n);

    // Early exit in case of an address error
    if (error) return false;
    
    // Emulate (An)+ register modification
    updateAnPI<M,S>(n);
    
    return !error;
}

template<Mode M, Size S, bool last> void
Moira::writeOp(int n, u32 ea, u32 val)
{
    // Handle non-memory modes
    if (M == MODE_DN) { writeD<S>(n, val); return; }
    if (M == MODE_AN) { writeA<S>(n, val); return; }
    if (M == MODE_IM) { assert(false);     return; }

    writeM<S,last>(ea, val);
}

template<Mode M, Size S, bool skip> u32
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
            sync(2);
            result = readA(n) - ((n == 7 && S == Byte) ? 2 : S);
            break;
        }
        case 5: // (d,An)
        {
            u32 an = readA(n);
            i16  d = (i16)queue.irc;

            result = d + an;
            readExt<skip>();
            break;
        }
        case 6: // (d,An,Xi)
        {
            i8   d = (i8)queue.irc;
            u32 an = readA(n);
            u32 xi = readR((queue.irc >> 12) & 0b1111);

            result = d + an + ((queue.irc & 0x800) ? xi : SEXT<Word>(xi));

            sync(2);
            readExt<skip>();
            break;
        }
        case 7: // ABS.W
        {
            result = (i16)queue.irc;
            readExt<skip>();
            break;
        }
        case 8: // ABS.L
        {
            result = queue.irc << 16;
            readExt();
            result |= queue.irc;
            readExt<skip>();
            break;
        }
        case 9: // (d,PC)
        {
            i16  d = (i16)queue.irc;

            result = reg.pc + d;
            readExt<skip>();
            break;
        }
        case 10: // (d,PC,Xi)
        {
            i8   d = (i8)queue.irc;
            u32 xi = readR((queue.irc >> 12) & 0b1111);

            result = d + reg.pc + ((queue.irc & 0x800) ? xi : SEXT<Word>(xi));
            sync(2);
            readExt<skip>();
            break;
        }
        case 11: // Im
        {
            result = readI<S>();
            break;
        }
        default:
        {
            assert(false);
        }
    }
    return result;
}

template<Mode M, Size S> void
Moira::updateAnPD(int n)
{
    // -(An)
    if (M == 4) reg.a[n] -= (n == 7 && S == Byte) ? 2 : S;
}

template<Mode M, Size S> void
Moira::undoAnPD(int n)
{
    // -(An)
    if (M == 4) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;
}

template<Mode M, Size S> void
Moira::updateAnPI(int n)
{
    // (An)+
    if (M == 3) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;

}

template<Mode M, Size S> void
Moira::updateAn(int n)
{
    // (An)+
    if (M == 3) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;

    // -(An)
    if (M == 4) reg.a[n] -= (n == 7 && S == Byte) ? 2 : S;
}

template<Size S, bool last> u32
Moira::readM(u32 addr)
{
    u32 result;

    if (S == Long) {
        result = readM<Word>(addr) << 16;
        result |= readM<Word,last>(addr + 2);
        return result;
    }

    // Check if a watchpoint is being accessed
    if ((flags & CPU_CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        watchpointReached(addr);
    }

    if (S == Byte) {
        sync(2);
        if (last) pollIrq();
        result = read8(addr & 0xFFFFFF);
        sync(2);
    }

    if (S == Word) {
        sync(2);
        if (last) pollIrq();
        result = read16(addr & 0xFFFFFF);
        sync(2);
    }

    return result;
}

template<Size S, bool last> u32
Moira::readM(u32 addr, bool &error)
{
    // Check for address error
    if ((error = misaligned<S>(addr))) {
        execAddressError(makeFrame(addr), 2);
        return 0;
    }
    
    // if ((error = addressReadError<S,2>(addr))) { return 0; }
    return readM<S,last>(addr);
}

template<Size S, bool last> void
Moira::writeM(u32 addr, u32 val)
{
    if (S == Long) {
        writeM<Word>     (addr,     val >> 16   );
        writeM<Word,last>(addr + 2, val & 0xFFFF);
        return;
    }

    // Check if a watchpoint is being accessed
    if ((flags & CPU_CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        watchpointReached(addr);
    }

    if (S == Byte) {
        sync(2);
        if (last) pollIrq();
        write8(addr & 0xFFFFFF, (u8)val);
        sync(2);
    }

    if (S == Word) {
        sync(2);
        if (last) pollIrq();
        write16(addr & 0xFFFFFF, (u16)val);
        sync(2);
    }
}

template<Size S, bool last> void
Moira::writeM(u32 addr, u32 val, bool &error)
{
    if ((error = misaligned<S>(addr))) {
        execAddressError(makeFrame(addr, true /* write */), 2);
        return;
    }
    // if ((error = addressWriteError<S,2>(addr))) { return; }
    
    writeM<S,last>(addr, val);
}

template<Size S, bool last> void
Moira::writeMrev(u32 addr, u32 val)
{
    switch (S) {

        case Byte:
        case Word:
        {
            writeM<S,last>(addr, val);
            break;
        }
        case Long:
        {
            writeM<Word>     (addr + 2, val & 0xFFFF);
            writeM<Word,last>(addr,     val >> 16   );
            break;
        }
    }
}

template<Size S, bool last> void
Moira::writeMrev(u32 addr, u32 val, bool &error)
{
    if ((error = misaligned<S>(addr))) {
        execAddressError(makeFrame(addr, true /* write */), 2);
        return;
    }
    // if ((error = addressWriteError<S,2>(addr))) { return; }
    writeMrev<S,last>(addr, val);
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
    }

    return result;
}

template<Size S, bool last> void
Moira::push(u32 val)
{
    reg.sp -= S;
    writeM<S,last>(reg.sp, val);
}

template<Size S, bool last> void
Moira::push(u32 val, bool &error)
{
    // printf("push<%d>(%x)\n", S, val);
    reg.sp -= S;
    writeM<S,last>(reg.sp, val, error);
}

template<Size S> bool
Moira::misaligned(u32 addr)
{
    return EMULATE_ADDRESS_ERROR ? ((addr & 1) && S != Byte) : false;
}

AEStackFrame
Moira::makeFrame(u32 addr, u32 pc, u16 sr, u16 ird, bool write)
{
    AEStackFrame frame;
    
    frame.code = (ird & 0xFFE0) | readFC() | (write ? 0x00 : 0x10);
    frame.addr = addr;
    frame.ird = ird;
    frame.sr = sr;
    frame.pc = pc;
    
    // Apply modification flags
    if (aeFlags & INC_PC_BY_2)    frame.pc += 2;
    if (aeFlags & DEC_PC_BY_2)    frame.pc -= 2;
    if (aeFlags & INC_ADDR_BY_2)  frame.addr += 2;
    if (aeFlags & DEC_ADDR_BY_2)  frame.addr -= 2;
    if (aeFlags & SET_CODE_BIT_3) frame.code |= (1 << 3);
    if (aeFlags & CLR_CODE_BIT_3) frame.code &= ~(1 << 3);
    aeFlags = 0;
    
    return frame;
}

AEStackFrame
Moira::makeFrame(u32 addr, u32 pc, bool write)
{
    return makeFrame(addr, pc, getSR(), getIRD(), write);
}

AEStackFrame
Moira::makeFrame(u32 addr, bool write)
{
    return makeFrame(addr, getPC(), getSR(), getIRD(), write);
}

template<bool last, int delay> void
Moira::prefetch()
{
    // At this point, the program counter points to the beginning of the
    // next instruction to execute. We save this value for further reference.
    reg.pc0 = reg.pc;
    
    // Update the function code pins
    setFC(FC_USER_PROG);

    queue.ird = queue.irc;
    if (delay) sync(delay);
    queue.irc = readM<Word,last>(reg.pc + 2);
}

template<bool last, int delay> void
Moira::newPrefetch()
{
    /* Whereas pc is a moving target (it moves forward while an instruction is
     * being processed, pc0 stays stable throughout instruction execution. It
     * always points to the start address of the currently executed instruction.
     */
    reg.pc0 = reg.pc;
    
    // Update the function code pins
    setFC(FC_USER_PROG);

    queue.ird = queue.irc;
    if (delay) sync(delay);
    reg.pc += 2;
    queue.irc = readM<Word,last>(reg.pc);
}

void
Moira::compensateNewPrefetch()
{
    // THIS METHOD MUST BE CALLED WHEN THE NEW PREFETCH FUNCTION IS USED.
    // IT FIXES THE PROGRAM COUNTER TO BE COMPATIBLE WITH THE OLD SEMANTICS
    // OF THE PC. ONCE ALL FUNCIONS USE THE NEW PREFETCH STYLE, THIS FUNCTION
    // WILL BE DELETET ENTIRELY.
    reg.pc -= 2;
}

template<bool last, int delay> void
Moira::fullPrefetch()
{
    // Update the function code pins
    setFC(FC_USER_PROG);
    
    // TODO: In theory, all PC address errors should be intercepted by now
    if (misaligned<Word>(reg.pc)) {
        execAddressError(makeFrame(reg.pc), 2);
        return;
    }
    // if (addressReadError<Word,2>(reg.pc)) return;

    queue.irc = readM<Word>(reg.pc);
    prefetch<last, delay>();
}

template<bool skip> void
Moira::readExt()
{
    reg.pc += 2;
    
    if (!skip) {
        
        // Update the function code pins
        setFC(FC_USER_PROG);
        
        // Check for address error
        if (misaligned<Word>(reg.pc)) {
            execAddressError(makeFrame(reg.pc));
            return;
        }
        
        // if (addressReadError<Word>(reg.pc)) return;
        queue.irc = readM<Word>(reg.pc);
    }
}

void
Moira::jumpToVector(int nr)
{
    exception = nr;
    
    // Update the function code pins
    setFC(FC_USER_DATA);

    // Update the program counter
    reg.pc = readM<Long>(4 * nr);

    // Align the exception pointer to an even address
    // FIXME: This is wrong.
    // TODO: Find out what the real CPU is doing here
    if (reg.pc & 1) {
        printf("ODD EXCEPTION %d POINTER: %x\n", nr, reg.pc); 
        reg.pc &= ~1;
        
        if (reg.sr.s) assert(false);
    }
    
    // Update the prefetch queue
    fullPrefetch<LAST_BUS_CYCLE,2>();
    
    /*
    queue.ird = readM<Word>(reg.pc);
    sync(2);
    queue.irc = readM<Word,LAST_BUS_CYCLE>(reg.pc + 2);
    */
    
    exceptionJump(nr, reg.pc);
}
