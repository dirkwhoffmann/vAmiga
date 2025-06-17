// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Core C> void
Moira::writeStackFrameAEBE(StackFrame &frame)
{
    // Push PC
    push<C, Word>((u16)frame.pc);
    push<C, Word>(frame.pc >> 16);

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
    switch (C) {

        case Core::C68000:

            if constexpr (MOIRA_MIMIC_MUSASHI) {

                push<C, Long>(pc);
                push<C, Word>(sr);

            } else {

                U32_DEC(reg.sp, 6);
                write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 4) & ~1, pc & 0xFFFF);
                write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 0) & ~1, sr);
                write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 2) & ~1, pc >> 16);
            }
            break;

        case Core::C68010:
        case Core::C68020:

            if constexpr (MOIRA_MIMIC_MUSASHI) {

                push<C, Word>(nr << 2);
                push<C, Long>(pc);
                push<C, Word>(sr);

            } else {

                U32_DEC(reg.sp, 8);
                write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 6) & ~1, 4 * nr);
                write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 4) & ~1, pc & 0xFFFF);
                write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 0) & ~1, sr);
                write<C, AddrSpace::DATA, Word>(U32_ADD(reg.sp, 2) & ~1, pc >> 16);
            }
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
Moira::writeStackFrame1000(StackFrame &frame, u16 sr, u32 pc, u32 ia, u16 nr, u32 addr)
{
    assert(C == Core::C68010);

    // Internal information
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);
    push<C, Long>(0);

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
Moira::writeStackFrame1010(u16 sr, u32 pc, u16 nr)
{
    // Internal registers
    push<C, Word>(0);
    push<C, Word>(0);

    // Data output buffer
    push<C, Long>(0);

    // Internal registers
    push<C, Word>(0);
    push<C, Word>(0);

    // Data cycle fault address
    push<C, Long>(0);

    // Instruction pipe stage B
    push<C, Word>(0);

    // Instruction pipe stage C
    push<C, Word>(0);

    // Special status word
    push<C, Word>(0);

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
Moira::writeStackFrame1011(u16 sr, u32 pc, u32 ia, u16 nr)
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
    push<C, Long>(0);

    // Internal registers
    push<C, Word>(0);
    push<C, Word>(0);

    // Data cycle fault address
    push<C, Long>(0);

    // Instruction pipe stage B
    push<C, Word>(0);

    // Instruction pipe stage C
    push<C, Word>(0);

    // Special status register
    push<C, Word>(0);

    // Internal register
    push<C, Word>(0);

    // 1011 | Vector offset
    push<C, Word>(0xB000 | nr << 2);

    // Program counter
    push<C, Long>(ia);

    // Status register
    push<C, Word>(sr);
}

template <Core C> void
Moira::execAddressError(StackFrame frame, int delay)
{
    u16 status = getSR();

    // Inform the delegate
    willExecute(M68kException::ADDRESS_ERROR, 3);

    // Emulate additional delay
    sync(delay);

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlags();
    flags &= ~State::TRACE_EXC;
    SYNC(8);

    // A misaligned stack pointer will cause a double fault
    if (misaligned<C>(reg.sp)) throw DoubleFault();

    // Write stack frame
    if constexpr (C == Core::C68000) {
        writeStackFrameAEBE<C>(frame);
    } else {
        writeStackFrame1000<C>(frame, status, frame.pc, reg.pc0, 3, frame.addr);
    }
    SYNC(2);

    // Jump to exception vector
    jumpToVector<C>(3);

    // Inform the delegate
    didExecute(M68kException::ADDRESS_ERROR, 3);
}

template <Core C> void
Moira::execBusError(StackFrame frame, int delay)
{
    u16 status = getSR();

    // Inform the delegate
    willExecute(M68kException::BUS_ERROR, 2);

    // Emulate additional delay
    sync(delay);

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlags();
    flags &= ~State::TRACE_EXC;
    SYNC(8);

    // A misaligned stack pointer will cause a double fault
    if (misaligned<C>(reg.sp)) throw DoubleFault();

    // Write stack frame
    if constexpr (C == Core::C68000) {
        writeStackFrameAEBE<C>(frame);
    } else {
        writeStackFrame1000<C>(frame, status, frame.pc, reg.pc0, 2, frame.addr);
    }
    SYNC(2);

    // Jump to exception vector
    jumpToVector<C>(2);

    // Inform the delegate
    didExecute(M68kException::BUS_ERROR, 2);
}

void
Moira::execException(M68kException exc, int nr)
{
    switch (cpuModel) {

        case Model::M68000: execException<Core::C68000>(exc, nr); break;
        case Model::M68010: execException<Core::C68010>(exc, nr); break;
        default:            execException<Core::C68020>(exc, nr); break;
    }
}

template <Core C> void
Moira::execException(M68kException exc, int nr)
{
    u16 status = getSR();

    // Determine the exception vector number
    u16 vector = (exc == M68kException::TRAP) ? u16(exc) + u16(nr) : (exc == M68kException::BKPT) ? 4 : u16(exc);

    // Inform the delegate
    willExecute(exc, vector);

    // Remember the exception vector
    exception = vector;

    // Enter supervisor mode and leave trace mode
    setSupervisorMode(true);
    clearTraceFlags();

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

        case M68kException::DIVIDE_BY_ZERO:
        case M68kException::CHK:
        case M68kException::TRAPV:

            // Write stack frame
            C == Core::C68020 ?
            writeStackFrame0010<C>(status, reg.pc, reg.pc0, vector) :
            writeStackFrame0000<C>(status, reg.pc, vector);

            // Branch to exception handler
            jumpToVector<C,AE_SET_RW|AE_SET_IF>(vector);
            break;

        case M68kException::PRIVILEGE:

            // Clear any pending trace event
            flags &= ~State::TRACE_EXC;

            SYNC(4);

            // Write stack frame
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

            // Write stack frame
            writeStackFrame0000<C>(status, reg.pc, vector);

            // Branch to exception handler
            jumpToVector<C>(vector);
            break;

        case M68kException::FORMAT_ERROR:

            // Clear any pending trace event
            flags &= ~State::TRACE_EXC;

            // Write stack frame
            if (MOIRA_MIMIC_MUSASHI) {
                writeStackFrame0000<C>(status, reg.pc, vector);
            } else {
                writeStackFrame0000<C>(status, reg.pc - 2, vector);
            }

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

    // Inform the delegate
    didExecute(exc, vector);
}

void
Moira::execInterrupt(u8 level)
{
    switch (cpuModel) {

        case Model::M68000: execInterrupt<Core::C68000>(level); break;
        case Model::M68010: execInterrupt<Core::C68010>(level); break;
        
        default:
            execInterrupt<Core::C68020>(level);
    }
}

template <Core C> void
Moira::execInterrupt(u8 level)
{
    assert(level < 8);

    // Notify delegate
    willInterrupt(level);

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
            write<C, AddrSpace::DATA, Word>(reg.sp + 4, reg.pc & 0xFFFF);

            SYNC(4);
            queue.ird = getIrqVector(level);

            SYNC(4);
            write<C, AddrSpace::DATA, Word>(reg.sp + 0, status);
            write<C, AddrSpace::DATA, Word>(reg.sp + 2, reg.pc >> 16);
            break;

        case Core::C68010:

            SYNC(12);
            reg.sp -= 8;
            queue.ird = getIrqVector(level);
            write<C, AddrSpace::DATA, Word>(reg.sp + 4, reg.pc & 0xFFFF);
            write<C, AddrSpace::DATA, Word>(reg.sp + 0, status);
            write<C, AddrSpace::DATA, Word>(reg.sp + 2, reg.pc >> 16);
            write<C, AddrSpace::DATA, Word>(reg.sp + 6, 4 * queue.ird);
            break;

        case Core::C68020:

            queue.ird = getIrqVector(level);

            writeStackFrame0000<C>(status, reg.pc, 4 * queue.ird);

            if (reg.sr.m) {

                writeStackFrame0001<C>(status, reg.pc, 4 * queue.ird);
            }
    }

    jumpToVector<C, AE_SET_CB3>(queue.ird);
}
