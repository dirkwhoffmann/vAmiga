// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

// Reference to the active Amiga instance
// DEPRECATED
Amiga *activeAmiga = NULL;

//
// Interface to Moira
//

namespace moira {

void
Moira::sync(int cycles)
{
    clock += cycles;
}

u8
Moira::read8(u32 addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.peek8(addr);
}

u16
Moira::read16(u32 addr)
{
     assert(activeAmiga != NULL);
     return activeAmiga->mem.peek16<BUS_CPU>(addr);
}

u16
Moira::read16Dasm(u32 addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.spypeek16(addr);
}

u16
Moira::read16OnReset(u32 addr)
{
    u16 result = 0;

    if (activeAmiga && activeAmiga->mem.chip) result = read16(addr);

    return result;
}

void
Moira::write8(u32 addr, u8 val)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke8(addr, val);
}

void
Moira::write16 (u32 addr, u16 val)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke16<BUS_CPU>(addr, val);
}

void
Moira::breakpointReached(u32 addr)
{
    printf("breakpointReached(%x)", addr);
    activeAmiga->setControlFlags(RL_BREAKPOINT_REACHED);
}

void
Moira::watchpointReached(u32 addr)
{
    printf("watchpointReached(%x)", addr);
    activeAmiga->setControlFlags(RL_WATCHPOINT_REACHED);
}

}


//
// CPU class
//

CPU::CPU(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("CPU");
    activeAmiga = &ref;
}

CPU::~CPU()
{
    
}

void
CPU::_initialize()
{
    debug(CPU_DEBUG, "CPU::_initialize()\n");
}

void
CPU::_powerOn()
{
    debug(CPU_DEBUG, "CPU::_powerOn()\n");
}

void
CPU::_powerOff()
{
    if (activeAmiga == &amiga) {
        debug("Stop being the active emulator instance\n");
        activeAmiga = NULL;
    }
}

void
CPU::_run()
{
    debug(CPU_DEBUG, "CPU::_run()\n");
}

void
CPU::_reset()
{
    debug(CPU_DEBUG, "CPU::_reset()\n");

    RESET_SNAPSHOT_ITEMS

    moiracpu.reset();

    // Remove all previously recorded instructions
    clearTraceBuffer();
}

void
CPU::_inspect()
{
    uint32_t pc;

    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);

    pc = getPC();

    // Registers
    info.pc = pc;

    for (int i = 0; i < 8; i++) {
        info.d[i] = moiracpu.getD(i);
        info.a[i] = moiracpu.getA(i);
    }
    info.usp = moiracpu.getUSP();
    info.ssp = moiracpu.getSSP();
    info.flags = moiracpu.getSR();

    // Disassemble the program starting at the program counter
    for (unsigned i = 0; i < CPUINFO_INSTR_COUNT; i++) {
        disassemble(pc, info.instr[i]);
        pc += info.instr[i].bytes;
    }

    // Disassemble the most recent entries in the trace buffer

    /* The last element in the trace buffer is the instruction that will be
     * be executed next. Because we don't want to show this element yet, we
     * don't dissassemble it.
     */
    for (unsigned i = 1; i <= CPUINFO_INSTR_COUNT; i++) {
        unsigned offset = (writePtr + traceBufferCapacity - 1 - i) % traceBufferCapacity;
        RecInstr recInstr = traceBuffer[offset];
        disassemble(recInstr, info.traceInstr[CPUINFO_INSTR_COUNT - i]);
    }

    pthread_mutex_unlock(&lock);
}

void
CPU::_dumpConfig()
{
}

void
CPU::_dump()
{
    _inspect();
    
    plainmsg("      PC: %8X\n", info.pc);
    plainmsg(" D0 - D3: ");
    for (unsigned i = 0; i < 4; i++) plainmsg("%8X ", info.d[i]);
    plainmsg("\n");
    plainmsg(" D4 - D7: ");
    for (unsigned i = 4; i < 8; i++) plainmsg("%8X ", info.d[i]);
    plainmsg("\n");
    plainmsg(" A0 - A3: ");
    for (unsigned i = 0; i < 4; i++) plainmsg("%8X ", info.a[i]);
    plainmsg("\n");
    plainmsg(" A4 - A7: ");
    for (unsigned i = 4; i < 8; i++) plainmsg("%8X ", info.a[i]);
    plainmsg("\n");
    plainmsg("     SSP: %X\n", info.ssp);
    plainmsg("   Flags: %X\n", info.flags);
}

CPUInfo
CPU::getInfo()
{
    CPUInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

DisInstr
CPU::getInstrInfo(long index)
{
    assert(index < CPUINFO_INSTR_COUNT);
    
    DisInstr result;
    
    pthread_mutex_lock(&lock);
    result = info.instr[index];
    pthread_mutex_unlock(&lock);
    
    return result;
}

DisInstr
CPU::getTracedInstrInfo(long index)
{
    assert(index < CPUINFO_INSTR_COUNT);
    
    DisInstr result;
    
    pthread_mutex_lock(&lock);
    result = info.traceInstr[index];
    pthread_mutex_unlock(&lock);
    
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
CPU::didLoadFromBuffer(uint8_t *buffer)
{
    SerReader reader(buffer);

    debug(SNAP_DEBUG, "CPU state checksum: %x (%d bytes)\n",
          fnv_1a_64(buffer, reader.ptr - buffer), reader.ptr - buffer);

    return reader.ptr - buffer;
}

size_t
CPU::didSaveToBuffer(uint8_t *buffer)
{
    SerWriter writer(buffer);

    debug(SNAP_DEBUG, "CPU state checksum: %x (%d bytes)\n",
          fnv_1a_64(buffer, writer.ptr - buffer), writer.ptr - buffer);

    return writer.ptr - buffer;
}

uint32_t
CPU::lengthOfInstruction(uint32_t addr)
{
    char tmp[128];
    return moiracpu.disassemble(addr, tmp);
}

void
CPU::disassemble(uint32_t addr, DisInstr &result)
{
    result.bytes = moiracpu.disassemble(addr, result.instr);

    mem.hex(result.data, addr, result.bytes, sizeof(result.data));
    sprint24x(result.addr, addr);

    result.flags[0] = 0;
}

void
CPU::disassemble(RecInstr recInstr, DisInstr &result)
{
    uint32_t pc = recInstr.pc;
    uint16_t sr = recInstr.sr;

    disassemble(pc, result);
    
    result.flags[0]  = (sr & 0b1000000000000000) ? 'T' : 't';
    result.flags[1]  = '-';
    result.flags[2]  = (sr & 0b0010000000000000) ? 'S' : 's';
    result.flags[3]  = '-';
    result.flags[4]  = '-';
    result.flags[5]  = (sr & 0b0000010000000000) ? '1' : '0';
    result.flags[6]  = (sr & 0b0000001000000000) ? '1' : '0';
    result.flags[7]  = (sr & 0b0000000100000000) ? '1' : '0';
    result.flags[8]  = '-';
    result.flags[9]  = '-';
    result.flags[10] = '-';
    result.flags[11] = (sr & 0b0000000000010000) ? 'X' : 'x';
    result.flags[12] = (sr & 0b0000000000001000) ? 'N' : 'n';
    result.flags[13] = (sr & 0b0000000000000100) ? 'Z' : 'z';
    result.flags[14] = (sr & 0b0000000000000010) ? 'V' : 'v';
    result.flags[15] = (sr & 0b0000000000000001) ? 'C' : 'c';
    result.flags[16] = 0;
}

void
CPU::truncateTraceBuffer(unsigned count)
{
    for (unsigned i = writePtr; i < writePtr + traceBufferCapacity - count; i++) {
        traceBuffer[i % traceBufferCapacity].pc = UINT32_MAX; // mark element as unsed
    }
}

void
CPU::recordInstruction()
{
    RecInstr instr;
    
    // Setup record
    instr.pc = getPC();
    instr.sr = getSR();

    // Store record
    assert(writePtr < traceBufferCapacity);
    traceBuffer[writePtr] = instr;

    // Advance write pointer
    writePtr = (writePtr + 1) % traceBufferCapacity;
}

Cycle
CPU::executeInstruction()
{
    moiracpu.execute();
    return CPU_CYCLES(moiracpu.getClock());
}
