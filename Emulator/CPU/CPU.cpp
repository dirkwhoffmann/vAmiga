// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
CPU::sync(int cycles)
{
    // Advance the CPU clock
    clock += cycles;

    // Emulate Agnus up to the same cycle
    agnus.executeUntil(CPU_CYCLES(clock));
}

u8
CPU::read8(u32 addr)
{
    return mem.peek8 <CPU_ACCESS> (addr);
}

u16
CPU::read16(u32 addr)
{
     return mem.peek16 <CPU_ACCESS> (addr);
}

u16
CPU::read16Dasm(u32 addr)
{
    return mem.spypeek16(addr);
}

u16
CPU::read16OnReset(u32 addr)
{
    return mem.chip ? read16(addr) : 0;
}

void
CPU::write8(u32 addr, u8 val)
{
    mem.poke8 <CPU_ACCESS> (addr, val);
}

void
CPU::write16 (u32 addr, u16 val)
{
    mem.poke16 <CPU_ACCESS> (addr, val);
}

void
CPU::signalReset()
{
    debug(XFILES, "XFILES (CPU): RESET instruction\n");
    amiga.softReset();
    debug("Reset done\n");
}

void
CPU::signalStop(u16 op)
{
    if (!(op & 0x2000)) {
        debug(XFILES, "XFILES (CPU): STOP instruction (%x)\n", op);
    }
}

void
CPU::signalTAS()
{
    debug(XFILES, "XFILES (CPU): TAS instruction\n");
}

void
CPU::signalHalt()
{
    amiga.putMessage(MSG_CPU_HALT);
}

void
CPU::signalTracingOn()
{
    debug(XFILES, "XFILES (CPU): TRACING ON\n");
}

void
CPU::signalTracingOff()
{
    debug(XFILES, "XFILES (CPU): TRACING OFF\n");
}

void
CPU::signalAddressError(moira::AEStackFrame &frame)
{
    debug(XFILES, "XFILES (CPU): Address error exception %x %x %x %x %x\n",
          frame.code, frame.addr, frame.ird, frame.sr, frame.pc);
}

void
CPU::signalLineAException(u16 opcode)
{
    debug(XFILES, "XFILES (CPU): lineAException(%x)\n", opcode);
}

void
CPU::signalLineFException(u16 opcode)
{
    debug(XFILES, "XFILES (CPU): lineFException(%x)\n", opcode);
}

void
CPU::signalIllegalOpcodeException(u16 opcode)
{
    debug(XFILES, "XFILES (CPU): illegalOpcodeException(%x)\n", opcode);
}

void
CPU::signalTraceException()
{
    debug(XFILES, "XFILES (CPU): traceException\n");
}

void
CPU::signalTrapException()
{
    debug(XFILES, "XFILES (CPU): trapException\n");
}

void
CPU::signalPrivilegeViolation()
{
}

void
CPU::signalInterrupt(u8 level)
{
    if (INT_DEBUG) {
        debug("*** INTERRUPT %d ***\n", level);
    }
    
    /*
    if (level == 3) {
        debug("IRQ level %d [%x%04x]\n", level, mem.spypeek16(0x6C), mem.spypeek16(0x6E));
    }
    */
}

void
CPU::signalJumpToVector(int nr, u32 addr)
{
    bool isIrqException = nr >= 24 && nr <= 31;

    if (isIrqException) {
        debug(INT_DEBUG, "Exception %d: Changing PC to %x\n", nr, addr);
    }
}

void
CPU::breakpointReached(u32 addr)
{
    amiga.setControlFlags(RL_BREAKPOINT_REACHED);
}

void
CPU::watchpointReached(u32 addr)
{
    amiga.setControlFlags(RL_WATCHPOINT_REACHED);
}

CPU::CPU(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("CPU");
}

void
CPU::_powerOn()
{
    debug(CPU_DEBUG, "CPU::_powerOn()\n");
}

void
CPU::_powerOff()
{

}

void
CPU::_run()
{
    debug(CPU_DEBUG, "CPU::_run()\n");
}

void
CPU::_reset(bool hard)
{
    debug(CPU_DEBUG, "CPU::_reset(%d)\n", hard);
    
    if (hard) {
        
        RESET_SNAPSHOT_ITEMS
        
        // Reset the Moira core
        Moira::reset();
        
        // Remove all previously recorded instructions
        debugger.clearLog();
        
    } else {
        
        /* A soft reset doesn't affect the CPU inside a real Amiga.
         * Hence, we only need to reset the clock to resync with the rest of
         * the system.
         */
        clock = 0;
    }
}

void
CPU::_inspect()
{
    _inspect(getPC0());
}

void
CPU::_inspect(u32 dasmStart)
{
    synchronized {
                
        // Registers
        info.pc0 = getPC0() & 0xFFFFFF;
        
        for (int i = 0; i < 8; i++) {
            info.d[i] = getD(i);
            info.a[i] = getA(i);
        }
        info.usp = getUSP();
        info.ssp = getSSP();
        info.sr = getSR();
        

        // Disassemble the program starting at 'dasmStart'
        info.start = dasmStart;
        for (unsigned i = 0; i < CPUINFO_INSTR_COUNT; i++) {
            
            int bytes = disassemble(dasmStart, info.instr[i].instr);
            disassemblePC(dasmStart, info.instr[i].addr);
            disassembleMemory(dasmStart, bytes / 2, info.instr[i].data);
            info.instr[i].sr[0] = 0;
            info.instr[i].bytes = bytes;
            dasmStart += bytes;
        }
        
        // Disassemble the most recent entries in the trace buffer
        long count = debugger.loggedInstructions();
        for (int i = 0; i < count; i++) {
            
            moira::Registers r = debugger.logEntryAbs(i);
            disassemble(r.pc, info.loggedInstr[i].instr);
            disassemblePC(r.pc, info.loggedInstr[i].addr);
            disassembleSR(r.sr, info.loggedInstr[i].sr);
        }
    }
}

void
CPU::_dumpConfig()
{
}

void
CPU::_dump()
{
    _inspect();
    
    msg("     PC0: %8X\n", info.pc0);
    msg(" D0 - D3: ");
    for (unsigned i = 0; i < 4; i++) msg("%8X ", info.d[i]);
    msg("\n");
    msg(" D4 - D7: ");
    for (unsigned i = 4; i < 8; i++) msg("%8X ", info.d[i]);
    msg("\n");
    msg(" A0 - A3: ");
    for (unsigned i = 0; i < 4; i++) msg("%8X ", info.a[i]);
    msg("\n");
    msg(" A4 - A7: ");
    for (unsigned i = 4; i < 8; i++) msg("%8X ", info.a[i]);
    msg("\n");
    msg("     SSP: %X\n", info.ssp);
    msg("   Flags: %X\n", info.sr);
}

DisassembledInstr
CPU::getInstrInfo(long nr, u32 start)
{
    // Update the cache if necessary
    if (info.start != start) _inspect(start);

    return getInstrInfo(nr);
}

DisassembledInstr
CPU::getInstrInfo(long nr)
{
    assert(nr < CPUINFO_INSTR_COUNT);
    
    DisassembledInstr result;
    synchronized { result = info.instr[nr]; }
    return result;
}

DisassembledInstr
CPU::getLoggedInstrInfo(long nr)
{
    assert(nr < CPUINFO_INSTR_COUNT);
    
    DisassembledInstr result;
    synchronized { result = info.loggedInstr[nr]; }
    return result;
}

size_t
CPU::_size()
{
    SerCounter counter;

    applyToPersistentItems(counter);
    applyToResetItems(counter);

    return counter.count;
}

size_t
CPU::didLoadFromBuffer(u8 *buffer)
{
    /* Because we don't save breakpoints and watchpoints in a snapshot, the
     * CPU flags for checking breakpoints and watchpoints can be in a corrupt
     * state after loading. These flags need to be updated according to the
     * current breakpoint and watchpoint list.
     */
    debugger.breakpoints.setNeedsCheck(debugger.breakpoints.elements() != 0);
    debugger.watchpoints.setNeedsCheck(debugger.watchpoints.elements() != 0);
    return 0;
}

DisassembledInstr
CPU::disassembleInstr(u32 addr)
{
    DisassembledInstr result;

    result.bytes = disassemble(addr, result.instr);
    disassemblePC(addr, result.addr);
    disassembleMemory(addr, result.bytes / 2, result.data);
    result.sr[0] = 0;

    return result;
}
