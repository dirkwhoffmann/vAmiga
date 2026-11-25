// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Core C> void
Moira::writeStackFrameAEBE(StackFrame &frame)
{
    if constexpr (C == Core::C68000) {

        U32_DEC(reg.sp, 14);
        write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 12), READ<Word>(frame.pc));
        write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 8), frame.sr);
        write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 10), READ<Word, 1>(frame.pc));
        write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 6), frame.ird);
        write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 4), (u16)frame.addr);
        write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 0), frame.code);
        write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 2), frame.addr >> 16);
        return;
    }

    // Push PC
    push<C, Word>(READ<Word>(frame.pc));
    push<C, Word>(READ<Word, 1>(frame.pc));

    // Push SR and IRD
    push<C, Word>(frame.sr);
    push<C, Word>(frame.ird);

    // Push address
    push<C, Word>((u16)frame.addr);
    push<C, Word>(frame.addr >> 16);

    // Push memory access type and function code
    push<C, Word>(frame.code);
}

template <Core C> void
Moira::writeStackFrame0000(u16 sr, u32 pc, u16 nr)
{
    // The 68020 does not align the stack pointer
    constexpr u32 mask = C == Core::C68020 ? ~0u : ~1u;

    switch (C) {

        case Core::C68000:

            U32_DEC(reg.sp, 6);
            if constexpr (compat != Compatibility::UAE) {

                // MAME has not adjusted the stack pointer yet if the first write fails
                try { write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 4) & mask, READ<Word>(pc)); }
                catch (BusError &) { U32_INC(reg.sp, 6); throw; }

            } else {

                write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 4) & mask, READ<Word>(pc));
            }
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 0) & mask, sr);
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 2) & mask, READ<Word, 1>(pc));
            break;

        case Core::C68010:

            // The write order follows UAE (it only matters if a write fails)
            U32_DEC(reg.sp, 8);
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 4) & mask, READ<Word>(pc));
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 0) & mask, sr);
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 2) & mask, READ<Word, 1>(pc));
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 6) & mask, 4 * nr);
            break;

        case Core::C68020:

            // The write order follows UAE (it only matters if a write fails)
            U32_DEC(reg.sp, 8);
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 6) & mask, 4 * nr);
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 2) & mask, READ<Word, 1>(pc));
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 4) & mask, READ<Word>(pc));
            write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 0) & mask, sr);
            break;
    }
}

template <Core C> void
Moira::writeStackFrame0001(u16 sr, u32 pc, u16 nr)
{
    assert(C == Core::C68020);

    // 0001 | Vector offset
    push<C, Word>(0x1000 | nr << 2);

    // Program counter
    push<C, Long>(pc);

    // Status register
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame0010(u16 sr, u32 pc, u32 ia, u16 nr)
{
    assert(C == Core::C68020);

    // Instruction address
    push<C, Long>(ia);

    // 0010 | Vector offset
    push<C, Word>(0x2000 | nr << 2);

    // Program counter
    push<C, Long>(pc);

    // Status register
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame0011(u16 sr, u32 pc, u32 ea, u16 nr)
{
    assert(C == Core::C68020);

    // Effective address
    push<C, Long>(ea);

    // 0011 | Vector offset
    push<C, Word>(0x3000 | nr << 2);

    // Program counter
    push<C, Long>(pc);

    // Status register
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame0111(u16 sr, u32 pc, u16 nr, u16 ssw, u32 addr, u32 dataOut)
{
    // Write-back 1 (unused)
    for (int i = 0; i < 4; i++) push<C, Long>(0);   // PD3 ... PD0
    push<C, Long>(0);                               // WB1A

    // Write-back 2 (unused)
    push<C, Long>(0);                               // WB2D
    push<C, Long>(0);                               // WB2A

    // Write-back 3 (the faulting write)
    bool write = !(ssw & SSW040::RW);
    push<C, Long>(dataOut);                         // WB3D
    push<C, Long>(write ? addr : 0);                // WB3A

    // Fault address
    push<C, Long>(addr);

    // Write-back status words
    push<C, Word>(0);                               // WB1S
    push<C, Word>(0);                               // WB2S
    push<C, Word>(write ? 0x80 | (ssw & 0x7F) : 0); // WB3S

    // Special status word
    push<C, Word>(ssw);

    // Effective address
    push<C, Long>(addr);

    // 0111 | Vector offset
    push<C, Word>(0x7000 | nr << 2);

    // Program counter
    push<C, Long>(pc);

    // Status register
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame1000(StackFrame &frame, u16 sr, u32 pc, u32 ia, u16 nr, u32 addr)
{
    assert(C == Core::C68010);

    // Internal information (the first word holds the version number, the rest is not emulated)
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(u32(VERSION_68010 << 10) << 16);

    // Instruction input buffer
    push<C, Word>(queue.irc);

    // Unused, reserved
    reg.sp -= 2;

    // Data input buffer
    push<C, Word>(readBuffer);

    // Unused, reserved
    reg.sp -= 2;

    // Data output buffer
    push<C, Word>(writeBuffer);

    // Unused, reserved
    reg.sp -= 2;

    // Fault address
    push<C, Long>(frame.addr);

    // Special status word
    push<C, Word>(frame.ssw);

    // 1000 | Vector offset
    push<C, Word>(0x8000 | nr << 2);

    // Program counter
    push<C, Long>(pc);

    // Status register
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame1001(u16 sr, u32 pc, u32 ia, u16 nr)
{

}

template <Core C> void
Moira::writeStackFrame1010(u16 sr, u32 pc, u16 nr, u16 ssw, u32 addr, u32 dataOut, u16 stageC, u16 stageB)
{
    // Internal registers
    push<C, Word>(0);
    push<C, Word>(0);

    // Data output buffer
    push<C, Long>(dataOut);

    // Internal registers
    push<C, Word>(0);
    push<C, Word>(0);

    // Data cycle fault address
    push<C, Long>(addr);

    // Instruction pipe stage B
    push<C, Word>(stageB);

    // Instruction pipe stage C
    push<C, Word>(stageC);

    // Special status word
    push<C, Word>(ssw);

    // Internal register
    push<C, Word>(0);

    // 1010 | Vector offset
    push<C, Word>(0xA000 | nr << 2);

    // Program counter
    push<C, Long>(pc);

    // Status register
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame1011(u16 sr, u32 pc, u32 ia, u16 nr, u16 ssw, u32 addr, u32 dataOut, u16 stageC, u16 stageB)
{
    // Internal registers
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);

    // Version#, Internal information
    push<C, Word>(0);

    // Internal registers
    push<C, Long>(0);
    push<C, Word>(0);

    // Data input buffer
    push<C, Long>(0);

    // Internal registers
    push<C, Long>(0);

    // Stage B address
    push<C, Long>(0);

    // Internal registers
    push<C, Long>(0);
    push<C, Long>(0);

    // Data output buffer
    push<C, Long>(dataOut);

    // Internal registers
    push<C, Word>(0);
    push<C, Word>(0);

    // Data cycle fault address
    push<C, Long>(addr);

    // Instruction pipe stage B
    push<C, Word>(stageB);

    // Instruction pipe stage C
    push<C, Word>(stageC);

    // Special status register
    push<C, Word>(ssw);

    // Internal register
    push<C, Word>(0);

    // 1011 | Vector offset
    push<C, Word>(0xB000 | nr << 2);

    // Program counter
    push<C, Long>(ia);

    // Status register
    push<C, Word>(sr);
}

/* The following code reproduces the 68020 bus error and address error frames of UAE.
 * UAE reports the stacked PC, the SSW and the pipe stages by simple rules. Many internal
 * words are not modeled. All frames use the long format ($B).
 */
static bool hasIndexedOperand(u16 ird)
{
    auto indexed = [](u16 m, u16 r) { return m == 6 || (m == 7 && r == 3); };

    switch (ird >> 12) {

        case 0x6: case 0x7: case 0xA: case 0xF: return false;
        case 0xE: if (((ird >> 6) & 3) != 3) return false; break;
        case 0x4: if ((ird & 0xFF80) == 0x4E00) return false; break;   // TRAP, LINK, UNLK, RTS, ...
    }
    if (indexed((ird >> 3) & 7, ird & 7)) return true;

    // MOVE has a second effective address
    return (ird & 0xC000) == 0 && (ird & 0x3000) && indexed((ird >> 6) & 7, (ird >> 9) & 7);
}

template <Core C> void
Moira::writeStackFrame1011UAE(u16 sr, u16 nr, u32 pc, u16 ssw, u32 addr, u32 end, bool write)
{
    u16 stageC = 0, stageB = 0;
    if (nr == 2 && !write) {
        stageC = peekInstr((end + 2) & addrMask<C>());
        stageB = peekInstr((end + 4) & addrMask<C>());
    }
    writeStackFrame1011<C>(sr, 0, pc, nr, ssw, addr, 0, stageC, stageB);
}

template <Core C> void
Moira::execAddressError(StackFrame frame, int delay)
{
    u16 status = getSR();
    u32 pc0Now = reg.pc0;

    // Inform the delegate
    willExecute(M68kException::ADDRESS_ERROR, 3);

    // Emulate additional delay
    sync(delay);

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlags();
    flags &= ~State::TRACE_EXC;

    // Interrupts are taken at an instruction boundary
    reg.pc = reg.pc0;
    SYNC(8);

    // A misaligned stack pointer will cause a double fault
    if (misaligned<C>(reg.sp)) throw DoubleFault();

    // Write stack frame
    if constexpr (C == Core::C68000) {

        // MAME delays the stack frame instead of the vector fetch
        SYNC(2);
        writeStackFrameAEBE<C>(frame);

    } else if constexpr (C == Core::C68010) {

        writeStackFrame1000<C>(frame, status, frame.pc, reg.pc0, 3, frame.addr);
        SYNC(2);

    } else if (cpuModel >= CPUModel::M68EC040) {

        // The 68040 has no bus fault frames. It reports an address error with a frame of
        // format 2 (MC68040UM, 8.4.3). Like UAE, the frame holds the address of the
        // instruction and the fault address with bit 0 cleared.
        writeStackFrame0010<C>(status, pc0Now, frame.addr & ~1, 3);

    } else {

        // The 68020 reports an odd instruction fetch with a frame of format B. According
        // to recordings made on a real A1200, the stacked PC is always the address of the
        // instruction (JMP, JSR, BSR, DBcc, RTS, RTR).
        u16 op = peekInstr(pc0Now & addrMask<C>());

        // Stage C and B are not emulated faithfully (UAE shows the next instruction word)
        bool ix = (op & 0xFFC0) == 0x4EC0 && hasIndexedOperand(op);
        u16 stageC = ix ? queue.irc : 0, stageB = (op == 0x4E75 || op == 0x4E77) ? queue.irc : 0;

        // Special status word as recorded on the A1200: Rerun flags for stage C (always) and
        // stage B (except for JMP d16(An) and BSR), supervisor program space, and the data
        // direction / size bits of the stack read performed by RTS and RTR
        bool jmpD16 = (op & 0xFFF8) == 0x4EE8;
        u16 ssw = 0x2005 | ((jmpD16 || (op >> 8) == 0x61) ? 0 : 0x1000);
        if (op == 0x4E75) ssw |= 0x40;
        if (op == 0x4E77) ssw |= 0x60;
        writeStackFrame1011<C>(status, 0, pc0Now, 3, ssw, frame.addr, 0, stageC, stageB);
    }

    // Jump to exception vector
    jumpToVector<C>(3);

    // Inform the delegate
    didExecute(M68kException::ADDRESS_ERROR, 3);
}

template <Core C> void
Moira::execBusError(StackFrame frame, int delay, const Registers *saved)
{
    u16 status = getSR();

    // UAE stacks a fixed special status word on the 68020 (read, word, function code)
    u16 uaeSsw = 0x60 | (getFC() & 7);

    // Inform the delegate
    willExecute(M68kException::BUS_ERROR, 2);

    // Emulate additional delay
    sync(delay);

    // Enter supervisor mode
    Registers faultReg = reg;
    setSupervisorMode(true);
    u32 entrySsp = reg.sp, entryUsp = reg.usp;

    // Disable tracing
    clearTraceFlags();
    flags &= ~State::TRACE_EXC;
    SYNC(8);

    // A misaligned stack pointer will cause a double fault
    if (misaligned<C>(reg.sp)) throw DoubleFault();

    // Write stack frame
    if constexpr (C == Core::C68000) {

        // MAME completes the faulting bus cycle and delays the frame instead of the vector fetch
        sync(4);
        writeStackFrameAEBE<C>(frame);

    } else if constexpr (C == Core::C68010) {

        writeStackFrame1000<C>(frame, status, frame.pc, reg.pc0, 2, frame.addr);
        SYNC(2);

    } else if (cpuModel >= CPUModel::M68EC040) {

        // The write-back buffer 3 holds the data of a faulting write
        bool write = !(frame.ssw & SSW040::RW);
        writeStackFrame0111<C>(status, reg.pc0, 2, frame.ssw, frame.addr, write ? frame.data : 0);

    } else if (cpuModel >= CPUModel::M68EC030) {

        /* The 68030 reports the cause of the fault in the special status word
         * and the logical fault address. A fault in a write cycle is reported
         * with the short format ($A), all others with the long format ($B).
         */

        // The pipe stages hold the two instruction words behind the opcode
        bool supervisor = status & 0x2000;
        auto peek = [&](u32 addr) -> u16 {

            addr &= addrMask<C>();
            if (mmu.isEnabled()) {
                auto phys = mmu.translate(addr, supervisor ? FC::SUPERVISOR_PROG : FC::USER_PROG, false);
                if (!phys) return 0;
                addr = phys.value() & addrMask<C>();
            }
            return peekInstr(addr);
        };
        u16 stageC = peek(reg.pc0 + 2);
        u16 stageB = peek(reg.pc0 + 4);

        // The data output buffer holds the value of a faulting write
        bool write = !(frame.ssw & SSW030::RW) && (frame.ssw & SSW030::DF);
        u32 dataOut = write ? frame.data : 0;

        if (write) {
            writeStackFrame1010<C>(status, reg.pc0, 2, frame.ssw, frame.addr, dataOut, stageC, stageB);
        } else {
            writeStackFrame1011<C>(status, frame.pc, reg.pc0, 2, frame.ssw, frame.addr, dataOut, stageC, stageB);
        }

    } else {

        // The 68020 follows UAE
        u16 op = peekInstr(reg.pc0 & addrMask<C>());
        auto indexed = [](u16 m, u16 r) { return m == 6 || (m == 7 && r == 3); };
        bool move = (op & 0xC000) == 0 && (op & 0x3000);
        u16 srcMode = (op >> 3) & 7, srcReg = op & 7;
        u16 dstMode = (op >> 6) & 7, dstReg = (op >> 9) & 7;

        // UAE stacks the start of the instruction. Indexed operands move it behind the instruction words.
        // The extension word of the destination of a MOVE is consumed after the source has been read.
        bool ix = hasIndexedOperand(op);
        if (ix && move && !indexed(srcMode, srcReg) && !frame.write) ix = false;

        // Determine the address behind the instruction words that have been consumed when the fault occurred.
        u32 end = reg.pc0 + 2;

        if constexpr (MOIRA_ENABLE_DASM) {

            // The length of the instruction is determined by the disassembler handlers of the CPU model.
            // Without MOIRA_ENABLE_DASM, the length is unknown and we fall back to the first extension word.
            end = reg.pc0 + peekInstrLength(reg.pc0);

            // The destination extension words of a MOVE are consumed after the source has been accessed
            if (move) {
                u32 destBytes = (dstMode == 5 || dstMode == 6) ? 2 : (dstMode == 7 && dstReg == 0) ? 2 : (dstMode == 7 && dstReg == 1) ? 4 : 0;
                if (!frame.write) end -= destBytes;
                else if (dstMode == 7 && dstReg == 1) end -= 2;
            }
        }

        u32 pc = ix ? end : reg.pc0;

        // If the source of a MOVE is indexed, UAE stacks the address of the extension words of the destination
        if (ix && move && frame.write && indexed(srcMode, srcReg)) {
            if (dstMode == 5 || (dstMode == 7 && dstReg <= 1)) pc -= 2;
        }
        u16 ssw = compat == Compatibility::UAE ? uaeSsw : frame.ssw;
        writeStackFrame1011UAE<C>(status, 2, pc, ssw, frame.addr, end, frame.write);

        // Remember how to restart the instruction
        restart.valid = saved != nullptr;
        if (restart.valid) {

            restart.pre = *saved;
            restart.fault = faultReg;
            auto sr = reg.sr; reg.sr = saved->sr; restart.preSr = getSR(); reg.sr = sr;
            restart.pc = reg.pc0;
            restart.frameSp = reg.sp;
            restart.frameSr = status;
            restart.framePC = pc;
            restart.entrySsp = entrySsp;
            restart.entryUsp = entryUsp;
        }
    }

    // Jump to exception vector
    jumpToVector<C>(2);

    // Inform the delegate
    didExecute(M68kException::BUS_ERROR, 2);
}

template <Core C> void
Moira::processException(const std::exception &exc, const Registers *saved)
{
    // A fault aborts the instruction and terminates loop mode
    flags &= ~State::LOOPING;

    try {

        if (auto ae = dynamic_cast<const AddressError *>(&exc); ae) {

            execAddressError<C>(ae->stackFrame);
            return;
        }

        if (auto be = dynamic_cast<const BusError *>(&exc); be) {

            // Abort the current instruction. It will be restarted by RTE.
            // Like UAE, the 68020 keeps the registers as they are and rolls
            // them back when the handler returns (see Restart).
            bool deferred = cpuModel <= CPUModel::M68020;
            if (saved && !deferred) restoreRegisters(*saved);

            // UAE decrements an address register after the access has succeeded
            u16 op = (saved && deferred) ? peekInstr(reg.pc0 & addrMask<C>()) : 0;
            if (saved && deferred) undoPredecrement(be->stackFrame, *saved, op);

            // UAE sets the condition codes of a MOVE before it writes the result
            if (saved && deferred && be->stackFrame.write && (op >> 12) >= 1 && (op >> 12) <= 3) {

                int bits = (op >> 12) == 1 ? 8 : (op >> 12) == 3 ? 16 : 32;
                u32 mask = bits == 32 ? 0xFFFFFFFF : (1u << bits) - 1;
                reg.sr.n = (be->stackFrame.data >> (bits - 1)) & 1;
                reg.sr.z = (be->stackFrame.data & mask) == 0;
                reg.sr.v = 0;
                reg.sr.c = 0;
            }

            // A bus error while processing a bus error is a double bus fault
            try { execBusError<C>(be->stackFrame, 0, saved); } catch (const BusError &) { halt(); }
            return;
        }

        if (auto df = dynamic_cast<const DoubleFault *>(&exc); df) {

            throw *df;
        }

    } catch (const DoubleFault &) {

        halt();
        return;
    }

    // Unhandled exception (we should not be here)
    fprintf(stderr, "Moira: CPU halted at PC %06x: %s\n", reg.pc0, exc.what());
    halt();
}

// The following definitions are not templated. They are compiled into the
// main translation unit only (see MoiraCore_cpp.h).
#ifdef MOIRA_MAIN_TU

void
Moira::execException(M68kException exc, int nr)
{
    switch (cpuModel) {

        case CPUModel::M68000: execException<Core::C68000>(exc, nr); break;
        case CPUModel::M68010: execException<Core::C68010>(exc, nr); break;
        default:            execException<Core::C68020>(exc, nr); break;
    }
}

void
Moira::undoPredecrement(const StackFrame &frame, const Registers &saved, u16 ird)
{
    /* UAE decrements an address register after the operand has been read, and before
     * a pure write. Hence, only a faulting read has to be taken back. A write fault
     * sees the register as it is.
     */
    if (frame.write) return;
    u32 addr = frame.addr;

    int regs[2] = { -1, -1 };
    u32 size = 0;

    switch (ird >> 12) {

        case 0x1: case 0x2: case 0x3:   // MOVE

            size = (ird >> 12) == 1 ? 1 : (ird >> 12) == 3 ? 2 : 4;
            if (((ird >> 3) & 7) == 4) regs[0] = ird & 7;
            if (((ird >> 6) & 7) == 4) regs[1] = (ird >> 9) & 7;
            break;

        case 0x8: case 0x9: case 0xC: case 0xD:

            // SBCD, SUBX, ABCD, ADDX in memory-to-memory form
            if ((ird & 0x0138) == 0x0108 && (ird & 0xC0) != 0xC0) {
                regs[0] = ird & 7; regs[1] = (ird >> 9) & 7;
                size = ((ird >> 12) == 8 || (ird >> 12) == 0xC) ? 1 : 1 << ((ird >> 6) & 3);
            }
            else if (((ird >> 3) & 7) == 4) regs[0] = ird & 7;
            break;

        case 0x4:

            // MOVEM stacks its registers in its own way
            if ((ird & 0xFB80) == 0x4880) break;
            if (((ird >> 3) & 7) == 4) regs[0] = ird & 7;
            break;

        case 0x0: case 0x5: case 0xB: case 0xE:

            if (((ird >> 3) & 7) == 4) regs[0] = ird & 7;
            break;

        default:
            break;
    }

    for (int i = 0; i < 2; i++) {

        int r = regs[i];
        if (r < 0 || reg.a[r] != addr || saved.a[r] == reg.a[r]) continue;

        // Both operands may use the same register
        bool both = regs[0] == regs[1];
        u32 delta = saved.a[r] - reg.a[r];
        reg.a[r] += (both && size) ? (r == 7 && size == 1 ? 2 : size) : delta;
        break;
    }
}

void
Moira::restoreRegisters(const Registers &saved)
{
    u8 ipl = reg.ipl;
    reg = saved;
    iCache.refresh();
    dCache.refresh();
    reg.ipl = ipl;

    // Bring the dependent flags in sync with the restored status register
    flags &= ~(State::TRACING | State::TRACE_EXC);
    if (reg.sr.t1) flags |= State::TRACING;
    flags |= State::CHECK_IRQ;
}

void
Moira::processException(const std::exception &exc, const Registers *saved)
{
    switch (cpuModel) {

        case CPUModel::M68000: processException<Core::C68000>(exc, saved); break;
        case CPUModel::M68010: processException<Core::C68010>(exc, saved); break;
        
        default:
            processException<Core::C68020>(exc, saved);
    }
}

#endif

template <Core C> void
Moira::execFpuTrap(u16 vector, bool post, u32 ea)
{
    u16 status = getSR();

    // Inform the delegate
    willExecute(M68kException::FPU_ARITHMETIC, vector);

    // Exceptions terminate loop mode and clear any pending trace event
    flags &= ~(State::LOOPING | State::TRACE_EXC);

    // Enter supervisor mode and leave trace mode
    setSupervisorMode(true);
    clearTraceFlags();

    // Write stack frame
    if (post) {
        writeStackFrame0011<C>(status, reg.pc, ea, vector);
    } else {
        writeStackFrame0000<C>(status, reg.pc0, vector);
    }

    // Branch to exception handler
    jumpToVector<C, AE_SET_CB3>(vector);

    // Inform the delegate
    didExecute(M68kException::FPU_ARITHMETIC, vector);
}

template <Core C> bool
Moira::takePendingFpuTrap()
{
    if (!fpu.pendingTrap) return false;

    // The exception is taken once. A handler that returns continues with the instruction.
    u16 vector = fpu.pendingTrap;
    fpu.pendingTrap = 0;
    execFpuTrap<C>(vector);

    return true;
}

template <Core C> void
Moira::execException(M68kException exc, int nr)
{
    u16 status = getSR();

    // Determine the exception vector number
    u16 vector =
    exc == M68kException::TRAP ? u16(exc) + u16(nr) :
    exc == M68kException::BKPT ? 4 :
    exc == M68kException::FPU_UNIMPLEMENTED ? 11 : u16(exc);

    // Inform the delegate
    willExecute(exc, vector);

    // Exceptions terminate loop mode
    flags &= ~State::LOOPING;

    // Trace on change of flow (T0) also follows the exceptions that instructions raise
    [[maybe_unused]] const bool t0 = C == Core::C68020 && reg.sr.t0 && !reg.sr.t1;

    // Enter supervisor mode and leave trace mode
    setSupervisorMode(true);
    clearTraceFlags();

    // A misaligned stack pointer causes a double fault
    if constexpr (C == Core::C68000) {
        if (exc != M68kException::BUS_ERROR && misaligned<C>(reg.sp)) throw DoubleFault();
    }

    switch (exc) {

        case M68kException::BUS_ERROR:

            // Write stack frame
            writeStackFrame1011<C>(status, reg.pc, reg.pc0, 2);

            // Branch to exception handler
            jumpToVector<C>(2);
            break;

        case M68kException::ILLEGAL:
        case M68kException::LINEA:
        case M68kException::LINEF:

            // Clear any pending trace event
            flags &= ~State::TRACE_EXC;

            SYNC(4);

            // Write stack frame
            if constexpr (C == Core::C68000) {

                // MAME marks bus errors during the exception sequence as "not instruction" cycles
                try {
                    try { writeStackFrame0000<C>(status, reg.pc - 2, vector); }
                    catch (BusError &e) { e.stackFrame.pc -= 2; throw; }
                    jumpToVector<C, AE_SET_CB3>(vector);
                }
                catch (BusError &e) { e.stackFrame.code |= 0x8; throw; }
                break;
            }
            if constexpr (C == Core::C68010 || C == Core::C68020) {
                writeStackFrame0000<C>(status, reg.pc0, vector);
            } else {
                writeStackFrame0000<C>(status, reg.pc - 2, vector);
            }

            // Branch to exception handler
            jumpToVector<C, AE_SET_CB3>(vector);
            break;

        case M68kException::BKPT:

            // Clear any pending trace event
            flags &= ~State::TRACE_EXC;

            SYNC(2);
            (void)readM<C, Mode::DN, Word>(reg.pc);
            SYNC(2);

            // Write stack frame
            writeStackFrame0000<C>(status, U32_SUB(reg.pc, 2), vector);

            // Branch to exception handler
            jumpToVector<C, AE_SET_CB3>(vector);
            break;

        case M68kException::FPU_UNIMPLEMENTED:

            // Clear any pending trace event
            flags &= ~State::TRACE_EXC;

            // The 68040 stacks the address of the next instruction and the effective
            // address of the operand, which is passed in nr (MC68040UM, 9.6.1)
            writeStackFrame0010<C>(status, reg.pc, u32(nr), vector);

            // Branch to exception handler
            jumpToVector<C, AE_SET_CB3>(vector);
            break;

        case M68kException::DIVIDE_BY_ZERO:
        case M68kException::CHK:
        case M68kException::TRAPV:

            // Write stack frame
            C == Core::C68020 ?
            writeStackFrame0010<C>(status, reg.pc, reg.pc0, vector) :
            writeStackFrame0000<C>(status, reg.pc, vector);

            // Branch to exception handler
            jumpToVector<C,AE_FETCH>(vector);
            break;

        case M68kException::PRIVILEGE:

            // Clear any pending trace event
            flags &= ~State::TRACE_EXC;

            SYNC(4);

            // Write stack frame
            if constexpr (C == Core::C68000) {

                try {
                    try { writeStackFrame0000<C>(status, reg.pc - 2, vector); }
                    catch (BusError &e) { e.stackFrame.pc -= 2; throw; }
                    jumpToVector<C,AE_SET_CB3>(vector);
                }
                catch (BusError &e) { e.stackFrame.code |= 0x8; throw; }
                break;
            }
            writeStackFrame0000<C>(status, reg.pc - 2, vector);

            // Branch to exception handler
            jumpToVector<C,AE_SET_CB3>(vector);
            break;

        case M68kException::TRACE:

            // Clear any pending trace event
            flags &= ~State::TRACE_EXC;

            // Recover from stop state
            flags &= ~State::STOPPED;

            SYNC(4);

            // Trace exceptions are taken at an instruction boundary
            reg.pc = reg.pc0;

            // Write stack frame (the 68020 stacks the address of the traced instruction)
            if constexpr (C == Core::C68020) {
                writeStackFrame0010<C>(status, reg.pc, reg.prev_pc0, vector);
            } else {
                writeStackFrame0000<C>(status, reg.pc, vector);
            }

            // Branch to exception handler
            jumpToVector<C>(vector);
            break;

        case M68kException::FORMAT_ERROR:

            // Clear any pending trace event
            flags &= ~State::TRACE_EXC;

            // Write stack frame
            writeStackFrame0000<C>(status, reg.pc - 2, vector);

            // Branch to exception handler
            jumpToVector<C, AE_SET_CB3>(vector);
            break;

        case M68kException::TRAP:

            // Write stack frame
            writeStackFrame0000<C>(status, reg.pc, u16(vector));

            // Branch to exception handler
            jumpToVector<C>(vector);
            break;

        default:
            break;
    }

    if constexpr (C == Core::C68020) {
        if (t0 && (exc == M68kException::TRAP || exc == M68kException::CHK ||
                   exc == M68kException::TRAPV || exc == M68kException::DIVIDE_BY_ZERO)) {
            flags |= State::TRACE_EXC;
        }
    }

    // Inform the delegate
    didExecute(exc, vector);
}

// The following definitions are not templated. They are compiled into the
// main translation unit only (see MoiraCore_cpp.h).
template <Core C> void
Moira::rethrow(BusError &e, u8 mask, u8 ccr)
{
    // The reference CPUs have already changed the flags when the access fails
    setCCR((getCCR() & ~mask) | (ccr & mask));

    // Show the flags in the stacked status register (a frame supplied by the host is left alone)
    if (!e.custom) e.stackFrame.sr = (e.stackFrame.sr & ~CCR::ALL) | getCCR();

    throw;
}

template <Core C> void
Moira::rethrow(BusError &e)
{
    rethrow<C>(e, CCR::ALL, getCCR());
}

template <Core C, Size S> void
Moira::rethrowD(BusError &e, int n, u32 value)
{
    // MAME updates the register (lower word for longs) before the prefetch
    if constexpr (C == Core::C68000) writeD<S == Long ? Word : S>(n, value);
    rethrow<C>(e);
}

template <Core C, Size S> void
Moira::rethrowA(BusError &e, int n, u32 value)
{
    if constexpr (C == Core::C68000) writeA<S>(n, value);
    rethrow<C>(e);
}

#ifdef MOIRA_MAIN_TU

u8
Moira::shiftPendingCCR(Instr I, Size S, u32 operand) const
{
    // Long operands are processed word by word
    u32 nbit = S == Byte ? 0x80 : 0x8000;
    u32 zmask = S == Byte ? 0xFF : 0xFFFF;

    u8 ccr = 0;
    if (operand & nbit) ccr |= CCR::N;
    if ((operand & zmask) == 0) ccr |= CCR::Z;

    // ROXL and ROXR copy the X flag into C
    if ((I == Instr::ROXL || I == Instr::ROXR) && reg.sr.x) ccr |= CCR::C;

    return ccr;
}

void
Moira::execInterrupt(u8 level)
{
    switch (cpuModel) {

        case CPUModel::M68000: execInterrupt<Core::C68000>(level); break;
        case CPUModel::M68010: execInterrupt<Core::C68010>(level); break;
        
        default:
            execInterrupt<Core::C68020>(level);
    }
}

#endif

template <Core C> void
Moira::execInterrupt(u8 level)
{
    assert(level < 8);

    // Notify delegate
    willInterrupt(level);

    // Level 7 is edge triggered
    if (level == 7) nmiTaken = true;

    // Remember the current value of the status register
    u16 status = getSR();

    // Recover from stop state and terminate loop mode
    flags &= ~(State::STOPPED | State::LOOPING);

    // Clear the polled IPL value
    reg.ipl = 0;

    // Temporarily raise the interrupt threshold
    reg.sr.ipl = level;

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlags();
    flags &= ~State::TRACE_EXC;

    switch (C) {

        case Core::C68000:

            SYNC(6);
            reg.sp -= 6;
            write<C, AddrSpace::DATA, Word>(reg.sp + 4, READ<Word>(reg.pc - 2));

            SYNC(4);
            queue.ird = getIrqVector(level);

            SYNC(4);
            write<C, AddrSpace::DATA, Word>(reg.sp + 0, status);
            write<C, AddrSpace::DATA, Word>(reg.sp + 2, READ<Word, 1>(reg.pc - 2));
            break;

        case Core::C68010:

            SYNC(12);
            reg.sp -= 8;
            queue.ird = getIrqVector(level);
            write<C, AddrSpace::DATA, Word>(reg.sp + 4, READ<Word>(reg.pc - 2));
            write<C, AddrSpace::DATA, Word>(reg.sp + 0, status);
            write<C, AddrSpace::DATA, Word>(reg.sp + 2, READ<Word, 1>(reg.pc - 2));
            write<C, AddrSpace::DATA, Word>(reg.sp + 6, 4 * queue.ird);
            break;

        case Core::C68020:

            queue.ird = getIrqVector(level);

            // Both frame writers take the vector number, not the vector offset
            writeStackFrame0000<C>(status, reg.pc - 2, queue.ird);

            if (reg.sr.m) {

                setMasterMode(false);
                writeStackFrame0001<C>(status | 0x2000, reg.pc - 2, queue.ird);
            }
    }

    jumpToVector<C, AE_SET_CB3>(queue.ird);
}
