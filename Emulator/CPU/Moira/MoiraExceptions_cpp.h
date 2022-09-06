// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Core C> void
Moira::writeStackFrameAEBE(StackFrame &frame)
{
    // assert(C == C68000);
    
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
            
        case C68000:
            
            if constexpr (MIMIC_MUSASHI) {
                
                push<C, Long>(pc);
                push<C, Word>(sr);
                
            } else {
                
                reg.sp -= 6;
                writeMS<C, MEM_DATA, Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA, Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;
            
        case C68010:
        case C68020:
            
            if constexpr (MIMIC_MUSASHI) {
                
                push<C, Word>(nr << 2);
                push<C, Long>(pc);
                push<C, Word>(sr);
                
            } else {
                
                reg.sp -= 8;
                writeMS<C, MEM_DATA, Word>((reg.sp + 6) & ~1, 4 * nr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 4) & ~1, pc & 0xFFFF);
                writeMS<C, MEM_DATA, Word>((reg.sp + 0) & ~1, sr);
                writeMS<C, MEM_DATA, Word>((reg.sp + 2) & ~1, pc >> 16);
            }
            break;
    }
}

template <Core C> void
Moira::writeStackFrame0001(u16 sr, u32 pc, u16 nr)
{
    assert(C == C68020);
    
    push<C, Word>(0x1000 | nr << 2);
    push<C, Long>(pc);
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame0010(u16 sr, u32 pc, u32 ia, u16 nr)
{
    assert(C == C68020);
    
    push<C, Long>(ia);
    push<C, Word>(0x2000 | nr << 2);
    push<C, Long>(pc);
    push<C, Word>(sr);
}

template <Core C> void
Moira::writeStackFrame1000(u16 sr, u32 pc, u16 nr)
{
    
}

template <Core C> void
Moira::writeStackFrame1001(u16 sr, u32 pc, u32 ia, u16 nr)
{
    
}

template <Core C> void
Moira::writeStackFrame1010(u16 sr, u32 pc, u16 nr){
    
}

template <Core C> void
Moira::writeStackFrame1011(u16 sr, u32 pc, u16 nr)
{
    
}

/*
void
Moira::execAddressError(StackFrame frame, int delay)
{
    switch (core) {
            
        case C68000: execAddressError<C68000>(frame, delay); break;
        case C68010: execAddressError<C68010>(frame, delay); break;
        case C68020: execAddressError<C68020>(frame, delay); break;
            
        default:
            assert(false);
    }
}
*/

template <Core C> void
Moira::execAddressError(StackFrame frame, int delay)
{
    assert(frame.addr & 1);
    
    // Inform the delegate
    willExecute(EXC_ADDRESS_ERROR, 3);
    
    // Emulate additional delay
    sync(delay);
    
    // Enter supervisor mode
    setSupervisorMode(true);
    
    // Disable tracing
    clearTraceFlags();
    flags &= ~CPU_TRACE_EXCEPTION;
    SYNC(8);
    
    // A misaligned stack pointer will cause a double fault
    bool doubleFault = misaligned<C>(reg.sp);
    
    if (!doubleFault) {
        
        // Write stack frame
        writeStackFrameAEBE<C>(frame);
        SYNC(2);
        jumpToVector<C>(3);
    }
    
    // Halt the CPU if a double fault occurred
    if (doubleFault) halt();
    
    // Inform the delegate
    didExecute(EXC_ADDRESS_ERROR, 3);
}

void
Moira::execException(ExceptionType exc, int nr)
{
    switch (model) {

        case M68000:    execException<C68000>(exc, nr); break;
        case M68010:    execException<C68010>(exc, nr); break;
        default:        execException<C68020>(exc, nr); break;
    }
}

template <Core C> void
Moira::execException(ExceptionType exc, int nr)
{
    u16 status = getSR();

    // Determine the exception vector number
    u16 vector = (exc == EXC_TRAP) ? u16(exc + nr) : u16(exc);

    // Inform the delegate
    willExecute(exc, vector);

    // Remember the exception vector
    exception = vector;

    // Enter supervisor mode and leave trace mode
    setSupervisorMode(true);
    clearTraceFlags();
    
    // Leave trace mode
    
    switch (exc) {
            
        case EXC_ILLEGAL:
        case EXC_LINEA:
        case EXC_LINEF:
            
            // Clear any pending trace event
            flags &= ~CPU_TRACE_EXCEPTION;
            
            SYNC(4);
            
            // Write stack frame
            writeStackFrame0000<C>(status, reg.pc - 2, vector);

            // Branch to exception handler
            jumpToVector<C, AE_SET_CB3>(vector);
            break;

        case EXC_BKPT:

            // Clear any pending trace event
            flags &= ~CPU_TRACE_EXCEPTION;

            SYNC(4);

            // Write stack frame
            writeStackFrame0000<C>(status, reg.pc, 4);

            // Branch to exception handler
            jumpToVector<C, AE_SET_CB3>(4);
            break;

        case EXC_DIVIDE_BY_ZERO:
        case EXC_CHK:
        case EXC_TRAPV:

            // Write stack frame
            C == C68020 ?
            writeStackFrame0010<C>(status, reg.pc, reg.pc0, vector) :
            writeStackFrame0000<C>(status, reg.pc, vector);
            
            // Branch to exception handler
            jumpToVector<C>(vector);
            break;
            
        case EXC_PRIVILEGE:
            
            // Clear any pending trace event
            flags &= ~CPU_TRACE_EXCEPTION;
            
            SYNC(4);
            
            // Write stack frame
            writeStackFrame0000<C>(status, reg.pc - 2, vector);
            
            // Branch to exception handler
            jumpToVector<C,AE_SET_CB3>(vector);
            break;
            
        case EXC_TRACE:
            
            // Clear any pending trace event
            flags &= ~CPU_TRACE_EXCEPTION;
            
            // Recover from stop state
            flags &= ~CPU_IS_STOPPED;
            
            SYNC(4);
            
            // Write stack frame
            writeStackFrame0000<C>(status, reg.pc, vector);
            
            // Branch to exception handler
            jumpToVector<C>(vector);
            break;
            
        case EXC_FORMAT_ERROR:
            
            // Clear any pending trace event
            flags &= ~CPU_TRACE_EXCEPTION;
            
            SYNC(4);
            
            // Write stack frame
            if (MIMIC_MUSASHI) {
                writeStackFrame0000<C>(status, reg.pc, vector);
            } else {
                writeStackFrame0000<C>(status, reg.pc - 2, vector);
            }
            
            // Branch to exception handler
            jumpToVector<C, AE_SET_CB3>(vector);
            break;
            
        case EXC_TRAP:
            
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
    switch (model) {

        case M68000:    execInterrupt<C68000>(level); break;
        case M68010:    execInterrupt<C68010>(level); break;
        default:        execInterrupt<C68020>(level); break;
    }
}

template <Core C> void
Moira::execInterrupt(u8 level)
{
    assert(level < 8);
    
    // Notify delegate
    signalInterrupt(level);
    
    // Remember the current value of the status register
    u16 status = getSR();
    
    // Recover from stop state and terminate loop mode
    flags &= ~(CPU_IS_STOPPED | CPU_IS_LOOPING);
    
    // Clear the polled IPL value
    reg.ipl = 0;
    
    // Temporarily raise the interrupt threshold
    reg.sr.ipl = level;
    
    // Enter supervisor mode
    setSupervisorMode(true);
    
    // Disable tracing
    clearTraceFlags();
    flags &= ~CPU_TRACE_EXCEPTION;
    
    switch (C) {
            
        case C68000:
            
            SYNC(6);
            reg.sp -= 6;
            writeMS<C, MEM_DATA, Word>(reg.sp + 4, reg.pc & 0xFFFF);
            
            SYNC(4);
            queue.ird = getIrqVector(level);
            
            SYNC(4);
            writeMS<C, MEM_DATA, Word>(reg.sp + 0, status);
            writeMS<C, MEM_DATA, Word>(reg.sp + 2, reg.pc >> 16);
            break;
            
        case C68010:
            
            SYNC(6);
            reg.sp -= 8;
            writeMS<C, MEM_DATA, Word>(reg.sp + 4, reg.pc & 0xFFFF);
            
            SYNC(4);
            queue.ird = getIrqVector(level);
            
            SYNC(4);
            writeMS<C, MEM_DATA, Word>(reg.sp + 0, status);
            writeMS<C, MEM_DATA, Word>(reg.sp + 2, reg.pc >> 16);
            
            writeMS<C, MEM_DATA, Word>(reg.sp + 6, 4 * queue.ird);
            break;
            
        case C68020:
            
            queue.ird = getIrqVector(level);
            
            writeStackFrame0000<C>(status, reg.pc, 4 * queue.ird);
            
            if (reg.sr.m) {
                
                writeStackFrame0001<C>(status, reg.pc, 4 * queue.ird);
            }
    }
    
    jumpToVector<C, AE_SET_CB3>(queue.ird);
}
