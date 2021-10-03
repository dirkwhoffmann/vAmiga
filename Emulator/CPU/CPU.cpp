// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CPU.h"
#include "Agnus.h"
#include "Amiga.h"
#include "IO.h"
#include "Memory.h"
#include "MsgQueue.h"

//
// Moira
//

namespace moira {

void
Moira::sync(int cycles)
{
    // Advance the CPU clock
    clock += cycles;

    // Emulate Agnus up to the same cycle
    agnus.execute(CPU_AS_DMA_CYCLES(cycles));
}

u8
Moira::read8(u32 addr)
{
    return mem.peek8 <ACCESSOR_CPU> (addr);
}

u16
Moira::read16(u32 addr)
{
    return mem.peek16 <ACCESSOR_CPU> (addr); 
}

u16
Moira::read16Dasm(u32 addr)
{
    return mem.spypeek16 <ACCESSOR_CPU> (addr);
}

u16
Moira::read16OnReset(u32 addr)
{
    return mem.chip ? read16(addr) : 0;
}

void
Moira::write8(u32 addr, u8 val)
{
    if constexpr (XFILES) if (addr - reg.pc < 5) {
        trace(true, "XFILES: write8 close to PC %x\n", reg.pc);
    }
    mem.poke8 <ACCESSOR_CPU> (addr, val);
}

void
Moira::write16 (u32 addr, u16 val)
{
    if constexpr (XFILES) if (addr - reg.pc < 5) {
        trace(true, "XFILES: write16 close to PC %x\n", reg.pc);
    }
    mem.poke16 <ACCESSOR_CPU> (addr, val);
}

u16
Moira::readIrqUserVector(u8 level) const
{
    return 0;
}

void
Moira::signalReset()
{
    trace(XFILES, "XFILES: RESET instruction\n");
    amiga.softReset();
}

void
Moira::signalStop(u16 op)
{
    if constexpr (XFILES) {
        if (!(op & 0x2000)) trace(true, "XFILES: STOP instruction (%x)\n", op);
    }
}

void
Moira::signalTAS()
{
    trace(XFILES, "XFILES: TAS instruction\n");
}

void
Moira::signalHalt()
{
    msgQueue.put(MSG_CPU_HALT);
}

void
Moira::signalAddressError(moira::AEStackFrame &frame)
{
    trace(XFILES, "XFILES: Address error exception %x %x %x %x %x\n",
          frame.code, frame.addr, frame.ird, frame.sr, frame.pc);
}

void
Moira::signalLineAException(u16 opcode)
{
    trace(XFILES, "XFILES: lineAException(%x)\n", opcode);
}

void
Moira::signalLineFException(u16 opcode)
{
    trace(XFILES, "XFILES: lineFException(%x)\n", opcode);
}

void
Moira::signalIllegalOpcodeException(u16 opcode)
{
    trace(XFILES, "XFILES: illegalOpcodeException(%x)\n", opcode);
}

void
Moira::signalTraceException()
{

}

void
Moira::signalTrapException()
{
    trace(XFILES, "XFILES: trapException\n");
}

void
Moira::signalPrivilegeViolation()
{
    
}

void
Moira::signalInterrupt(u8 level)
{
    debug(INT_DEBUG, "Executing level %d IRQ\n", level);
    
    /*
    if (agnus.frame.nr > 2180) {
        trace(true, "Executing level %d IRQ\n", level);
        amiga.signalStop();
    }
    */
}

void
Moira::signalJumpToVector(int nr, u32 addr)
{
    bool isIrqException = nr >= 24 && nr <= 31;

    if (isIrqException) {
        trace(INT_DEBUG, "Exception %d: Changing PC to %x\n", nr, addr);
    }
}

void
Moira::addressErrorHandler()
{
    
}

void
Moira::breakpointReached(u32 addr)
{
    if (debugger.breakpointPC == -1) {
        amiga.setFlag(RL::SOFTSTOP_REACHED);
    } else {
        amiga.setFlag(RL::BREAKPOINT_REACHED);
    }
}

void
Moira::watchpointReached(u32 addr)
{
    amiga.setFlag(RL::WATCHPOINT_REACHED);
}

}

//
// CPU
//

CPU::CPU(Amiga& ref) : moira::Moira(ref)
{
    
}

void
CPU::_reset(bool hard)
{    
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {
                
        // Reset the Moira core
        Moira::reset();
        
        // Remove all previously recorded instructions
        debugger.clearLog();
        
    } else {
        
        /* "The RESET instruction causes the processor to assert RESET for 124
         *  clock periods toreset the external devices of the system. The
         *  internal state of the processor is notaffected. Neither the status
         *  register nor any of the internal registers is affected by an
         *  internal reset operation. All external devices in the system should
         *  be reset at the completion of the RESET instruction."
         *      [Motorola M68000 User Manual]
         */            
    }
}

void
CPU::_inspect() const
{
    _inspect(getPC0());
}

void
CPU::_inspect(u32 dasmStart) const
{
    synchronized {
        
        info.pc0 = getPC0() & 0xFFFFFF;
        
        for (isize i = 0; i < 8; i++) {
            info.d[i] = getD((int)i);
            info.a[i] = getA((int)i);
        }
        info.usp = getUSP();
        info.ssp = getSSP();
        info.sr = getSR();
    }
}

void
CPU::_dump(dump::Category category, std::ostream& os) const
{
    // using namespace util;
    
    if (category & dump::State) {
        
        os << util::tab("Clock");
        os << util::dec(clock) << std::endl;
        os << util::tab("Control flags");
        os << util::hex((u16)flags) << std::endl;
        os << util::tab("Last exception");
        os << util::dec(exception);
    }
    
    if (category & dump::Registers) {

        os << util::tab("PC");
        os << util::hex(reg.pc0) << std::endl;
        os << std::endl;
        
        os << util::tab("SSP");
        os << util::hex(reg.ssp) << std::endl;
        os << util::tab("USP");
        os << util::hex(reg.usp) << std::endl;
        os << util::tab("IRC");
        os << util::hex(queue.irc) << std::endl;
        os << util::tab("IRD");
        os << util::hex(queue.ird) << std::endl;
        os << std::endl;
        
        os << util::tab("D0 - D3");
        os << util::hex(reg.d[0]) << ' ' << util::hex(reg.d[1]) << ' ';
        os << util::hex(reg.d[2]) << ' ' << util::hex(reg.d[3]) << ' ' << std::endl;
        os << util::tab("D4 - D7");
        os << util::hex(reg.d[4]) << ' ' << util::hex(reg.d[5]) << ' ';
        os << util::hex(reg.d[6]) << ' ' << util::hex(reg.d[7]) << ' ' << std::endl;
        os << util::tab("A0 - A3");
        os << util::hex(reg.a[0]) << ' ' << util::hex(reg.a[1]) << ' ';
        os << util::hex(reg.a[2]) << ' ' << util::hex(reg.a[3]) << ' ' << std::endl;
        os << util::tab("A4 - A7");
        os << util::hex(reg.a[4]) << ' ' << util::hex(reg.a[5]) << ' ';
        os << util::hex(reg.a[6]) << ' ' << util::hex(reg.a[7]) << ' ' << std::endl;
        os << std::endl;
        
        os << util::tab("Flags");
        os << (reg.sr.t ? 'T' : 't');
        os << (reg.sr.s ? 'S' : 's') << "--";
        os << "<" << util::dec(reg.sr.ipl) << ">---";
        os << (reg.sr.x ? 'X' : 'x');
        os << (reg.sr.n ? 'N' : 'n');
        os << (reg.sr.z ? 'Z' : 'z');
        os << (reg.sr.v ? 'V' : 'v');
        os << (reg.sr.c ? 'C' : 'c') << std::endl;
    }
}

void
CPU::_debugOn()
{
    debug(RUN_DEBUG, "Enabling debug mode\n");
    debugger.enableLogging();
}

void
CPU::_debugOff()
{
    debug(RUN_DEBUG, "Disabling debug mode\n");
    debugger.disableLogging();
}

isize
CPU::didLoadFromBuffer(const u8 *buffer)
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

const char *
CPU::disassembleRecordedInstr(isize i, isize *len)
{
    return disassembleInstr(debugger.logEntryAbs((int)i).pc0, len);
}
const char *
CPU::disassembleRecordedWords(isize i, isize len)
{
    return disassembleWords(debugger.logEntryAbs((int)i).pc0, len);
}

const char *
CPU::disassembleRecordedFlags(isize i)
{
    static char result[18];
    
    disassembleSR(debugger.logEntryAbs((int)i).sr, result);
    return result;
}

const char *
CPU::disassembleRecordedPC(isize i)
{
    static char result[16];
    
    Moira::disassemblePC(debugger.logEntryAbs((int)i).pc0, result);
    return result;
}

const char *
CPU::disassembleInstr(u32 addr, isize *len)
{
    static char result[128];

    int l = disassemble(addr, result);

    if (len) *len = (isize)l;
    return result;
}

const char *
CPU::disassembleWords(u32 addr, isize len)
{
    static char result[64];

    disassembleMemory(addr, (int)len, result);
    return result;
}

const char *
CPU::disassembleAddr(u32 addr)
{
    static char result[16];

    disassemblePC(addr, result);
    return result;
}

const char *
CPU::disassembleInstr(isize *len)
{
    return disassembleInstr(reg.pc0, len);
}
const char *
CPU::disassembleWords(isize len)
{
    return disassembleWords(reg.pc0, len);
    return "";
}
