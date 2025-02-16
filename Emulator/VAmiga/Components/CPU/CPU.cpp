// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "CmdQueue.h"
#include "Emulator.h"
#include "CPU.h"
#include "Agnus.h"
#include "Amiga.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MsgQueue.h"

//
// Moira
//

namespace vamiga::moira {

void
Moira::sync(int cycles)
{
    CPU *cpu = (CPU *)this;

    if (!cpu->config.overclocking) {

        // Advance the CPU clock
        clock += cycles;

        // Emulate Agnus up to the same cycle
        agnus.execute(CPU_AS_DMA_CYCLES(cycles));

    } else {

        // Compute the number of mico-cycles executed in one DMA cycle
        auto microCyclesPerCycle = 2 * cpu->config.overclocking;

        // Execute some cycles at normal speed if required
        while (cpu->slowCycles && cycles) {

            cpu->debt += microCyclesPerCycle;
            cycles--;
            cpu->slowCycles--;
        }

        // Execute all other cycles
        cpu->debt += cycles;

        while (cpu->debt >= microCyclesPerCycle) {

            // Advance the CPU clock by one DMA cycle
            clock += 2;

            // Emulate Agnus for one DMA cycle
            agnus.execute();

            cpu->debt -= microCyclesPerCycle;
        }
    }
}

u8
Moira::read8(u32 addr) const
{
    return mem.peek8<Accessor::CPU>(addr);
}

u16
Moira::read16(u32 addr) const
{
    return mem.peek16<Accessor::CPU>(addr);
}

u16
Moira::read16Dasm(u32 addr) const
{
    auto result = mem.spypeek16<Accessor::CPU>(addr);
    
    // For LINE-A instructions, check if the opcode is a software trap
    if (Debugger::isLineAInstr(result)) result = debugger.swTraps.resolve(result);

    return result;
}

u16
Moira::read16OnReset(u32 addr) const
{
    return mem.chip ? read16(addr) : 0;
}

void
Moira::write8(u32 addr, u8 val) const
{
    if (XFILES) {
        if (addr - reg.pc < 5) xfiles("write8 close to PC %x\n", reg.pc);
    }
    mem.poke8 <Accessor::CPU> (addr, val);
}

void
Moira::write16(u32 addr, u16 val) const
{
    if (XFILES) {
        if (addr - reg.pc < 5) xfiles("write16 close to PC %x\n", reg.pc);
    }
    mem.poke16 <Accessor::CPU> (addr, val);
}

u16
Moira::readIrqUserVector(u8 level) const
{
    return 0;
}

void
Moira::willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode)
{
    switch (I) {

        case STOP:

            if (!(opcode & 0x2000)) {
                // xfiles("STOP instruction (%x)\n", opcode);
            }
            break;

        case TAS:

            xfiles("TAS instruction\n");
            break;

        case BKPT:

            xfiles("BKPT instruction\n");
            break;

        default:
        {
            
            char str[128];
            disassemble(str, reg.pc0);
            printf("%s\n", str);
            break;
        }
    }
}

void
Moira::didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode)
{
    switch (I) {

        case RESET:

            xfiles("RESET instruction\n");
            amiga.softReset();
            break;

        default:
            break;
    }
}

void
Moira::willExecute(ExceptionType exc, u16 vector)
{
    /*
    switch (exc) {

        case EXC_RESET:             xfiles("EXC_RESET\n");              break;
        case EXC_BUS_ERROR:         xfiles("EXC_BUS_ERROR\n");          break;
        case EXC_ADDRESS_ERROR:     xfiles("EXC_ADDRESS_ERROR\n");      break;
        case EXC_ILLEGAL:           xfiles("EXC_ILLEGAL\n");            break;
        case EXC_DIVIDE_BY_ZERO:    xfiles("EXC_DIVIDE_BY_ZERO\n");     break;
        case EXC_CHK:               xfiles("EXC_CHK\n");                break;
        case EXC_TRAPV:             xfiles("EXC_TRAPV\n");              break;
        case EXC_PRIVILEGE:         xfiles("EXC_PRIVILEGE\n");          break;
        case EXC_TRACE:             xfiles("EXC_TRACE\n");              break;
        case EXC_LINEA:             xfiles("EXC_LINEA\n");              break;
        case EXC_LINEF:             xfiles("EXC_LINEF\n");              break;
        case EXC_FORMAT_ERROR:      xfiles("EXC_FORMAT_ERROR\n");       break;
        case EXC_IRQ_UNINITIALIZED: xfiles("EXC_IRQ_UNINITIALIZED\n");  break;
        case EXC_IRQ_SPURIOUS:      xfiles("EXC_IRQ_SPURIOUS\n");       break;
        case EXC_TRAP:              xfiles("EXC_TRAP\n");               break;

        default:
            break;
    }
    */
}

void
Moira::didExecute(ExceptionType exc, u16 vector)
{

}

void
Moira::cpuDidReset()
{

}

void
Moira::cpuDidHalt()
{
    msgQueue.put(Msg::CPU_HALT);
}

void
Moira::willInterrupt(u8 level)
{
    debug(INT_DEBUG, "Executing level %d IRQ\n", level);
}

void
Moira::didJumpToVector(int nr, u32 addr)
{
    bool isIrqException = nr >= 24 && nr <= 31;

    if (isIrqException) {
        trace(INT_DEBUG, "Exception %d: Changing PC to %x\n", nr, addr);
    }
}

void
Moira::didChangeCACR(u32 value)
{

}

void
Moira::didChangeCAAR(u32 value)
{

}

void
Moira::softstopReached(u32 addr)
{
    amiga.setFlag(RL::SOFTSTOP_REACHED);
}

void
Moira::breakpointReached(u32 addr)
{
    amiga.setFlag(RL::BREAKPOINT_REACHED);
}

void
Moira::watchpointReached(u32 addr)
{
    amiga.setFlag(RL::WATCHPOINT_REACHED);
}

void
Moira::catchpointReached(u8 vector)
{
    amiga.setFlag(RL::CATCHPOINT_REACHED);
}

void
Moira::softwareTrapReached(u32 addr)
{
    amiga.setFlag(RL::SWTRAP_REACHED);
}

}


//
// CPU
//

namespace vamiga {

CPU::CPU(Amiga& ref) : moira::Moira(ref)
{
    
}

i64
CPU::getOption(Opt option) const
{
    switch (option) {

        case Opt::CPU_REVISION:      return (long)config.revision;
        case Opt::CPU_DASM_REVISION: return (long)config.dasmRevision;
        case Opt::CPU_DASM_SYNTAX:   return (long)config.dasmSyntax;
        case Opt::CPU_DASM_NUMBERS:  return (long)config.dasmNumbers;
        case Opt::CPU_OVERCLOCKING:  return (long)config.overclocking;
        case Opt::CPU_RESET_VAL:     return (long)config.regResetVal;

        default:
            fatalError;
    }
}

void
CPU::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::CPU_REVISION:

            if (!CPURevEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, CPURevEnum::keyList());
            }
            return;

        case Opt::CPU_DASM_REVISION:

            if (!DasmRevEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, DasmRevEnum::keyList());
            }
            return;

        case Opt::CPU_DASM_SYNTAX:

            if (!DasmSyntaxEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, DasmSyntaxEnum::keyList());
            }
            return;

        case Opt::CPU_DASM_NUMBERS:

            if (!DasmNumbersEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, DasmNumbersEnum::keyList());
            }
            return;

        case Opt::CPU_OVERCLOCKING:
        case Opt::CPU_RESET_VAL:

            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
CPU::setOption(Opt option, i64 value)
{
    auto cpuModel = [&](CPURev rev) { return moira::Model(rev); };
    auto dasmModel = [&](DasmRev rev) { return moira::Model(rev); };
    auto syntax = [&](DasmSyntax rev) { return moira::DasmSyntax(rev); };

    switch (option) {

        case Opt::CPU_REVISION:

            config.revision = CPURev(value);
            setModel(cpuModel(config.revision), dasmModel(config.dasmRevision));
            return;

        case Opt::CPU_DASM_REVISION:

            config.dasmRevision = DasmRev(value);
            setModel(cpuModel(config.revision), dasmModel(config.dasmRevision));
            return;

        case Opt::CPU_DASM_SYNTAX:

            config.dasmSyntax = DasmSyntax(value);
            setDasmSyntax(syntax(config.dasmSyntax));
            return;

        case Opt::CPU_DASM_NUMBERS:

            config.dasmNumbers = DasmNumbers(value);
            
            switch (config.dasmNumbers) {
                    
                case DasmNumbers::HEX:
                    
                    setDasmNumberFormat(moira::DasmNumberFormat {
                        .prefix = "$",
                        .radix = 16,
                        .upperCase = false,
                        .plainZero = false
                    });
                    return;
                    
                case DasmNumbers::DEC:
                    
                    setDasmNumberFormat(moira::DasmNumberFormat {
                        .prefix = "",
                        .radix = 10,
                        .upperCase = false,
                        .plainZero = false
                    });
                    return;

            }
            
        case Opt::CPU_OVERCLOCKING:

            config.overclocking = isize(value);
            msgQueue.put(Msg::OVERCLOCKING, config.overclocking);
            return;

        case Opt::CPU_RESET_VAL:

            config.regResetVal = u32(value);
            return;

        default:
            fatalError;
    }
}

void
CPU::_didReset(bool hard)
{
    if (hard) {

        // Reset the Moira core
        Moira::reset();
        
        // Initialize all data and address registers with the startup value
        for(int i = 0; i < 8; i++) reg.d[i] = reg.a[i] = config.regResetVal;
        reg.a[7] = reg.isp;
        
        // Remove all recorded instructions and set the log flag if needed
        debugger.clearLog();
        if (emulator.isTracking()) flags |= moira::CPU_LOG_INSTRUCTION;

    } else {
        
        /* "The RESET instruction causes the processor to assert RESET for 124
         *  clock periods toreset the external devices of the system. The
         *  internal state of the processor is not affected. Neither the status
         *  register nor any of the internal registers is affected by an
         *  internal reset operation. All external devices in the system should
         *  be reset at the completion of the RESET instruction."
         *  [Motorola M68000 User Manual]
         */
    }
}

void
CPU::cacheInfo(CPUInfo &info) const
{
    {   SYNCHRONIZED
        
        info.clock = clock;

        info.pc0 = getPC0() & 0xFFFFFF;
        info.ird = getIRD();
        info.irc = getIRC();
        
        for (int i = 0; i < 8; i++) {
            info.d[i] = getD(i);
            info.a[i] = getA(i);
        }
        info.isp = getISP();
        info.usp = getUSP();
        info.msp = getMSP();
        info.vbr = getVBR();
        info.sr = getSR();
        info.sfc = (u8)getSFC();
        info.dfc = (u8)getDFC();
        info.cacr = (u8)getCACR();
        info.caar = (u8)getCAAR();
        info.ipl = (u8)getIPL();
        info.fc = (u8)readFC(); // TODO
        
        info.halt = isHalted();
    }
}

void
CPU::_dump(Category category, std::ostream& os) const
{
    auto print = [&](const string &name, const GuardList &guards) {

        for (int i = 0; i < guards.elements(); i++) {

            auto bp = *guards.guardNr(i);

            os << util::tab(name + " " + std::to_string(i));
            os << util::hex(bp.addr);

            if (!bp.enabled) os << " (Disabled)";
            else if (bp.ignore) os << " (Disabled for " << util::dec(bp.ignore) << " hits)";
            os << std::endl;
        }
    };

    if (category == Category::Config) {

        dumpConfig(os);
    }

    if (category == Category::Registers) {
        
        os << util::tab("PC");
        os << util::hex(reg.pc0) << std::endl;
        os << std::endl;
        
        os << util::tab("ISP");
        os << util::hex(reg.isp) << std::endl;
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
        os << (reg.sr.t1 ? 'T' : 't');
        os << (reg.sr.t0 ? 'T' : 't');
        os << (reg.sr.s ? 'S' : 's');
        os << (reg.sr.m ? 'M' : 'm') << "-";
        os << "<" << util::dec(reg.sr.ipl) << ">---";
        os << (reg.sr.x ? 'X' : 'x');
        os << (reg.sr.n ? 'N' : 'n');
        os << (reg.sr.z ? 'Z' : 'z');
        os << (reg.sr.v ? 'V' : 'v');
        os << (reg.sr.c ? 'C' : 'c') << std::endl;
    }

    if (category == Category::State) {

        os << util::tab("Clock");
        os << util::dec(clock) << std::endl;
        os << util::tab("Flags");
        os << util::hex((u16)flags) << std::endl;

        if (flags) {

            os << std::endl;
            if (flags & moira::CPU_IS_HALTED) os << util::tab("") << "CPU_IS_HALTED" << std::endl;
            if (flags & moira::CPU_IS_STOPPED) os << util::tab("") << "CPU_IS_STOPPED" << std::endl;
            if (flags & moira::CPU_IS_LOOPING) os << util::tab("") << "CPU_IS_LOOPING" << std::endl;
            if (flags & moira::CPU_LOG_INSTRUCTION) os << util::tab("") << "CPU_LOG_INSTRUCTION" << std::endl;
            if (flags & moira::CPU_CHECK_IRQ) os << util::tab("") << "CPU_CHECK_IRQ" << std::endl;
            if (flags & moira::CPU_TRACE_EXCEPTION) os << util::tab("") << "CPU_TRACE_EXCEPTION" << std::endl;
            if (flags & moira::CPU_TRACE_FLAG) os << util::tab("") << "CPU_TRACE_FLAG" << std::endl;
            if (flags & moira::CPU_CHECK_BP) os << util::tab("") << "CPU_CHECK_BP" << std::endl;
            if (flags & moira::CPU_CHECK_WP) os << util::tab("") << "CPU_CHECK_WP" << std::endl;
            if (flags & moira::CPU_CHECK_CP) os << util::tab("") << "CPU_CHECK_CP" << std::endl;
            os << std::endl;
        }

        os << util::tab("Read buffer");
        os << util::hex(readBuffer) << std::endl;
        os << util::tab("Write buffer");
        os << util::hex(readBuffer) << std::endl;
        os << util::tab("Last exception");
        os << util::dec(exception);
    }
    
    if (category == Category::Breakpoints) {

        if (debugger.breakpoints.elements()) {
            print("Breakpoint", breakpoints);
        } else {
            os << "No breakpoints set" << std::endl;
        }
    }

    if (category == Category::Watchpoints) {

        if (debugger.watchpoints.elements()) {
            print("Watchpoint", watchpoints);
        } else {
            os << "No watchpoints set" << std::endl;
        }
    }
    
    if (category == Category::Catchpoints) {

        if (debugger.catchpoints.elements()) {

            for (int i = 0; i < debugger.catchpoints.elements(); i++) {

                auto wp = debugger.catchpoints.guardNr(i);
                auto nr = "Catchpoint " + std::to_string(i);

                os << util::tab(nr);
                os << "Vector " << util::dec(wp->addr);
                os << " (" << cpu.debugger.vectorName(u8(wp->addr)) << ")";
                if (!wp->enabled) os << " (Disabled)";
                else if (wp->ignore) os << " (Disabled for " << wp->ignore << " hits)";
                os << std::endl;
            }

        } else {

            os << "No catchpoints set" << std::endl;
        }
    }

    if (category == Category::SwTraps) {

        if (!debugger.swTraps.traps.empty()) {

            for (auto &trap : debugger.swTraps.traps) {

                os << util::tab("0x" + util::hexstr <4> (trap.first));
                os << "Replaced by 0x" << util::hexstr <4> (trap.second.instruction);
                os << std::endl;
            }

        } else {

            os << "No software traps set" << std::endl;
        }
    }
}

void
CPU::_trackOn()
{
    debug(RUN_DEBUG, "Enabling debug mode\n");
    debugger.enableLogging();
}

void
CPU::_trackOff()
{
    debug(RUN_DEBUG, "Disabling debug mode\n");
    debugger.disableLogging();
}

void
CPU::_didLoad()
{
    auto cpuModel = (moira::Model)config.revision;
    auto dasmModel = (moira::Model)config.dasmRevision;

    // Rectify the CPU type
    setModel(cpuModel, dasmModel);

    /* Because we don't save breakpoints and watchpoints in a snapshot, the
     * CPU flags for checking breakpoints and watchpoints can be in a corrupt
     * state after loading. These flags need to be updated according to the
     * current breakpoint and watchpoint list.
     */
    debugger.breakpoints.setNeedsCheck(debugger.breakpoints.elements() != 0);
    debugger.watchpoints.setNeedsCheck(debugger.watchpoints.elements() != 0);
}

void
CPU::resyncOverclockedCpu()
{
    if (debt) {

        clock += 2;
        agnus.execute();
        debt = 0;
    }
}

const char *
CPU::disassembleRecordedInstr(isize i, isize *len) const
{
    return disassembleInstr(debugger.logEntryAbs((int)i).pc0, len);
}
const char *
CPU::disassembleRecordedWords(isize i, isize len) const
{
    return disassembleWords(debugger.logEntryAbs((int)i).pc0, len);
}

const char *
CPU::disassembleRecordedFlags(isize i) const
{
    static char result[18];
    
    disassembleSR(result, debugger.logEntryAbs((int)i).sr);
    return result;
}

const char *
CPU::disassembleRecordedPC(isize i) const
{
    static char result[16];

    Moira::dump24(result, debugger.logEntryAbs((int)i).pc0);
    return result;
}

const char *
CPU::disassembleAddr(u32 addr) const
{
    static char result[16];

    Moira::dump24(result, addr);
    return result;
}

const char *
CPU::disassembleWord(u16 value) const
{
    static char result[16];

    Moira::dump16(result, value);
    return result;
}

const char *
CPU::disassembleInstr(u32 addr, isize *len) const
{
    static char result[128];

    int l = disassemble(result, addr);

    if (len) *len = (isize)l;
    return result;
}

const char *
CPU::disassembleWords(u32 addr, isize len) const
{
    static char result[64];

    dump16(result, addr, (int)len);
    return result;
}

const char *
CPU::disassembleInstr(isize *len) const
{
    return disassembleInstr(reg.pc0, len);
}

const char *
CPU::disassembleWords(isize len) const
{
    return disassembleWords(reg.pc0, len);
}

const char *
CPU::disassemblePC() const
{
    return disassembleAddr(reg.pc0);
}

void
CPU::dumpLogBuffer(std::ostream& os, isize count) const
{
    isize numBytes = 0;
    isize num = debugger.loggedInstructions();

    for (isize i = num - count; i < num ; i++) {

        if (i >= 0) {

            auto pc = disassembleRecordedPC(i);
            auto instr = disassembleRecordedInstr(i, &numBytes);
            auto flags = disassembleRecordedFlags(i);

            os << std::setfill('0');
            os << "   ";
            os << std::right << std::setw(6) << pc;
            os << "  ";
            os << flags;
            os << "  ";
            os << instr;
            os << std::endl;
        }
    }
}

void
CPU::dumpLogBuffer(std::ostream& os) const
{
    dumpLogBuffer(os, debugger.loggedInstructions());
}

void
CPU::disassembleRange(std::ostream& os, u32 addr, isize count) const
{
    disassembleRange(os, std::pair<u32, u32>(addr, UINT32_MAX), count);
}

void
CPU::disassembleRange(std::ostream& os, std::pair<u32, u32> range, isize max) const
{
    u32 addr = range.first;
    isize numBytes = 0;
    auto pc = cpu.getPC0();

    for (isize i = 0; i < max && addr <= range.second; i++, addr += numBytes) {

        // auto pc = disassembleAddr(addr);
        auto instr = disassembleInstr(addr, &numBytes);
        auto data = disassembleWords(addr, numBytes / 2);

        os << std::setfill(' ');

        os << (addr == pc ? "->" : "  ");

        if (debugger.breakpoints.isDisabledAt(addr)) {
            os << "b";
        } else if (debugger.breakpoints.isSetAt(addr)) {
            os << "B";
        } else {
            os << " ";
        }

        os << std::right << std::setw(6) << disassembleAddr(addr);
        os << "  ";
        os << std::left << std::setw(15) << data;
        os << "   ";
        os << instr;
        os << std::endl;
    }
}

void
CPU::jump(u32 addr)
{
    debugger.jump(addr);
}

void
CPU::processCommand(const Command &cmd)
{
    isize nr = isize(cmd.value);
    u32 addr = u32(cmd.value);
    auto guards = (GuardList *)cmd.sender;

    switch (cmd.type) {

        case Cmd::GUARD_SET_AT:      guards->setAt(addr); break;
        case Cmd::GUARD_REMOVE_NR:   guards->remove(nr); break;
        case Cmd::GUARD_MOVE_NR:     guards->moveTo(nr, u32(cmd.value2)); break;
        case Cmd::GUARD_IGNORE_NR:   guards->ignore(nr, long(cmd.value2)); break;
        case Cmd::GUARD_REMOVE_AT:   guards->removeAt(addr); break;
        case Cmd::GUARD_REMOVE_ALL:  guards->removeAll(); break;
        case Cmd::GUARD_ENABLE_NR:   guards->enable(nr); break;
        case Cmd::GUARD_ENABLE_AT:   guards->enableAt(addr); break;
        case Cmd::GUARD_ENABLE_ALL:  guards->enableAll(); break;
        case Cmd::GUARD_DISABLE_NR:  guards->disable(nr); break;
        case Cmd::GUARD_DISABLE_AT:  guards->disableAt(addr); break;
        case Cmd::GUARD_DISABLE_ALL: guards->disableAll(); break;
            
        default:
            fatalError;
    }
}

}
